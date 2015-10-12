#include "de_clock.h"
#include <asm/io.h>
#include <asm/system.h>
#include <smc.h>

#define SETMASK(width, shift)   ((width?((-1U) >> (32-width)):0)  << (shift))
#define CLRMASK(width, shift)   (~(SETMASK(width, shift)))
#define GET_BITS(shift, width, reg)     \
            (((reg) & SETMASK(width, shift)) >> (shift))
#define SET_BITS(shift, width, reg, val) \
            (((reg) & CLRMASK(width, shift)) | (val << (shift)))

#define __inf(msg...)
#define __wrn(msg...)		printf(msg)

//#define readl(addr) (*(volatile u32 *) (addr))
//#define writel(val, addr) ((*(volatile u32 *) (addr)) = (val))

#define PLL7_BIAS 0x060000b8
#define PLL8_BIAS 0x060000bc

static u32 de_clk_freq = 0;

clk_pll_para disp_clk_pll_tbl[] ={
	PLL_CFG(SYS_CLK_PLL7,   CLK_NONE, 0,            0x06000018, 31, 16, 1, 8, 8, 32, 0)
	PLL_CFG(SYS_CLK_PLL8,   CLK_NONE, 297000000,    0x0600001c, 31, 16, 1, 8, 8, 0,  2)
	PLL_CFG(SYS_CLK_PLL10,  CLK_NONE, 2376000000UL, 0x06000024, 31, 16, 1, 8, 8, 18, 1)
};

clk_mod_para disp_clk_mod_tbl[] ={
	MOD_CFG(MOD_CLK_DETOP,    SYS_CLK_PLL10, 396000000, 0x06000588, 7,  0x060005a8, 7,  0x06000490, 31, 32, 0, 0,  4)
	MOD_CFG(MOD_CLK_LCD0CH0,  SYS_CLK_PLL7,  0,         0x06000588, 0,  0x060005a8, 0,  0x0600049c, 31, 24, 4, 0,  4)
	MOD_CFG(MOD_CLK_LCD0CH1,  SYS_CLK_PLL7,  0,         0x06000588, 0,  0x060005a8, 0,  0x0600049c, 31, 24, 4, 0,  4)
	MOD_CFG(MOD_CLK_LCD1CH0,  SYS_CLK_PLL8,  0,         0x06000588, 1,  0x060005a8, 1,  0x060004a0, 31, 24, 4, 0,  4)
	MOD_CFG(MOD_CLK_LCD1CH1,  SYS_CLK_PLL8,  0,         0x06000588, 1,  0x060005a8, 1,  0x060004a0, 31, 24, 4, 0,  4)
	MOD_CFG(MOD_CLK_MIPIDSIS, SYS_CLK_PLL7,  297000000, 0x06000588, 11, 0x060005a8, 11, 0x060004a8, 31, 24, 4, 0,  4)
	MOD_CFG(MOD_CLK_MIPIDSIP, SYS_CLK_PLL7,  297000000, 0x06000588, 32, 0x060005a8, 32, 0x060004ac, 31, 32, 0, 32, 0)
	MOD_CFG(MOD_CLK_HDMI,     SYS_CLK_PLL8,  0,         0x06000588, 5,  0x060005a8, 5,  0x060004b0, 31, 24, 4, 0,  4)
	MOD_CFG(MOD_CLK_HDMI_DDC, SYS_CLK_PLL8,  0,         0x06000588, 32, 0x060005a8, 6,  0x060004b4, 31, 32, 0, 32, 0)/* add hdmi reset at hdmi ddc */
	MOD_CFG(MOD_CLK_LVDS,     CLK_NONE,      0,         0x06000588, 32, 0x060005a8, 3,  0x0,        32, 32, 0, 32, 0)
	MOD_CFG(MOD_CLK_EDP,      CLK_NONE,      0,         0x06000588, 2,  0x060005a8, 2,  0x06000494, 31, 32, 0, 32, 0)
};

de_top_para disp_de_top_tbl[] ={
	DE_TOP_CFG(MOD_CLK_DEBE0,   396000000, 0x03000008, 8,  0x0300000c, 8,  0x03000004,  8,  0x03000000, 8,  0x03000020, 16)
	DE_TOP_CFG(MOD_CLK_DEBE1,   396000000, 0x03000008, 9,  0x0300000c, 9,  0x03000004,  9,  0x03000000, 9,  0x03000020, 20)
	DE_TOP_CFG(MOD_CLK_DEBE2,   396000000, 0x03000008, 10, 0x0300000c, 10, 0x03000004,  10, 0x03000000, 10, 0x03000020, 24)
	DE_TOP_CFG(MOD_CLK_DEFE0,   396000000, 0x03000008, 0,  0x0300000c, 0,  0x03000004,  0,  0x03000000, 0,  0x03000020, 0)
	DE_TOP_CFG(MOD_CLK_DEFE1,   396000000, 0x03000008, 1,  0x0300000c, 1,  0x03000004,  1,  0x03000000, 1,  0x03000020, 4)
	DE_TOP_CFG(MOD_CLK_DEFE2,   396000000, 0x03000008, 2,  0x0300000c, 2,  0x03000004,  2,  0x03000000, 2,  0x03000020, 8)
	DE_TOP_CFG(MOD_CLK_IEPDEU0, 0,         0x03000008, 4,  0x0300000c, 4,  0x03000004,  4,  0x03000000, 4,  0x0,        32)
	DE_TOP_CFG(MOD_CLK_IEPDEU1, 0,         0x03000008, 5,  0x0300000c, 5,  0x03000004,  5,  0x03000000, 5,  0x0,        32)
	DE_TOP_CFG(MOD_CLK_IEPDRC0, 0,         0x03000008, 12, 0x0300000c, 12, 0x03000004,  12, 0x03000000, 12, 0x0,        32)
	DE_TOP_CFG(MOD_CLK_IEPDRC1, 0,         0x03000008, 13, 0x0300000c, 13, 0x03000004,  13, 0x03000000, 13, 0x0,        32)
	DE_TOP_CFG(MOD_CLK_MERGE,   0,         0x0,        32, 0x0300000c, 20, 0x0,         32, 0x03000000, 20, 0x0,        32)
};

u32 pll_enable_count[] = {0, 0, 0};
u32 mod_enable_count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
u32 de_top_enable_count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


void modify_cpu_source_ctrl(void)
{
	u32 reg_val;
	reg_val = smc_readl(CCM_CPU_SOURCECTRL);
	reg_val &= ~1;

	smc_writel(reg_val, CCM_CPU_SOURCECTRL);
	__usdelay(10);
	dmb();
	isb();
}

void restore_cpu_source_ctrl(void)
{
	u32 reg_val;
	reg_val = smc_readl(CCM_CPU_SOURCECTRL);
	reg_val |= 1;
	smc_writel(reg_val, CCM_CPU_SOURCECTRL);

	__usdelay(1000);
	dmb();
	isb();
}

u64 dif(u64 value0, u64 value1)
{
	if(value0 > value1)
		return value0 - value1;
	else
		return value1 - value0;
}

u32 pow2(u32 val)
{
	if(val == 0)
		return 1;
	else
		return 2 * pow2(val - 1);
}

u32 de_clk_get_freq(u32 freq_level)
{
	if(freq_level == 0)
		de_clk_freq = 297000000;
	else if(freq_level == 1)
		de_clk_freq = 396000000;

	return de_clk_freq;
}

void calc_src_coef3(u32 *div_M, u32 *div_P, u32 *fac_N, u32 freq)
{
	u32 m, p, n;
	u64 temp;
	u32 temp_m = 0;
	u32 temp_p = 0;
	u32 temp_n = 0;
	u64 cur_val = 0;

	if(freq > 4200000000UL) {
		freq = 4200000000UL;
		__wrn("warning: freq set is over 4200M\n");
	}

	for(n = 12; n < 256; n++)
		for(p = 0; p < 2; p++)
			for(m = 0; m < 2; m++)
			{
				temp = n;
				temp = temp * 24000000;
				temp = temp / ((m + 1) * (p + 1));
				if(dif(temp, freq) < dif(cur_val, freq))
				{
					temp_m = m;
					temp_p = p;
					temp_n = n;
					cur_val = temp;
				}
			}

	*div_M = temp_m;
	*div_P = temp_p;
	*fac_N = temp_n;
}

void calc_pll8_coef(u32 *div_M, u32 *div_P, u32 *fac_N, u32 freq)
{
	u32 m, p, n;
	u64 temp;
	u32 temp_m = 0;
	u32 temp_p = 0;
	u32 temp_n = 0;
	u64 cur_val = 0;

	if(freq > 4200000000UL) {
		freq = 4200000000UL;
		__wrn("warning: freq set is over 4200M\n");
	}

	for(n = 12; n < 256; n++)
		for(m = 0; m < 2; m++)
			for(p = 0; p < 3; p++)
			{
				temp = n;
				temp = temp * 24000000;
				temp = temp / ((m + 1) * pow2(p));
				if(dif(temp, freq) < dif(cur_val, freq))
				{
					temp_m = m;
					temp_p = p;
					temp_n = n;
					cur_val = temp;
				}
			}

	*div_M = temp_m;
	*div_P = temp_p;
	*fac_N = temp_n;
}

void calc_src_coef2(u32 *div_M, u32 *fac_N, u32 freq)
{
	u32 m, n;
	u64 temp;
	u32 temp_m = 0;
	u32 temp_n = 0;
	u64 cur_val = 0;

	if(freq > 4200000000UL) {
		freq = 4200000000UL;
		__wrn("warning: freq set is over 4200M\n");
	}

	for(n = 12; n < 256; n++)
		for(m = 0; m < 2; m++)
		{
			temp = n;
			temp = temp * 24000000;
			temp = temp / (m + 1);
			if(dif(temp, freq) < dif(cur_val, freq))
			{
				temp_m = m;
				temp_n = n;
				cur_val = temp;
			}
		}

		*div_M = temp_m;
		*fac_N = temp_n;
}

void de_clk_init_pll(void)
{
	u32 i;
	u32 count;

	count = sizeof(disp_clk_pll_tbl) / sizeof(clk_pll_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_pll_tbl[i].freq)
			de_set_pll_rate(disp_clk_pll_tbl[i].clk_id, disp_clk_pll_tbl[i].freq);
	}
	/* PLL BIAS default config */
	smc_writel(0x00040000, PLL7_BIAS);
	smc_writel(0x00040000, PLL8_BIAS);
}

u32 de_get_pll_rate(__disp_clk_id_t clk_id)
{
	u32 fac_N, fac_M, ext_fac_P;
	u32 reg_val;

	if(clk_id == SYS_CLK_PLL10)
	{
		reg_val = smc_readl(disp_clk_pll_tbl[2].pll_adr);
		fac_N = GET_BITS(disp_clk_pll_tbl[2].fac_shift, disp_clk_pll_tbl[2].fac_bit_num, reg_val);
		fac_M = GET_BITS(disp_clk_pll_tbl[2].div_shift, disp_clk_pll_tbl[2].div_bit_num, reg_val);
		ext_fac_P = GET_BITS(disp_clk_pll_tbl[2].ext_div_shift, disp_clk_pll_tbl[2].ext_div_bit_num, reg_val);
		return 24000 * fac_N / ((fac_M + 1) * (ext_fac_P + 1)) * 1000;//avoid overflow, multiply 1000 at the last
	}
	else if(clk_id == SYS_CLK_PLL7)
	{
		reg_val = smc_readl(disp_clk_pll_tbl[0].pll_adr);
		fac_N = GET_BITS(disp_clk_pll_tbl[0].fac_shift, disp_clk_pll_tbl[0].fac_bit_num, reg_val);
		fac_M = GET_BITS(disp_clk_pll_tbl[0].div_shift, disp_clk_pll_tbl[0].div_bit_num, reg_val);
		return 24000 * fac_N / (fac_M + 1) * 1000;
	}
	else if(clk_id == SYS_CLK_PLL8)
	{
		reg_val = smc_readl(disp_clk_pll_tbl[1].pll_adr);
		fac_N = GET_BITS(disp_clk_pll_tbl[1].fac_shift, disp_clk_pll_tbl[1].fac_bit_num, reg_val);
		fac_M = GET_BITS(disp_clk_pll_tbl[1].div_shift, disp_clk_pll_tbl[1].div_bit_num, reg_val);
		ext_fac_P = GET_BITS(disp_clk_pll_tbl[1].ext_div_shift, disp_clk_pll_tbl[1].ext_div_bit_num, reg_val);
		return 24000 * fac_N / ((fac_M + 1) * pow2(ext_fac_P)) * 1000;
	}
	else
	{
	}

	__wrn("get clk id %d fail\n", clk_id);
	return 0;
}

u32 de_set_pll_rate(__disp_clk_id_t clk_id, u32 freq)
{
	u32 fac_N, fac_M, ext_fac;
	u32 reg_val;
	if(clk_id == SYS_CLK_PLL7)
	{
		reg_val = smc_readl(disp_clk_pll_tbl[0].pll_adr);
		calc_src_coef2(&fac_M, &fac_N, freq);
		reg_val = SET_BITS(disp_clk_pll_tbl[0].fac_shift, disp_clk_pll_tbl[0].fac_bit_num, reg_val, fac_N);
		reg_val = SET_BITS(disp_clk_pll_tbl[0].div_shift, disp_clk_pll_tbl[0].div_bit_num, reg_val, fac_M);
		smc_writel(reg_val, disp_clk_pll_tbl[0].pll_adr);
		return 1;
	}
	else if(clk_id == SYS_CLK_PLL8)
	{
		reg_val = smc_readl(disp_clk_pll_tbl[1].pll_adr);
		calc_pll8_coef(&fac_M, &ext_fac, &fac_N, freq);
		if(freq == 297000000) {
			fac_N = 0xc6;
			fac_M = 0x1;
			ext_fac = 0x3;
		}
		reg_val = SET_BITS(disp_clk_pll_tbl[1].fac_shift, disp_clk_pll_tbl[1].fac_bit_num, reg_val, fac_N);
		reg_val = SET_BITS(disp_clk_pll_tbl[1].div_shift, disp_clk_pll_tbl[1].div_bit_num, reg_val, fac_M);
		reg_val = SET_BITS(disp_clk_pll_tbl[1].ext_div_shift, disp_clk_pll_tbl[1].ext_div_bit_num, reg_val, ext_fac);
		smc_writel(reg_val, disp_clk_pll_tbl[1].pll_adr);
		return 1;
	}
	else if(clk_id == SYS_CLK_PLL10)
	{
		reg_val = smc_readl(disp_clk_pll_tbl[2].pll_adr);
		calc_src_coef3(&fac_M, &ext_fac, &fac_N, freq);
		reg_val = SET_BITS(disp_clk_pll_tbl[2].fac_shift, disp_clk_pll_tbl[2].fac_bit_num, reg_val, fac_N);
		reg_val = SET_BITS(disp_clk_pll_tbl[2].div_shift, disp_clk_pll_tbl[2].div_bit_num, reg_val, fac_M);
		reg_val = SET_BITS(disp_clk_pll_tbl[2].ext_div_shift, disp_clk_pll_tbl[2].ext_div_bit_num, reg_val, ext_fac);
		smc_writel(reg_val, disp_clk_pll_tbl[2].pll_adr);
		return 1;
	}
	else
	{
	}
	__wrn("set pll freq fail!\n");
	return 0;
}

u32 de_set_mclk_src(__hdle clk_hdl)
{
	u32 i;
	u32 count;
	u32 reg_val;

	clk_mod_para *hdl = (clk_mod_para*)clk_hdl;

	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == hdl->clk_id)
		{
			reg_val = smc_readl(disp_clk_mod_tbl[i].mod_adr);
			if(disp_clk_mod_tbl[i].src_id == SYS_CLK_PLL7)
			{
				reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_src_shift, disp_clk_mod_tbl[i].mod_src_bit_num, reg_val, 8);
				smc_writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
				return 1;
			}
			else if(disp_clk_mod_tbl[i].src_id == SYS_CLK_PLL8)
			{
				reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_src_shift, disp_clk_mod_tbl[i].mod_src_bit_num, reg_val, 9);
				smc_writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
				return 1;
			}
			else if(disp_clk_mod_tbl[i].src_id == SYS_CLK_PLL10)
			{
				reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_src_shift, disp_clk_mod_tbl[i].mod_src_bit_num, reg_val, 11);
				smc_writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
				return 1;
			}
			else
			{
				break;
			}
		}
	}
	__wrn("set mod clock %d src fail!\n", hdl->clk_id);
	return 0;
}

u32 de_get_mclk_src(__hdle clk_hdl)
{
	u32 i;
	u32 count;
	u32 src_sel;
	u32 reg_val;

	clk_mod_para *hdl = (clk_mod_para*)clk_hdl;
	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == hdl->clk_id)
		{
			reg_val = smc_readl(disp_clk_mod_tbl[i].mod_adr);
			src_sel = GET_BITS(disp_clk_mod_tbl[i].mod_src_shift, disp_clk_mod_tbl[i].mod_src_bit_num, reg_val);
			if(disp_clk_mod_tbl[i].clk_id == MOD_CLK_DETOP)
			{
				return SYS_CLK_PLL10;
			}
			else
			{
				if(src_sel == 8)
				{
					return SYS_CLK_PLL7;
				}
				else if(src_sel == 9)
				{
					return SYS_CLK_PLL8;
				}
				else if(src_sel == 11)
				{
					return SYS_CLK_PLL10;
				}
				else
				{
					break;
				}
			}
		}
	}
	__wrn("get mod clock %d src fail!\n", hdl->clk_id);
	return 0;
}

u32 de_set_mclk_freq(__hdle clk_hdl, u32 nFreq)
{
	u32 i;
	u32 count;
	u32 src_freq;
	u32 div;
	__disp_clk_id_t src_id;
	u32 reg_val;

	src_id = de_get_mclk_src(clk_hdl);
	if(!src_id) {
		__wrn("get mode clk src fail!\n");
		return 0;
	}

	src_freq = de_get_pll_rate(src_id);
	if(!src_freq) {
		__wrn("get src freq fail!\n");
		return 0;
	}

	clk_mod_para *hdl = (clk_mod_para*)clk_hdl;
	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == hdl->clk_id)
		{
			if(nFreq == 0) {
				nFreq = disp_clk_mod_tbl[i].freq;
			}

			if(nFreq == 0) {
				__wrn("get mode clk %d freq from table fail\n", hdl->clk_id);
				return 0;
			}

			div = (src_freq + nFreq / 2 - 1) / nFreq;
			if((div > 16) || (div < 1)) {
				__wrn("div is overflow\n");
				return 0;
			}

			reg_val = smc_readl(disp_clk_mod_tbl[i].mod_adr);
			reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_div_shift, disp_clk_mod_tbl[i].mod_div_bit_num, reg_val, (div - 1));
			smc_writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
			return 1;
		}
	}

	__wrn("get clk %d hdl fail\n", hdl->clk_id);
	return 0;
}

u32 de_get_mclk_freq(__hdle clk_hdl)
{
	u32 i;
	u32 count;
	u32 src_freq;
	u32 div;
	__disp_clk_id_t src_id;
	u32 reg_val;

	src_id = de_get_mclk_src(clk_hdl);
	if(!src_id) {
		__wrn("get mode clk src fail!\n");
		return 0;
	}

	src_freq = de_get_pll_rate(src_id);

	clk_mod_para *hdl = (clk_mod_para*)clk_hdl;
	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == hdl->clk_id)
		{
			reg_val = smc_readl(disp_clk_mod_tbl[i].mod_adr);
			div = GET_BITS(disp_clk_mod_tbl[i].mod_div_shift, disp_clk_mod_tbl[i].mod_div_bit_num, reg_val);
			return src_freq / (div + 1);
		}
	}
	return 0;
}

u32 de_pll_enable(__disp_clk_id_t clk_id, s32 bOnOff)
{
	u32 i;
	u32 count;
	u32 reg_val;

	count = sizeof(disp_clk_pll_tbl) / sizeof(clk_pll_para);

	for(i = 0; i < count; i++)
	{
		if(disp_clk_pll_tbl[i].clk_id == clk_id)
		{
			reg_val = smc_readl(disp_clk_pll_tbl[i].pll_adr);
			if(bOnOff)
			{
				pll_enable_count[i]++;
				if(pll_enable_count[i] == 1)
				{
					reg_val = SET_BITS(disp_clk_pll_tbl[i].enable_shift, 1, reg_val, 1);
					modify_cpu_source_ctrl();
					smc_writel(reg_val, disp_clk_pll_tbl[i].pll_adr);
					restore_cpu_source_ctrl();
					if(disp_clk_pll_tbl[i].src_id)
					{
						de_pll_enable(disp_clk_pll_tbl[i].src_id, bOnOff);
					}
				}

				__inf("enable pll %d\n", clk_id);
				return 1;
			}
			else
			{
				if(pll_enable_count[i] == 0)
				{
					__wrn("pll %d is already disable\n", clk_id);
				}

				pll_enable_count[i]--;
				if(pll_enable_count[i] == 0)
				{
					reg_val = SET_BITS(disp_clk_pll_tbl[i].enable_shift, 1, reg_val, 0);
					modify_cpu_source_ctrl();
					smc_writel(reg_val, disp_clk_pll_tbl[i].pll_adr);
					restore_cpu_source_ctrl();
					if(disp_clk_pll_tbl[i].src_id)
					{
						de_pll_enable(disp_clk_pll_tbl[i].src_id, bOnOff);
					}
				}
				__inf("disable pll %d\n", clk_id);
				return 1;
			}
		}
	}

	return 0;
}

u32 de_mclk_enable(__hdle clk_hdl, s32 bOnOff)
{
	u32 i;
	u32 count;
	u32 reg_val;

	clk_mod_para *hdl = (clk_mod_para*)clk_hdl;
	__inf("mod %d clock enable\n", hdl->clk_id);
	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == hdl->clk_id)
		{
			if(bOnOff)
			{
				mod_enable_count[i]++;
				if(mod_enable_count[i] == 1)
				{
					if(disp_clk_mod_tbl[i].mod_enable_shift < 32) {
						reg_val = smc_readl(disp_clk_mod_tbl[i].mod_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_enable_shift, 1, reg_val, 1);
						__inf("mod clock %d enable on\n", hdl->clk_id);
						smc_writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
					}

					if(disp_clk_mod_tbl[i].ahb_gate_shift < 32) {
						reg_val = smc_readl(disp_clk_mod_tbl[i].ahb_gate_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].ahb_gate_shift, 1, reg_val, 1);
						__inf("mod clock %d ahb gate on\n", hdl->clk_id);
						smc_writel(reg_val, disp_clk_mod_tbl[i].ahb_gate_adr);
					}

					if(disp_clk_mod_tbl[i].ahb_reset_shift < 32) {
						reg_val = smc_readl(disp_clk_mod_tbl[i].ahb_reset_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].ahb_reset_shift, 1, reg_val, 1);
						__inf("mod clock %d ahb reset on\n", hdl->clk_id);
						smc_writel(reg_val, disp_clk_mod_tbl[i].ahb_reset_adr);
					}

					if(disp_clk_mod_tbl[i].src_id)
					{
						de_pll_enable(disp_clk_mod_tbl[i].src_id, bOnOff);
					}
				}
				return 1;
			}
			else
			{
				if(mod_enable_count[i] == 0)
				{
					__wrn("mod %d is already disable\n", hdl->clk_id);
					break;
				}

				mod_enable_count[i]--;
				if(mod_enable_count[i] == 0)
				{
					if(disp_clk_mod_tbl[i].mod_enable_shift < 32) {
						reg_val = smc_readl(disp_clk_mod_tbl[i].mod_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].mod_enable_shift, 1, reg_val, 0);
						__inf("mod clock %d enable off\n", hdl->clk_id);
						smc_writel(reg_val, disp_clk_mod_tbl[i].mod_adr);
					}

					if(disp_clk_mod_tbl[i].ahb_gate_shift < 32) {
						reg_val = smc_readl(disp_clk_mod_tbl[i].ahb_gate_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].ahb_gate_shift, 1, reg_val, 0);
						__inf("mod clock %d ahb gate off\n", hdl->clk_id);
						smc_writel(reg_val, disp_clk_mod_tbl[i].ahb_gate_adr);
					}

					if(disp_clk_mod_tbl[i].ahb_reset_shift < 32) {
						reg_val = smc_readl(disp_clk_mod_tbl[i].ahb_reset_adr);
						reg_val = SET_BITS(disp_clk_mod_tbl[i].ahb_reset_shift, 1, reg_val, 0);
						__inf("mod clock %d ahb reset off\n", hdl->clk_id);
						smc_writel(reg_val, disp_clk_mod_tbl[i].ahb_reset_adr);
					}

					if(disp_clk_mod_tbl[i].src_id)
					{
						de_pll_enable(disp_clk_mod_tbl[i].src_id, bOnOff);
					}
				}

				return 1;
			}
		}
	}

	return 0;
}

u32 de_top_clk_get_freq(__disp_clk_id_t clk_id)
{
	u32 i;
	u32 count;
	u32 src_freq;
	u32 reg_val;
	u32 div;
	__hdle hdl;

	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == MOD_CLK_DETOP)
		{
			hdl = (__hdle)&disp_clk_mod_tbl[i];
			break;
		}

		__wrn("get detop clk fail\n");
		return 0;
	}

	src_freq = de_get_mclk_freq(hdl);

	if(src_freq == 0) {
		__wrn("get mod clk freq fail\n");
		return 0;
	}

	count = sizeof(disp_de_top_tbl) / sizeof(de_top_para);
	for(i = 0; i < count; i++)
	{
		if(disp_de_top_tbl[i].clk_id == clk_id)
		{
			reg_val = smc_readl(disp_de_top_tbl[i].mod_div_adr);
			div = GET_BITS(disp_de_top_tbl[i].mod_div_shift, 4, reg_val);
			return src_freq / div;
		}
	}

	__wrn("de top get freq fail\n");
	return 0;
}

u32 de_top_clk_set_freq(__disp_clk_id_t clk_id, u32 nFreq)
{
	u32 i;
	u32 count;
	u32 src_freq;
	u32 reg_val;
	u32 div;
	__hdle hdl;

	count = sizeof(disp_clk_mod_tbl) / sizeof(clk_mod_para);
	for(i = 0; i < count; i++)
	{
		if(disp_clk_mod_tbl[i].clk_id == MOD_CLK_DETOP)
		{
			hdl = (__hdle)&disp_clk_mod_tbl[i];
			break;
		}

		__wrn("get detop clk fail\n");
		return 0;
	}

	src_freq = de_get_mclk_freq(hdl);

	if(src_freq == 0) {
		__wrn("get mod clk freq fail\n");
		return 0;
	}

	count = sizeof(disp_de_top_tbl) / sizeof(de_top_para);
	for(i = 0; i < count; i++)
	{
		if(disp_de_top_tbl[i].clk_id == clk_id)
		{
			nFreq = de_clk_freq;

			if(nFreq == 0)
			{
				nFreq = disp_de_top_tbl[i].freq;
			}

			if(nFreq == 0)
			{
				__wrn("de top set freq is 0\n");
				return 0;
			}

			div = (src_freq + nFreq / 2 - 1) / nFreq;

			if((div > 16) || (div < 1))
			{
				__wrn("de top clk div is overflow\n");
				return 0;
			}

			reg_val = smc_readl(disp_de_top_tbl[i].mod_div_adr);
			reg_val = SET_BITS(disp_de_top_tbl[i].mod_div_shift, 4, reg_val, (div -1));
			smc_writel(reg_val, disp_de_top_tbl[i].mod_div_adr);
			return 1;
		}
	}

	return 0;
}

u32 de_top_clk_enable(__disp_clk_id_t clk_id, s32 bOnOff)
{
	u32 i;
	u32 count;
	u32 reg_val;

	count = sizeof(disp_de_top_tbl) / sizeof(de_top_para);
	for(i = 0; i < count; i++)
	{
		if(disp_de_top_tbl[i].clk_id == clk_id)
		{
			if(bOnOff)
			{
				de_top_enable_count[i]++;
				if(de_top_enable_count[i] == 1)
				{
					if(disp_de_top_tbl[i].mod_enable_shift < 32)
					{
						reg_val = smc_readl(disp_de_top_tbl[i].mod_adr);
						reg_val = SET_BITS(disp_de_top_tbl[i].mod_enable_shift, 1, reg_val, 1);
						smc_writel(reg_val, disp_de_top_tbl[i].mod_adr);
						__inf("clk %d mod enable\n", clk_id);
					}

					if(disp_de_top_tbl[i].ahb_gate_shift < 32)
					{
						reg_val = smc_readl(disp_de_top_tbl[i].ahb_gate_adr);
						reg_val = SET_BITS(disp_de_top_tbl[i].ahb_gate_shift, 1, reg_val, 1);
						smc_writel(reg_val, disp_de_top_tbl[i].ahb_gate_adr);
						__inf("clk %d gate enable\n", clk_id);
					}

					if(disp_de_top_tbl[i].ahb_reset_shift < 32)
					{
						reg_val = smc_readl(disp_de_top_tbl[i].ahb_reset_adr);
						reg_val = SET_BITS(disp_de_top_tbl[i].ahb_reset_shift, 1, reg_val, 1);
						smc_writel(reg_val, disp_de_top_tbl[i].ahb_reset_adr);
						__inf("clk %d reset enable\n", clk_id);
					}

					if(disp_de_top_tbl[i].dram_gate_shift < 32)
					{
						reg_val = smc_readl(disp_de_top_tbl[i].dram_gate_adr);
						reg_val = SET_BITS(disp_de_top_tbl[i].dram_gate_shift, 1, reg_val, 1);
						smc_writel(reg_val, disp_de_top_tbl[i].dram_gate_adr);
						__inf("clk %d dram enable\n", clk_id);
					}
				}
			}
			else
			{
				if(de_top_enable_count[i] == 0)
				{
					__wrn("de top clk %d is already close\n", clk_id);
					return 0;
				}

				de_top_enable_count[i]--;
				if(de_top_enable_count[i] == 0)
				{
					if(disp_de_top_tbl[i].mod_enable_shift < 32)
					{
						reg_val = smc_readl(disp_de_top_tbl[i].mod_adr);
						reg_val = SET_BITS(disp_de_top_tbl[i].mod_enable_shift, 1, reg_val, 0);
						smc_writel(reg_val, disp_de_top_tbl[i].mod_adr);
						__inf("clk %d mod disable\n", clk_id);
					}

					if(disp_de_top_tbl[i].ahb_gate_shift < 32)
					{
						reg_val = smc_readl(disp_de_top_tbl[i].ahb_gate_adr);
						reg_val = SET_BITS(disp_de_top_tbl[i].ahb_gate_shift, 1, reg_val, 0);
						smc_writel(reg_val, disp_de_top_tbl[i].ahb_gate_adr);
						__inf("clk %d gate disable\n", clk_id);
					}

					if(disp_de_top_tbl[i].ahb_reset_shift < 32)
					{
						reg_val = smc_readl(disp_de_top_tbl[i].ahb_reset_adr);
						reg_val = SET_BITS(disp_de_top_tbl[i].ahb_reset_shift, 1, reg_val, 0);
						smc_writel(reg_val, disp_de_top_tbl[i].ahb_reset_adr);
						__inf("clk %d reset disable\n", clk_id);
					}

					if(disp_de_top_tbl[i].dram_gate_shift < 32)
					{
						reg_val = smc_readl(disp_de_top_tbl[i].dram_gate_adr);
						reg_val = SET_BITS(disp_de_top_tbl[i].dram_gate_shift, 1, reg_val, 0);
						smc_writel(reg_val, disp_de_top_tbl[i].dram_gate_adr);
						__inf("clk %d dram disable\n", clk_id);
					}
				}
			}
		}
	}

	return 0;
}
