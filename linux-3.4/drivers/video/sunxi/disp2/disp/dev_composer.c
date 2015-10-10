#ifndef DEV_COMPOSER_C_C
#define DEV_COMPOSER_C_C


#include <linux/sw_sync.h>
#include <linux/sync.h>
#include <linux/file.h>
#include "dev_disp.h"
#include <video/sunxi_display2.h>
static struct   mutex	gcommit_mutek;

#define DBG_TIME_TYPE 3
#define DBG_TIME_SIZE 100
typedef struct
{
	unsigned long         time[DBG_TIME_TYPE][DBG_TIME_SIZE];
	unsigned int          time_index[DBG_TIME_TYPE];
	unsigned int          count[DBG_TIME_TYPE];
}composer_health_info;

typedef struct {
    int                     outaquirefencefd;
    int                     width;
    int                     Wstride;
    int                     height;
    disp_pixel_format       format;
    unsigned int            phys_addr;
}WriteBack_t;
typedef struct
{
    int                 layer_num[2];
    disp_layer_config   layer_info[2][16];
    int*                aquireFenceFd;
    int                 aquireFenceCnt;
    int*                returnfenceFd;
    bool                needWB[2];
    unsigned int        ehancemode[2]; //0 is close,1 is whole,2 is half mode
    unsigned int        androidfrmnum;
    WriteBack_t         *WriteBackdata;
}setup_dispc_data_t;
typedef struct
{
    struct list_head    list;
    unsigned  int       framenumber;
    unsigned  int       androidfrmnum;
    setup_dispc_data_t  hwc_data;
}dispc_data_list_t;

typedef struct
{
    struct list_head    list;
    void *              vm_addr;
    void*               p_addr;
    unsigned int        size;
    unsigned  int       androidfrmnum;
    bool                isphaddr;
    bool                update;
}dumplayer_t;

struct composer_private_data
{
	struct work_struct    post2_cb_work;
	u32	                  Cur_Write_Cnt;
	u32                   Cur_Disp_Cnt[2];
	bool                  b_no_output;
    bool                  display_active[2];
    bool                  countrotate[2];
	struct                mutex	runtime_lock;
	struct list_head        update_regs_list;

	unsigned int            timeline_max;
	struct mutex            update_regs_list_lock;
	spinlock_t              update_reg_lock;
	struct work_struct      commit_work;
    struct workqueue_struct *Display_commit_work;
    struct sw_sync_timeline *relseastimeline;
    struct sw_sync_timeline *writebacktimeline;
    setup_dispc_data_t      *tmptransfer;
    disp_drv_info           *psg_disp_drv;

    struct list_head        dumplyr_list;
    spinlock_t              dumplyr_lock;
    unsigned int            listcnt;
    unsigned char           dumpCnt;
    unsigned char           display;
    unsigned char           layerNum;
    unsigned char           channelNum;
    bool                    pause;
    bool                    cancel;
    bool                    dumpstart;
    unsigned int            ehancemode[2];
	composer_health_info    health_info;
};
static struct composer_private_data composer_priv;

int dispc_gralloc_queue(setup_dispc_data_t *psDispcData, unsigned int framenuber);

//type: 0:acquire, 1:release; 2:display
static s32 composer_get_frame_fps(u32 type)
{
	__u32 pre_time_index, cur_time_index;
	__u32 pre_time, cur_time;
	__u32 fps = 0xff;

	pre_time_index = composer_priv.health_info.time_index[type];
	cur_time_index = (pre_time_index == 0)? (DBG_TIME_SIZE -1):(pre_time_index-1);

	pre_time = composer_priv.health_info.time[type][pre_time_index];
	cur_time = composer_priv.health_info.time[type][cur_time_index];

	if(pre_time != cur_time) {
		fps = 1000 * 100 / (cur_time - pre_time);
	}

	return fps;
}

//type: 0:acquire, 1:release; 2:display
static void composer_frame_checkin(u32 type)
{
	u32 index = composer_priv.health_info.time_index[type];
	composer_priv.health_info.time[type][index] = jiffies;
	index ++;
	index = (index>=DBG_TIME_SIZE)?0:index;
	composer_priv.health_info.time_index[type] = index;
	composer_priv.health_info.count[type]++;
}

unsigned int composer_dump(char* buf)
{
	u32 fps0,fps1,fps2;
	u32 cnt0,cnt1,cnt2;

	fps0 = composer_get_frame_fps(0);
	fps1 = composer_get_frame_fps(1);
	fps2 = composer_get_frame_fps(2);
	cnt0 = composer_priv.health_info.count[0];
	cnt1 = composer_priv.health_info.count[1];
	cnt2 = composer_priv.health_info.count[2];

	return sprintf(buf, "acquire: %d, %d.%d fps\nrelease: %d, %d.%d fps\ndisplay: %d, %d.%d fps\n",
		cnt0, fps0/10, fps0%10, cnt1, fps1/10, fps1%10, cnt2, fps2/10, fps2%10);
}

int dev_composer_debug(setup_dispc_data_t *psDispcData, unsigned int framenuber)
{
    unsigned int  size = 0 ;
    void *kmaddr = NULL,*vm_addr = NULL,*sunxi_addr = NULL;
    disp_layer_config *dumlayer = NULL;
    dumplayer_t *dmplyr = NULL, *next = NULL, *tmplyr = NULL;
    bool    find = 0;
    if(composer_priv.cancel && composer_priv.display != 255 && composer_priv.channelNum != 255 && composer_priv.layerNum != 255)
    {
        dumlayer = &(psDispcData->layer_info[composer_priv.display][composer_priv.channelNum*4+composer_priv.layerNum]);
        dumlayer->enable =0;
    }
    if(composer_priv.dumpstart)
    {
        if(composer_priv.dumpCnt > 0 && composer_priv.display != 255 && composer_priv.channelNum != 255 && composer_priv.layerNum != 255 )
        {
            dumlayer = &(psDispcData->layer_info[composer_priv.display][composer_priv.channelNum*4+composer_priv.layerNum]);
            if(dumlayer != NULL && dumlayer->enable)
            {
                switch(dumlayer->info.fb.format)
                {
                    case DISP_FORMAT_ABGR_8888:
                    case DISP_FORMAT_ARGB_8888:
                    case DISP_FORMAT_XRGB_8888:
                    case DISP_FORMAT_XBGR_8888:
                        size = 4*dumlayer->info.fb.size[0].width * dumlayer->info.fb.size[0].height;
                    break;
                    case DISP_FORMAT_BGR_888:
                        size = 3*dumlayer->info.fb.size[0].width * dumlayer->info.fb.size[0].height;
                    break;
                    case DISP_FORMAT_RGB_565:
                        size = 2*dumlayer->info.fb.size[0].width * dumlayer->info.fb.size[0].height;
                    break;
                    case DISP_FORMAT_YUV420_P:
                        size = dumlayer->info.fb.size[0].width * dumlayer->info.fb.size[0].height
                            +dumlayer->info.fb.size[1].width * dumlayer->info.fb.size[1].height
                            +dumlayer->info.fb.size[2].width * dumlayer->info.fb.size[2].height;
                    break;
                    case DISP_FORMAT_YUV420_SP_VUVU:
                        size = dumlayer->info.fb.size[0].width * dumlayer->info.fb.size[0].height
                            +dumlayer->info.fb.size[1].width * dumlayer->info.fb.size[1].height;
                    break;
                    case DISP_FORMAT_YUV420_SP_UVUV:
                        size = dumlayer->info.fb.size[0].width * dumlayer->info.fb.size[0].height
                            +dumlayer->info.fb.size[1].width * dumlayer->info.fb.size[1].height;
                    break;
                    default:
                        printk("we got a err info..\n");
                }
                sunxi_addr = sunxi_map_kernel((unsigned int)dumlayer->info.fb.addr[0],size);
                if(sunxi_addr == NULL)
                {
                    printk("map mem err...\n");
                    goto err;
                }
                list_for_each_entry_safe(dmplyr, next, &composer_priv.dumplyr_list, list)
                {
                    if(!dmplyr->update)
                    {
                        if(dmplyr->size == size)
                        {
                            find = 1;
                            break;
                        }else{
                            tmplyr = dmplyr;
                        }
                    }
                }
                if(find)
                {
                    memcpy(dmplyr->vm_addr, sunxi_addr, size);
                    sunxi_unmap_kernel(sunxi_addr,dumlayer->info.fb.addr[0],size);
                    dmplyr->update = 1;
                    dmplyr->androidfrmnum = framenuber;
                    composer_priv.dumpCnt--;
                    goto ok;
                }else if(tmplyr != NULL)
                {
                    dmplyr = tmplyr;
                     if(dmplyr->isphaddr)
                        {
                            sunxi_unmap_kernel(dmplyr->vm_addr,dmplyr->isphaddr,dmplyr->size);
                            sunxi_free_phys((unsigned int)dmplyr->p_addr,dmplyr->size);
                        }else{
                            kfree((void *)dmplyr->vm_addr);
                        }
                        dmplyr->vm_addr = NULL;
                        dmplyr->p_addr = NULL;
                        dmplyr->size = 0;
                }else{
                    dmplyr = kzalloc(sizeof(dumplayer_t),GFP_KERNEL);
                    if(dmplyr == NULL)
                    {
                        printk("kzalloc mem err...\n");
			sunxi_unmap_kernel(sunxi_addr,dumlayer->info.fb.addr[0],size);
                        goto err;
                    }
                }
                vm_addr = sunxi_buf_alloc(size, (unsigned int*)&kmaddr);
                dmplyr->isphaddr = 1;
                if(kmaddr == 0)
                {
                    vm_addr = kzalloc(size,GFP_KERNEL);
                    dmplyr->isphaddr = 0;
                    if(vm_addr == NULL)
                    {
			sunxi_unmap_kernel(sunxi_addr,dumlayer->info.fb.addr[0],size);
                        dmplyr->update = 0;
                        printk("kzalloc mem err...\n");
                        goto err;
                    }
                    dmplyr->vm_addr = vm_addr;
                }
                if(dmplyr->isphaddr)
                {
                    dmplyr->vm_addr = sunxi_map_kernel((unsigned int)kmaddr,size);
                    if(dmplyr->vm_addr == NULL)
                    {
			sunxi_unmap_kernel(sunxi_addr,dumlayer->info.fb.addr[0],size);
                        dmplyr->update = 0;
                        printk("kzalloc mem err...\n ");
                        goto err;
                    }
                    dmplyr->p_addr = kmaddr;
                }
                dmplyr->size = size;
                dmplyr->androidfrmnum = framenuber;
                memcpy(dmplyr->vm_addr, sunxi_addr, size);
		sunxi_unmap_kernel(sunxi_addr,dumlayer->info.fb.addr[0],size);
                dmplyr->update = 1;
                if(tmplyr == NULL)
                {
                    composer_priv.listcnt++;
                    list_add_tail(&dmplyr->list, &composer_priv.dumplyr_list);
                }
            }
            composer_priv.dumpCnt--;
        }
    }
ok:
    return 0;
err:
    return -1;
}
static int debug_write_file(dumplayer_t *dumplyr)
{
    char s[30];
    struct file *dumfile;
    mm_segment_t old_fs;
    int cnt;
    if(dumplyr->vm_addr != NULL && dumplyr->size != 0)
    {
        cnt = sprintf(s, "/mnt/sdcard/dumplayer%d",dumplyr->androidfrmnum);
        dumfile = filp_open(s, O_RDWR|O_CREAT, 0755);
        if(IS_ERR(dumfile))
        {
            printk("open %s err[%d]\n",s,(int)dumfile);
            return 0;
        }
        old_fs = get_fs();
        set_fs(KERNEL_DS);
        if(dumplyr->vm_addr != NULL && dumplyr->size !=0)
        {
            dumfile->f_op->write(dumfile, dumplyr->vm_addr, dumplyr->size, &dumfile->f_pos);
        }
        set_fs(old_fs);
        filp_close(dumfile, NULL);
        dumfile = NULL;
        dumplyr->update = 0;
    }
   return 0 ;
}

/* define the data cache of frame */
#define DATA_ALLOC 1
#define DATA_FREE 0
static int cache_num = 8;
static struct mutex cache_opr;
typedef struct data_cache{
	dispc_data_list_t *addr;
	int flag;
	struct data_cache *next;
}data_cache_t;
static data_cache_t *frame_data;
/* destroy data cache of frame */
static int mem_cache_destroy(){
	data_cache_t *cur = frame_data;
	data_cache_t *next = NULL;
	while(cur != NULL){
		if(cur->addr != NULL){
			kfree(cur->addr);
		}
		next = cur->next;
		kfree(cur);
		cur = next;
	}
	mutex_destroy(&cache_opr);
	return 0;
}
/* create data cache of frame */
static int mem_cache_create(){
	int i = 0;
	data_cache_t *cur = NULL;
	mutex_init(&cache_opr);
	frame_data = kzalloc(sizeof(data_cache_t), GFP_ATOMIC);
	if(frame_data == NULL){
		printk("alloc frame data[0] fail\n");
		return -1;
	}
	frame_data->addr = kzalloc(sizeof(dispc_data_list_t), GFP_ATOMIC);
	if(frame_data->addr == NULL){
		printk("alloc dispc data[0] fail\n");
		mem_cache_destroy();
		return -1;
	}
	frame_data->flag = DATA_FREE;
	cur = frame_data;
	for(i = 1; i < cache_num ; i ++){
		cur->next = kzalloc(sizeof(data_cache_t), GFP_ATOMIC);
		if(cur->next == NULL){
			printk("alloc frame data[%d] fail\n", i);
			mem_cache_destroy();
			return -1;
		}
		cur->next->addr = kzalloc(sizeof(dispc_data_list_t), GFP_ATOMIC);
		if(cur->next->addr == NULL){
			printk("alloc dispc data[%d] fail\n", i);
			mem_cache_destroy();
			return -1;
		}
		cur->next->flag = DATA_FREE;
		cur = cur->next;
	}
	return 0;
}
/* free data of a frame from cache*/
static int mem_cache_free(dispc_data_list_t *addr){
	int i = 0;
	data_cache_t *cur = NULL;
	mutex_lock(&cache_opr);
	cur = frame_data;
	for(i = 0; (cur != NULL) && (i < cache_num); i++){
		if(addr != NULL && cur->addr == addr){
			cur->flag = DATA_FREE;
			mutex_unlock(&cache_opr);
			return 0;
		}
		cur = cur->next;
	}
	mutex_unlock(&cache_opr);
	return -1;
}
/* alloc data of a frame from cache */
static dispc_data_list_t* mem_cache_alloc(){
	int i = 0;
	data_cache_t *cur = NULL;
	mutex_lock(&cache_opr);
	cur = frame_data;
	for(i = 0; i < cache_num; i++){
		if(cur != NULL && cur->flag == DATA_FREE){
			if(cur->addr != NULL){
				memset(cur->addr, 0, sizeof(dispc_data_list_t));
			}
			cur->flag = DATA_ALLOC;
			mutex_unlock(&cache_opr);
			return cur->addr;
		}else if(cur == NULL){
			printk("alloc frame data fail, can not find avail buffer.\n");
			mutex_unlock(&cache_opr);
			return NULL;
		}
		if(i < cache_num - 1){
			cur = cur->next;
		}
	}
	printk("All frame data are used, try adding new...\n");
	cur->next = kzalloc(sizeof(data_cache_t), GFP_ATOMIC);
	if(cur->next == NULL){
		printk("alloc a frame data fail\n");
		mutex_unlock(&cache_opr);
		return NULL;
	}
	cur->next->addr = kzalloc(sizeof(dispc_data_list_t), GFP_ATOMIC);
	if(cur->next->addr == NULL){
		printk("alloc a dispc data fail\n");
		kfree(cur->next);
		cur->next = NULL;
		mutex_unlock(&cache_opr);
		return NULL;
	}
	cur->next->flag = DATA_ALLOC;
	cache_num++;
	printk("create a new frame data success, cache num update to %d", cache_num);
	mutex_unlock(&cache_opr);
	return cur->next->addr;
}

static void hwc_commit_work(struct work_struct *work)
{
    dispc_data_list_t *data, *next;
    struct list_head saved_list;
    int err;
    int i;
    struct sync_fence *AcquireFence;

    mutex_lock(&(gcommit_mutek));
    mutex_lock(&(composer_priv.update_regs_list_lock));
    list_replace_init(&composer_priv.update_regs_list, &saved_list);
    mutex_unlock(&(composer_priv.update_regs_list_lock));

    list_for_each_entry_safe(data, next, &saved_list, list)
    {
        list_del(&data->list);
	    for(i = 0; i < data ->hwc_data.aquireFenceCnt; i++)
	    {
            AcquireFence =(struct sync_fence *) data->hwc_data.aquireFenceFd[i];
            if(AcquireFence != NULL)
            {
                err = sync_fence_wait(AcquireFence,1000);
                sync_fence_put(AcquireFence);
                if (err < 0)
	            {
	                printk("synce_fence_wait timeout AcquireFence:%p\n",AcquireFence);
                    sw_sync_timeline_inc(composer_priv.relseastimeline, 1);
					goto free;
	            }
            }
	    }
        dev_composer_debug(&data->hwc_data, data->hwc_data.androidfrmnum);
        if(composer_priv.pause == 0)
        {
            dispc_gralloc_queue(&data->hwc_data, data->framenumber);
        }
free:
        kfree(data->hwc_data.aquireFenceFd);
        mem_cache_free(data);
    }
	mutex_unlock(&(gcommit_mutek));
}

static int hwc_commit(setup_dispc_data_t *disp_data)
{
	dispc_data_list_t *disp_data_list;
	struct sync_fence *fence;
	struct sync_pt *pt;
	int fd = -1, cout = 0, coutoffence = 0;
    int *fencefd = NULL;

    fencefd = kzalloc(( disp_data->aquireFenceCnt * sizeof(int)), GFP_ATOMIC);
    if(!fencefd){
        printk("out of momery , do not display.\n");
        return -1;
    }
    if(copy_from_user( fencefd, (void __user *)disp_data->aquireFenceFd, disp_data->aquireFenceCnt * sizeof(int)))
    {
            printk("copy_from_user fail\n");
            kfree(fencefd);
            return  -1;
    }
    for(cout = 0; cout < disp_data->aquireFenceCnt; cout++)
    {
        fence = sync_fence_fdget(fencefd[cout]);
        if(!fence)
        {
            printk("sync_fence_fdget failed,fd[%d]:%d\n",cout, fencefd[cout]);
            continue;
        }
        fencefd[coutoffence] = (int)fence;
        coutoffence++;
    }
    disp_data->aquireFenceFd = fencefd;
    disp_data->aquireFenceCnt = coutoffence;

    if(!composer_priv.b_no_output)
    {
        if(disp_data->layer_num[0]+disp_data->layer_num[1] > 0)
        {
            disp_data_list = mem_cache_alloc();
            if(!disp_data_list){
                kfree(fencefd);
                printk("%s: %d, alloc data for disp_data_list fail.\n", __func__, __LINE__);
                copy_to_user((void __user *)disp_data->returnfenceFd, &fd, sizeof(int));
                return -1;
            }
            fd = get_unused_fd();
            if (fd < 0)
            {
                mem_cache_free(disp_data_list);
                kfree(fencefd);
                return -1;
            }
            composer_priv.timeline_max++;
            pt = sw_sync_pt_create(composer_priv.relseastimeline, composer_priv.timeline_max);
            fence = sync_fence_create("sunxi_display", pt);
            sync_fence_install(fence, fd);
            memcpy(&disp_data_list->hwc_data, disp_data, sizeof(setup_dispc_data_t));
            disp_data_list->framenumber = composer_priv.timeline_max;
            mutex_lock(&(composer_priv.update_regs_list_lock));
            list_add_tail(&disp_data_list->list, &composer_priv.update_regs_list);
            mutex_unlock(&(composer_priv.update_regs_list_lock));
            if(!composer_priv.pause)
            {
                queue_work(composer_priv.Display_commit_work, &composer_priv.commit_work);
            }
        }else{
            kfree(fencefd);
            return -1;
        }
    }else{
        flush_workqueue(composer_priv.Display_commit_work);
        kfree(fencefd);
        return -1;
    }
    if(copy_to_user((void __user *)disp_data->returnfenceFd, &fd, sizeof(int)))
    {
	    printk("copy_to_user fail\n");
	    return  -EFAULT;
	}
	return 0;

}

static int hwc_commit_ioctl(unsigned int cmd, unsigned long arg)
{
    int ret = -1;
	if(DISP_HWC_COMMIT == cmd)
    {
        unsigned long *ubuffer;
        ubuffer = (unsigned long *)arg;
        memset(composer_priv.tmptransfer, 0, sizeof(setup_dispc_data_t));
        if(copy_from_user(composer_priv.tmptransfer, (void __user *)ubuffer[1], sizeof(setup_dispc_data_t)))
        {
            printk("copy_from_user fail\n");
            return  -EFAULT;
		}
        ret = hwc_commit(composer_priv.tmptransfer);
	}
	return ret;
}

static void disp_composer_proc(u32 sel)
{
    if(sel<2)
    {
        if(composer_priv.Cur_Write_Cnt < composer_priv.Cur_Disp_Cnt[sel])
        {
            composer_priv.countrotate[sel] = 1;
        }
        composer_priv.Cur_Disp_Cnt[sel] = composer_priv.Cur_Write_Cnt;
    }
	schedule_work(&composer_priv.post2_cb_work);
	return ;
}

static void imp_finish_cb(bool force_all)
{
    u32 little = 1;
	u32 flag = 0;

    if(composer_priv.pause)
    {
        return;
    }
    if(composer_priv.display_active[0])
    {
        little = composer_priv.Cur_Disp_Cnt[0];
    }
    if( composer_priv.display_active[0] && composer_priv.countrotate[0]
        &&composer_priv.display_active[1]&& composer_priv.countrotate[1])
    {
        composer_priv.countrotate[0] = 0;
        composer_priv.countrotate[1] = 0;
    }
    if(composer_priv.display_active[1])
    {
        if( composer_priv.display_active[0])
        {
            if(composer_priv.countrotate[0] != composer_priv.countrotate[1])
            {
                if(composer_priv.countrotate[0] && composer_priv.display_active[0])
                {
                    little = composer_priv.Cur_Disp_Cnt[1];
                }else{
                    little = composer_priv.Cur_Disp_Cnt[0];
                }
            }else{
                if(composer_priv.Cur_Disp_Cnt[1] > composer_priv.Cur_Disp_Cnt[0])
                {
                    little = composer_priv.Cur_Disp_Cnt[0];
                }else{
                    little = composer_priv.Cur_Disp_Cnt[1];
                }
            }
      }else{
            little = composer_priv.Cur_Disp_Cnt[1];
      }
    }
    while(composer_priv.relseastimeline->value != composer_priv.Cur_Write_Cnt)
    {
        if(!force_all && (composer_priv.relseastimeline->value >= little -1))
        {
            break;
        }
        sw_sync_timeline_inc(composer_priv.relseastimeline, 1);
        composer_frame_checkin(1);//release
        flag = 1;
    }
    if(flag)
		composer_frame_checkin(2);//display
}

static void post2_cb(struct work_struct *work)
{
	mutex_lock(&composer_priv.runtime_lock);
    imp_finish_cb(composer_priv.b_no_output);
	mutex_unlock(&composer_priv.runtime_lock);
}
extern s32  bsp_disp_shadow_protect(u32 disp, bool protect);
int dispc_gralloc_queue(setup_dispc_data_t *psDispcData, unsigned int framenuber)
{
    int disp;
    disp_drv_info *psg_disp_drv = composer_priv.psg_disp_drv;
    struct disp_manager *psmgr = NULL;
    struct disp_enhance *psenhance = NULL;
    disp_layer_config *psconfig;
    disp = 0;

    while( disp < DISP_NUMS_SCREEN )
    {
        if(!psDispcData->layer_num[disp])
        {
            composer_priv.display_active[disp] = 0;
            disp++;
            continue;
        }
        psmgr = psg_disp_drv->mgr[disp];
        if( psmgr != NULL  )
        {
            psenhance = psmgr->enhance;
            bsp_disp_shadow_protect(disp,true);
            if(psDispcData->ehancemode[disp] )
            {
                if(psDispcData->ehancemode[disp] != composer_priv.ehancemode[disp])
                {
                    switch (psDispcData->ehancemode[disp])
                    {
                        case 1:
                            psenhance->demo_disable(psenhance);
                            psenhance->enable(psenhance);
                        break;
                        case 2:
                            psenhance->enable(psenhance);
                            psenhance->demo_enable(psenhance);
                        break;
                        default:
                            psenhance->disable(psenhance);
                            printk("translat a err info\n");
                    }
                }
                composer_priv.ehancemode[disp] = psDispcData->ehancemode[disp];
            }else{
                if(composer_priv.ehancemode[disp])
                {
                    psenhance->disable(psenhance);
                    composer_priv.ehancemode[disp] = 0;
                }
            }

            psconfig = &psDispcData->layer_info[disp][0];
            psmgr->set_layer_config(psmgr, psconfig, disp?8:16);
            bsp_disp_shadow_protect(disp,false);
            if(composer_priv.display_active[disp] == 0)
            {
                composer_priv.display_active[disp] = 1;
                composer_priv.Cur_Disp_Cnt[disp] = framenuber;
            }
        }
        disp++;
    }
    composer_priv.Cur_Write_Cnt = framenuber;
    composer_frame_checkin(0);//acquire
    if(composer_priv.b_no_output)
    {
        mutex_lock(&composer_priv.runtime_lock);
        imp_finish_cb(1);
        mutex_unlock(&composer_priv.runtime_lock);
    }
  return 0;
}

static int hwc_suspend(void)
{
	composer_priv.b_no_output = 1;
	mutex_lock(&composer_priv.runtime_lock);
	printk("%s after lock\n", __func__);
	imp_finish_cb(1);
	mutex_unlock(&composer_priv.runtime_lock);
	printk("%s release lock\n", __func__);
	return 0;
}

static int hwc_resume(void)
{
	composer_priv.b_no_output = 0;
	printk("%s\n", __func__);
	return 0;
}
static struct dentry *composer_pdbg_root;

static int dumplayer_open(struct inode * inode, struct file * file)
{
	return 0;
}
static int dumplayer_release(struct inode * inode, struct file * file)
{
	return 0;
}

static ssize_t dumplayer_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    dumplayer_t *dmplyr = NULL, *next = NULL;
    composer_priv.dumpstart = 1;

    while(composer_priv.dumpCnt != 0)
    {
        list_for_each_entry_safe(dmplyr, next, &composer_priv.dumplyr_list, list)
        {
            if(dmplyr->update)
            {
                debug_write_file(dmplyr);
            }
        }
    }
    printk("dumplist counter %d\n",composer_priv.listcnt);
    list_for_each_entry_safe(dmplyr, next, &composer_priv.dumplyr_list, list)
    {
        list_del(&dmplyr->list);
        if(dmplyr->update)
        {
            debug_write_file(dmplyr);
        }
        if(dmplyr->isphaddr)
        {
            sunxi_unmap_kernel(dmplyr->vm_addr,dmplyr->isphaddr,dmplyr->size);
            sunxi_free_phys((unsigned int)dmplyr->p_addr,dmplyr->size);
        }else{
            kfree(dmplyr->vm_addr);
        }
        kfree(dmplyr);
    }
    composer_priv.dumpstart = 0;
	return 0;
}
static ssize_t dumplayer_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    dumplayer_t *dmplyr = NULL, *next = NULL;
    char temp[count+1];
    char *s = temp;
    int cnt=0;
    if(copy_from_user( temp, (void __user *)buf, count))
    {
        printk("copy_from_user fail\n");
        return  -EFAULT;
    }
    temp[count] = '\0';
    printk("%s\n",temp);
    switch(*s++)
    {
        case 'p':
            composer_priv.pause = 1;
        break;
        case 'q':
            composer_priv.pause = 0;
            composer_priv.cancel = 0;
            composer_priv.dumpstart = 0;
            composer_priv.dumpCnt = 0;
            list_for_each_entry_safe(dmplyr, next, &composer_priv.dumplyr_list, list)
            {
                list_del(&dmplyr->list);
                if(dmplyr->isphaddr)
                {
                    sunxi_unmap_kernel(dmplyr->vm_addr,dmplyr->isphaddr,dmplyr->size);
                    sunxi_free_phys((unsigned int)dmplyr->p_addr,dmplyr->size);
                }else{
                    kfree(dmplyr->vm_addr);
                }
                kfree(dmplyr);
            }
        break;
        case 'c':
            composer_priv.cancel = 1;
        case 'd':
            composer_priv.pause = 0;
            composer_priv.display = (*s >= 48 && *s <=49)? *s - 48 : 255;
            s++;
            if(*s == 'c')
            {
                s++;
                composer_priv.channelNum = (*s >= 48 && *s <=51)? *s - 48 : 255;
            }else{
                composer_priv.dumpCnt = 0;
                break;
            }
            s++;
            if(*s == 'l')
            {
                s++;
                composer_priv.layerNum = (*s >= 48 && *s <=51)? *s - 48 : 255;
            }else{
                composer_priv.dumpCnt = 0;
                break;
            }
            s++;
            if(*s == 'n')
            {
                while(*++s != '\0')
                {
                    if( 57< *s || *s <48)
                        break;
                    cnt += (*s-48);
                    cnt *=10;
                }
                composer_priv.dumpCnt = cnt/10;
            }else{
                composer_priv.dumpCnt = 0;
            }
            composer_priv.listcnt = 0;
        break;
        default:
            printk(" dev_composer debug give me a wrong arg...\n");
    }
    printk("dumps --Cancel[%d]--display[%d]--channel[%d]--layer[%d]--Cnt:[%d]--pause[%d] \n",composer_priv.cancel,composer_priv.display,composer_priv.channelNum,composer_priv.layerNum,composer_priv.dumpCnt,composer_priv.pause);
    return count;
}


static const struct file_operations dumplayer_ops = {
	.write        = dumplayer_write,
	.read        = dumplayer_read,
	.open        = dumplayer_open,
	.release    = dumplayer_release,
};

int composer_dbg(void)
{
	composer_pdbg_root = debugfs_create_dir("composerdbg", NULL);
	if(!debugfs_create_file("dumplayer", 0644, composer_pdbg_root, NULL,&dumplayer_ops))
		goto Fail;
	return 0;

Fail:
	debugfs_remove_recursive(composer_pdbg_root);
	composer_pdbg_root = NULL;
	return -ENOENT;
}

s32 composer_init(disp_drv_info *psg_disp_drv)
{
	int ret = 0;

	memset(&composer_priv, 0x0, sizeof(struct composer_private_data));

	INIT_WORK(&composer_priv.post2_cb_work, post2_cb);
	mutex_init(&composer_priv.runtime_lock);

    composer_priv.Display_commit_work = create_freezable_workqueue("SunxiDisCommit");
    INIT_WORK(&composer_priv.commit_work, hwc_commit_work);
	INIT_LIST_HEAD(&composer_priv.update_regs_list);
    INIT_LIST_HEAD(&composer_priv.dumplyr_list);
	composer_priv.relseastimeline = sw_sync_timeline_create("sunxi-display");
	composer_priv.timeline_max = 0;
	composer_priv.b_no_output = 0;
    composer_priv.Cur_Write_Cnt = 0;
    composer_priv.display_active[0] = 0;
    composer_priv.display_active[1] = 0;
    composer_priv.Cur_Disp_Cnt[0] = 0;
    composer_priv.Cur_Disp_Cnt[1] = 0;
    composer_priv.countrotate[0] = 0;
    composer_priv.countrotate[1] = 0;
    composer_priv.pause = 0;
    composer_priv.listcnt = 0;
	mutex_init(&composer_priv.update_regs_list_lock);
    mutex_init(&gcommit_mutek);
	spin_lock_init(&(composer_priv.update_reg_lock));
    spin_lock_init(&(composer_priv.dumplyr_lock));
	disp_register_ioctl_func(DISP_HWC_COMMIT, hwc_commit_ioctl);
    disp_register_sync_finish_proc(disp_composer_proc);
	disp_register_standby_func(hwc_suspend, hwc_resume);
    composer_priv.tmptransfer = kzalloc(sizeof(setup_dispc_data_t), GFP_ATOMIC);
    composer_priv.psg_disp_drv = psg_disp_drv;
    composer_dbg();
    /* alloc mem_des struct pool, 48k */
    ret = mem_cache_create();
    if(ret != 0){
        printk("%s(%d) alloc frame buffer err!\n", __func__, __LINE__);
    }

  return 0;
}
#endif
