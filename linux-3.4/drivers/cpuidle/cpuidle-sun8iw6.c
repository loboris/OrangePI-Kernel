/* linux/arch/arm/mach-sunxi/cpuidle.c
 *
 * Copyright (C) 2013-2014 allwinner.
 * kevin.z.m <kevin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cpuidle.h>
#include <linux/cpu_pm.h>
#include <linux/io.h>
#include <linux/export.h>
#include <linux/time.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/suspend.h>
#include <linux/cpumask.h>
#include <linux/smp.h>
#include <linux/delay.h>
#include <linux/clockchips.h>
#include <linux/sched.h>
#include <linux/hrtimer.h>
#include <linux/tick.h>
#include <linux/arisc/arisc.h>
#include <linux/arisc/hwspinlock.h>

#include <asm/smp_plat.h>
#include <asm/delay.h>
#include <asm/cp15.h>
#include <asm/suspend.h>
#include <asm/cacheflush.h>
#include <asm/hardware/gic.h>
#include <asm/cpuidle.h>
#include <mach/cpuidle-sunxi.h>

/* C2 State Flags */
#define CPUIDLE_FLAG_C2_STATE   (1<<16) /* into c2 state */

static DEFINE_PER_CPU(struct cpuidle_device, sunxi_cpuidle_device);

static int sunxi_enter_c0state(struct cpuidle_device *dev,
				struct cpuidle_driver *drv, int index)
{
	struct timeval before, after;
	int idle_time;

	local_irq_disable();
	do_gettimeofday(&before);

	cpu_do_idle();

	do_gettimeofday(&after);
	local_irq_enable();

	idle_time = (after.tv_sec - before.tv_sec) * USEC_PER_SEC +
	            (after.tv_usec - before.tv_usec);
	dev->last_residency = idle_time < 0 ? 0 : idle_time;

	return index;
}

/*
 * notrace prevents trace shims from getting inserted where they
 * should not. Global jumps and ldrex/strex must not be inserted
 * in power down sequences where caches and MMU may be turned off.
 */
static int notrace sunxi_powerdown_finisher(unsigned long flg)
{
	/* MCPM works with HW CPU identifiers */
	unsigned int mpidr = read_cpuid_mpidr();
	unsigned int cluster = MPIDR_AFFINITY_LEVEL(mpidr, 1);
	unsigned int cpu = MPIDR_AFFINITY_LEVEL(mpidr, 0);

	mcpm_set_entry_vector(cpu, cluster, cpu_resume);

	/*
	 * Residency value passed to mcpm_cpu_suspend back-end
	 * has to be given clear semantics. Set to 0 as a
	 * temporary value.
	 */
	mcpm_cpu_suspend(mpidr | flg);

	/* return value != 0 means failure */
	return 1;
}

static int sunxi_cpu_core_power_down(struct cpuidle_device *dev,
				struct cpuidle_driver *drv, int index)
{
	cpu_pm_enter();
	clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_ENTER, &dev->cpu);
	smp_wmb();

	cpu_suspend(0, sunxi_powerdown_finisher);

	clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_EXIT, &dev->cpu);
	cpu_pm_exit();

	return index;
}

/*
 * sunxi_enter_c1state - Programs CPU to enter the specified state
 * @dev: cpuidle device
 * @drv: The target state to be programmed
 * @idx: state index
 *
 * Called from the CPUidle framework to program the device to the
 * specified target state selected by the governor.
 */
static int sunxi_enter_c1state(struct cpuidle_device *dev,
				struct cpuidle_driver *drv, int index)
{
	struct timeval before, after;
	int idle_time;

	local_irq_disable();
	do_gettimeofday(&before);

	sunxi_cpu_core_power_down(dev, drv, index);

	do_gettimeofday(&after);
	local_irq_enable();
	idle_time = (after.tv_sec - before.tv_sec) * USEC_PER_SEC +
	            (after.tv_usec - before.tv_usec);
	dev->last_residency = idle_time;

	return index;
}

#define MSG_VBASE IO_ADDRESS(SUNXI_SRAM_A2_PBASE)
#define ARISC_MESSAGE_POOL_END (0x14000)
#define MSG_VEND (MSG_VBASE + ARISC_MESSAGE_POOL_END)

/* flg between cpux & cpus for synchronization */
#define CLUSTER_CPUX_FLG(cluster, cpu) (MSG_VEND -  4 - 32*(cluster) - 4*(cpu))
#define CLUSTER_CPUS_FLG(cluster, cpu) (MSG_VEND - 20 - 32*(cluster) - 4*(cpu))

/*
 * We can't use regular spinlocks. In the switcher case, it is possible
 * for an outbound CPU to call power_down() after its inbound counterpart
 * is already live using the same logical CPU number which trips lockdep
 * debugging.
 */
static arch_spinlock_t sun8i_mcpm_lock = __ARCH_SPIN_LOCK_UNLOCKED;

/* sunxi cluster and cpu use status,
 * this is use to detect the first-man and last-man.
 */
static int sun8i_cpu_use_count[MAX_NR_CLUSTERS][MAX_CPUS_PER_CLUSTER];
static int sun8i_cluster_use_count[MAX_NR_CLUSTERS];

/*
 * notrace prevents trace shims from getting inserted where they
 * should not. Global jumps and ldrex/strex must not be inserted
 * in power down sequences where caches and MMU may be turned off.
 */
static int notrace sunxi_powerdown_c2_finisher(unsigned long flg)
{
	/* MCPM works with HW CPU identifiers */
	unsigned int mpidr = read_cpuid_mpidr();
	unsigned int cluster = MPIDR_AFFINITY_LEVEL(mpidr, 1);
	unsigned int cpu = MPIDR_AFFINITY_LEVEL(mpidr, 0);
	bool last_man = false;
	struct sunxi_enter_idle_para sunxi_idle_para;

	mcpm_set_entry_vector(cpu, cluster, cpu_resume);

	arch_spin_lock(&sun8i_mcpm_lock);
	sun8i_cpu_use_count[cluster][cpu]--;
	/* check is the last-man, and set flg */
	sun8i_cluster_use_count[cluster]--;
	if (sun8i_cluster_use_count[cluster] == 0) {
		writel(1, CLUSTER_CPUX_FLG(cluster, cpu));
		last_man = true;
	}
	arch_spin_unlock(&sun8i_mcpm_lock);

	/* call cpus to power off */
	sunxi_idle_para.flags = (unsigned long)mpidr | flg;
	sunxi_idle_para.resume_addr = (void *)(virt_to_phys(mcpm_entry_point));
	arisc_enter_cpuidle(NULL, NULL, &sunxi_idle_para);

	if (last_man) {
		int t = 0;

		/* wait for cpus received this message and respond,
		 * for reconfirm is this cpu the man really, then clear flg
		 */
		while (1) {
			udelay(2);
			if (readl(CLUSTER_CPUS_FLG(cluster, cpu)) == 2) {
				writel(0, CLUSTER_CPUX_FLG(cluster, cpu));
				break; /* last_man is true */
			} else if (readl(CLUSTER_CPUS_FLG(cluster, cpu)) == 3) {
				writel(0, CLUSTER_CPUX_FLG(cluster, cpu));
				goto out; /* last_man is false */
			}
			if(++t > 5000) {
				printk(KERN_WARNING "cpu%didle time out!\n",  \
				                     cluster * 4 + cpu);
				t = 0;
			}
		}
		sunxi_idle_cluster_die(cluster);
	}
out:
	sunxi_idle_cpu_die();

	/* return value != 0 means failure */
	return 1;
}

static int sunxi_cpu_power_down_c2state(struct cpuidle_device *dev, \
                                               struct cpuidle_driver *drv, \
                                               int index)
{
	unsigned int mpidr = read_cpuid_mpidr();
	unsigned int cluster = MPIDR_AFFINITY_LEVEL(mpidr, 1);
	unsigned int cpu = MPIDR_AFFINITY_LEVEL(mpidr, 0);

	cpu_pm_enter();
	//cpu_cluster_pm_enter();
	clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_ENTER, &dev->cpu);
	smp_wmb();

	cpu_suspend(CPUIDLE_FLAG_C2_STATE, sunxi_powerdown_c2_finisher);

	/*
	 * Since this is called with IRQs enabled, and no arch_spin_lock_irq
	 * variant exists, we need to disable IRQs manually here.
	 */
	local_irq_disable();

	arch_spin_lock(&sun8i_mcpm_lock);
	sun8i_cpu_use_count[cluster][cpu]++;
	sun8i_cluster_use_count[cluster]++;
	arch_spin_unlock(&sun8i_mcpm_lock);

	local_irq_enable();

	clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_EXIT, &dev->cpu);
	//cpu_cluster_pm_exit();
	cpu_pm_exit();

	return index;
}

static int sunxi_enter_c2state(struct cpuidle_device *dev, \
                               struct cpuidle_driver *drv, int index)
{
	struct timeval before, after;
	int idle_time;

	local_irq_disable();
	do_gettimeofday(&before);

	sunxi_cpu_power_down_c2state(dev, drv, index);

	do_gettimeofday(&after);
	local_irq_enable();
	idle_time = (after.tv_sec - before.tv_sec) * USEC_PER_SEC + \
	            (after.tv_usec - before.tv_usec);
	dev->last_residency = idle_time < 0 ? 0 : idle_time;

	return index;
}

/*
 * @enter: low power process function
 * @exit_latency: latency of exit this state, based on us
 * @power_usage: power used by cpu under this state, based on mw
 * @target_residency: the minimum of time should cpu spend in
 *   this state, based on us
 */
static struct cpuidle_driver sunxi_idle_driver = {
	.name                           = "sunxi_idle",
	.owner                          = THIS_MODULE,
	.states[0] = {
		.enter                  = sunxi_enter_c0state,
		.exit_latency           = 1,
		.target_residency       = 100,
		.power_usage            = 1000,
		.flags                  = CPUIDLE_FLAG_TIME_VALID,
		.name                   = "C0",
		.desc                   = "ARM (WFI)",
	},
	.states[1] = {
		.enter                  = sunxi_enter_c1state,
		.exit_latency           = 3000,
		.target_residency       = 10000,
		.power_usage            = 500,
		.flags                  = CPUIDLE_FLAG_TIME_VALID,
		.name                   = "C1",
		.desc                   = "SUNXI CORE POWER DOWN",
	},
	.states[2] = {
		.enter                  = sunxi_enter_c2state,
		.exit_latency           = 10000,
		.target_residency       = 20000,
		.power_usage            = 100,
		.flags                  = CPUIDLE_FLAG_TIME_VALID,
		.name                   = "C2",
		.desc                   = "SUNXI CLUSTER POWER DOWN",
	},
	.state_count = 3,
};

static int sun8i_cpuidle_state_init(void)
{
	unsigned int cluster, cpu;

	for (cluster = 0; cluster < MAX_NR_CLUSTERS; cluster++) {
		for (cpu = 0; cpu < MAX_CPUS_PER_CLUSTER; cpu++)
			sun8i_cpu_use_count[cluster][cpu] = 1;
		sun8i_cluster_use_count[cluster] = MAX_CPUS_PER_CLUSTER;
	}

	return 0;
}

static int __init sunxi_init_cpuidle(void)
{
	int cpu;
	struct cpuidle_device *device;

	sun8i_cpuidle_state_init();

	sunxi_idle_driver.safe_state_index = 0;
	cpuidle_register_driver(&sunxi_idle_driver);

	for_each_possible_cpu(cpu) {
		device = &per_cpu(sunxi_cpuidle_device, cpu);
		device->cpu = cpu;
		if (cpuidle_register_device(device)) {
			printk(KERN_ERR "CPUidle register device failed\n,");
			return -EIO;
		}
	}

	return 0;
}
device_initcall(sunxi_init_cpuidle);
