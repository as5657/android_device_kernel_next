/*************************************************************************
*   版权所有(C) 1987-2015, 深圳海思半导体技术有限公司.
*
*   文 件 名 :  adrv.h
*
*   作    者 :  mohaoju
*
*   描    述 :  本文件命名为"adrv.h", 给出AP侧底软直接对外提供API接口描述
*
*   修改记录 :  2015年2月28日  v1.00  mohaoju创建
*************************************************************************/

#ifndef HISI_AP_DRV_H
#define HISI_AP_DRV_H

/*************************************************************************
*
*   启动/加载/复位/校验
*
*************************************************************************/


#define PARTITION_MODEM_IMAGE_NAME       "modem"
#define PARTITION_MODEM_DSP_NAME         "modem_dsp"
#define PARTITION_MODEM_NVDLOAD_NAME     "modemnvm_update"
#define PARTITION_MODEM_NVDEFAULT_NAME   "modemnvm_factory"
#define PARTITION_MODEM_NVBACKUP_NAME    "modemnvm_backup"
#define PARTITION_MODEM_NVSYS_NAME       "modemnvm_system"
#define PARTITION_MODEM_NVIMG_NAME       "modemnvm_img"
#define PARTITION_MODEM_LOG_NAME         "modem_om"
#define PARTITION_MODEM_DTB_NAME	 "modem_dtb"
#define PARTITION_PTN_VRL_P_NAME 	 "vrl"
#define PARTITION_PTN_VRL_M_NAME	 "vrl_backup"

typedef enum
{
	BSP_START_MODE_CHARGING = 0,
	BSP_START_MODE_NORMAL,
	BSP_START_MODE_UPDATE,
	BSP_START_MODE_BUILT
} BSP_START_MODE_E;

typedef enum {
	BSP_CCORE = 0,
	BSP_HIFI,
	BSP_BBE,
	BSP_CDSP,
	BSP_CCORE_TAS,
	BSP_CCORE_WAS,
	BSP_CCORE_CAS,
	BSP_BUTT
} BSP_CORE_TYPE_E;

/*****************************************************************************
* 函 数 名  : flash_find_ptn
*
* 功能描述  : 根据分区名返回分区信息
*
* 输入参数  : str 分区名
*
* 输出参数  : 无
*
* 返 回 值  : 0 成功 -1 失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int flash_find_ptn(const char* str, char* pblkname);

/*****************************************************************************
* 函 数 名  : bsp_need_loadmodem
*
* 功能描述  : 获取是否需要启动modem
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 1 需要启动
			  0 不需要启动
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_need_loadmodem(void);

/*****************************************************************************
* 函 数 名  : bsp_get_bootmode
*
* 功能描述  : 获取启动时modem的状态，正常启动/关机充电/升级
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 见BSP_START_MODE_E
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_get_bootmode(void);

/*****************************************************************************
* 函 数 名  : bsp_get_bootmode
*
* 功能描述  : Modem和HIFI 复位请求接口
*
* 输入参数  : ecoretype 见BSP_CORE_TYPE_E
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_reset_request (BSP_CORE_TYPE_E ecoretype);


/*****************************************************************************
* 函 数 名  : bsp_reset_loadimage
*
* 功能描述  : Modem和HIFI 镜像加载
*
* 输入参数  : ecoretype 见BSP_CORE_TYPE_E
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_reset_loadimage (BSP_CORE_TYPE_E ecoretype);

/*****************************************************************************
* 函 数 名  : bsp_image_seccheck
*
* 功能描述  : Modem和HIFI 镜像安全校验 校验通过后直接触发复位
*
* 输入参数  : ecoretype 见BSP_CORE_TYPE_E
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_image_seccheck (BSP_CORE_TYPE_E ecoretype);

/*****************************************************************************
* 函 数 名  : bsp_sysboot_core_notify
*
* 功能描述  : 复位C核时需要发送给m3和hifi的同步消息
*
* 输入参数  : ecoretype 见BSP_CORE_TYPE_E
			  cmdtype 命令码
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_sysboot_core_notify(BSP_CORE_TYPE_E ecoretype, int cmdtype );



/*************************************************************************
*
*   RDR/Coresight
*
*************************************************************************/
#ifndef  MODULE_NAME_LEN
#define MODULE_NAME_LEN (64 - sizeof(unsigned long))
#endif
#define STR_EXCEPTIONDESC_MAXLEN	48

struct list_head_rdr {
	struct list_head_rdr *next, *prev;
};
typedef void (*rdr_e_callback)(unsigned int, void*);

/*
 *   struct list_head_rdr   e_list;
 *   u32 modid,			    exception id;
 *		if modid equal 0, will auto generation modid, and return it.
 *   u32 modid_end,		    can register modid region. [modid~modid_end];
		need modid_end >= modid,
 *		if modid_end equal 0, will be register modid only,
		but modid & modid_end cant equal 0 at the same time.
 *   u32 process_priority,	exception process priority
 *   u32 reboot_priority,	exception reboot priority
 *   u64 save_log_mask,		need save log mask
 *   u64 notify_core_mask,	need notify other core mask
 *   u64 reset_core_mask,	need reset other core mask
 *   u64 from_core,		    the core of happen exception
 *   u32 reentrant,		    whether to allow exception reentrant
 *   u32 exce_type,		    the type of exception
 *   char* from_module,		    the module of happen excption
 *   char* desc,		        the desc of happen excption
 *   rdr_e_callback callback,	will be called when excption has processed.
 *   u32 reserve_u32;		reserve u32
 *   void* reserve_p		    reserve void *
 */
struct rdr_exception_info_s {
	struct list_head_rdr e_list;	/*list_head_rdr instead of list_head  to solve the confliction between <linux/types.h>*/
	unsigned int	e_modid;
	unsigned int	e_modid_end;
	unsigned int	e_process_priority;
	unsigned int	e_reboot_priority;
	unsigned long long	e_notify_core_mask;
	unsigned long long	e_reset_core_mask;
	unsigned long long	e_from_core;
	unsigned int	e_reentrant;
    unsigned int    e_exce_type;
    unsigned int    e_upload_flag;
	unsigned char	e_from_module[MODULE_NAME_LEN];
	unsigned char	e_desc[STR_EXCEPTIONDESC_MAXLEN];
	unsigned int	e_reserve_u32;
	void*	e_reserve_p;
	rdr_e_callback e_callback;
};

typedef void (*pfn_cb_dump_done)(unsigned int modid, unsigned long long coreid);
typedef void (*pfn_dump)(unsigned int modid, unsigned int etype, unsigned long long coreid, char* logpath, pfn_cb_dump_done fndone);
typedef void (*pfn_reset)(unsigned int modid, unsigned int etype, unsigned long long coreid);

struct rdr_module_ops_pub {
    pfn_dump    ops_dump;
    pfn_reset   ops_reset;
};

struct rdr_register_module_result {
    unsigned long long   log_addr;
    unsigned int         log_len;
    unsigned long long   nve;
};


/*****************************************************************************
* 函 数 名  : rdr_register_exception
*
* 功能描述  : RDR异常注册接口
*
* 输入参数  : rdr_exception_info_s
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理失败
			  >0 e_modid_end
*
* 其它说明  : kernel
*
*****************************************************************************/
unsigned int rdr_register_exception (struct rdr_exception_info_s* e);

/*****************************************************************************
* 函 数 名  : rdr_register_module_ops
*
* 功能描述  : 注册异常处理函数
*
* 输入参数  : coreid  异常核ID
			  ops     异常处理函数与reset函数
*             retinfo 注册返回信息
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int rdr_register_module_ops(unsigned long long coreid, struct rdr_module_ops_pub* ops, struct rdr_register_module_result* retinfo);


/*****************************************************************************
* 函 数 名  : rdr_system_error
*
* 功能描述  : 软件异常记录接口，完成异常文件记录之后主动复位单板
*
* 输入参数  :
*
* 输出参数  : 无
*
* 返 回 值  : 无
*
* 其它说明  : kernel
*
*****************************************************************************/
void rdr_system_error (unsigned int modid, unsigned int arg1, unsigned int arg2,  char * data, unsigned int length);

/*****************************************************************************
* 函 数 名  : rdr_syserr_process_for_ap
*
* 功能描述  : 当已注册到rdr上的异常发生时，需要调用此接口触发保存log和reset流程。此接口供AP使用。
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*
* 其它说明  : kernel
*
*****************************************************************************/
void rdr_syserr_process_for_ap(unsigned int modid, unsigned long long arg1, unsigned long long arg2);

/*
void top_tmc_enable(void);

void top_tmc_disable(void);

unsigned int rdr_register_soc_ops (RDR_U64 coreid, struct rdr_soc_ops_pub* ops,
        struct rdr_register_soc_result* retinfo);
unsigned long long rdr_unregister_soc_ops_info (RDR_U64 coreid);
*/


/*************************************************************************
*
*   eFuse
*
*************************************************************************/

/*****************************************************************************
* 函 数 名  : get_efuse_dieid_value
*
* 功能描述  : 返回die id 的值
*
* 输入参数  : u32Length 以字节为单位，并且要求最小是4个字节
			  timeout   超时时间
*
* 输出参数  : pu8Buffer 存储读取efuse的值
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int get_efuse_dieid_value(unsigned char *pu8Buffer, unsigned int u32Length, unsigned int timeout);

/*****************************************************************************
* 函 数 名  : get_efuse_chipid_value
*
* 功能描述  : 返回chip id 的值
*
* 输入参数  : u32Length 以字节为单位，并且要求最小是4个字节
			  timeout   超时时间
*
* 输出参数  : pu8Buffer 存储读取efuse的值
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int get_efuse_chipid_value(unsigned char *pu8Buffer, unsigned int u32Length, unsigned int timeout);

/*****************************************************************************
* 函 数 名  : get_efuse_kce_value
*
* 功能描述  : 返回kce的值
*
* 输入参数  : u32Length 以字节为单位，并且要求最小是4个字节
			  timeout   超时时间
*
* 输出参数  : pu8Buffer 存储读取efuse的值
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int get_efuse_kce_value(unsigned char *pu8Buffer, unsigned int u32Length, unsigned int timeout);

/*****************************************************************************
* 函 数 名  : set_efuse_kce_value
*
* 功能描述  : 设置kce的值
*
* 输入参数  : pu8Buffer 存储读取efuse的值
			  u32Length 以字节为单位，并且要求最小是4个字节
			  timeout   超时时间
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int set_efuse_kce_value(unsigned char *pu8Buffer, unsigned int u32Length, unsigned int timeout);



/*************************************************************************
*
*   USB
*
*************************************************************************/


/* IOCTL CMD 定义 */
#define ACM_IOCTL_SET_WRITE_CB      0x7F001000
#define ACM_IOCTL_SET_READ_CB       0x7F001001
#define ACM_IOCTL_SET_EVT_CB        0x7F001002
#define ACM_IOCTL_SET_FREE_CB       0x7F001003

#define ACM_IOCTL_WRITE_ASYNC       0x7F001010
#define ACM_IOCTL_GET_RD_BUFF       0x7F001011
#define ACM_IOCTL_RETURN_BUFF       0x7F001012
#define ACM_IOCTL_RELLOC_READ_BUFF  0x7F001013
#define ACM_IOCTL_SEND_BUFF_CAN_DMA 0x7F001014

#define ACM_IOCTL_IS_IMPORT_DONE    0x7F001020
#define ACM_IOCTL_WRITE_DO_COPY     0x7F001021

#define ACM_MODEM_IOCTL_SET_MSC_READ_CB 0x7F001030
#define ACM_MODEM_IOCTL_MSC_WRITE_CMD   0x7F001031
#define ACM_MODEM_IOCTL_SET_REL_IND_CB  0x7F001032

/* 异步数据收发结构 */
typedef struct tagACM_WR_ASYNC_INFO
{
    char *pVirAddr;
    char *pPhyAddr;
    unsigned int u32Size;
    void* pDrvPriv;
}ACM_WR_ASYNC_INFO;

/* ACM设备事件类型 */
typedef enum tagACM_EVT_E
{
    ACM_EVT_DEV_SUSPEND = 0,        /* 设备不可以进行读写(主要用于事件回调函数的状态) */
    ACM_EVT_DEV_READY = 1,          /* 设备可以进行读写(主要用于事件回调函数的状态) */
    ACM_EVT_DEV_BOTTOM
}ACM_EVT_E;

typedef enum tagACM_IOCTL_FLOW_CONTROL_E
{
    ACM_IOCTL_FLOW_CONTROL_DISABLE = 0,      /* resume receiving data from ACM port */
    ACM_IOCTL_FLOW_CONTROL_ENABLE      /* stop receiving data from ACM port */
}ACM_IOCTL_FLOW_CONTROL_E;

/* 读buffer信息 */
typedef struct tagACM_READ_BUFF_INFO
{
    unsigned int u32BuffSize;
    unsigned int u32BuffNum;
}ACM_READ_BUFF_INFO;


typedef void (*ACM_WRITE_DONE_CB_T)(char *pVirAddr, char *pPhyAddr, int size);
typedef void (*ACM_READ_DONE_CB_T)(void);
typedef void (*ACM_EVENT_CB_T)(ACM_EVT_E evt);
typedef void (*ACM_FREE_CB_T)(char* buf);
typedef void (*ACM_MODEM_REL_IND_CB_T)(unsigned int bEnable);

typedef void (*USB_UDI_ENABLE_CB_T)(void);
typedef void (*USB_UDI_DISABLE_CB_T)(void);


/*****************************************************************************
* 函 数 名  : bsp_acm_open
*
* 功能描述  : 打开设备
*
* 输入参数  : dev_id 要打开的设备id
*
* 输出参数  : 无
*
* 返 回 值  : 非0 返回的handle
			  NULL 错误
*
* 其它说明  : kernel
*
*****************************************************************************/
void *bsp_acm_open(unsigned int dev_id);

/*****************************************************************************
* 函 数 名  : bsp_acm_close
*
* 功能描述  : 关闭设备
*
* 输入参数  : 要关闭的设备handle
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_acm_close(void * handle);

/*****************************************************************************
* 函 数 名  : bsp_acm_write
*
* 功能描述  : 数据写
*
* 输入参数  : handle 设备handle
			  buf  待写入的数据
			  size 代写入的数据大小
*
* 输出参数  : 无
*
* 返 回 值  : >0  处理成功的数据大小
			  <=0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_acm_write(void * handle, void *buf, unsigned int size);

/*****************************************************************************
* 函 数 名  : bsp_acm_read
*
* 功能描述  : 数据写
*
* 输入参数  : handle 设备handle
			  size 代写入的数据大小
*
* 输出参数  : buf  待返回的数据
*
* 返 回 值  : >0  处理成功的数据大小
			  <=0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_acm_read(void * handle, void *buf, unsigned int size);

/*****************************************************************************
* 函 数 名  : bsp_acm_ioctl
*
* 功能描述  : 根据命令码进行设置
*
* 输入参数  : handle 设备handle
			  cmd 命令码
			  para 命令参数
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int bsp_acm_ioctl(void * handle, unsigned int cmd, void *para);

/*****************************************************************************
* 函 数 名  : BSP_USB_RegUdiEnableCB
*
* 功能描述  : 注册usb枚举完成回调
*
* 输入参数  : pFunc 回调指针
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
unsigned int BSP_USB_RegUdiEnableCB(USB_UDI_ENABLE_CB_T pFunc);

/*****************************************************************************
* 函 数 名  : BSP_USB_RegUdiDisableCB
*
* 功能描述  : 注册usb拔出完成回调
*
* 输入参数  : pFunc 回调指针
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
unsigned int BSP_USB_RegUdiDisableCB(USB_UDI_DISABLE_CB_T pFunc);


/*************************************************************************
*
*   PMU
*
*************************************************************************/


typedef  void   (*PMU_OCP_CALLBACK)( const char * name);
/*****************************************************************************
* 函 数 名  : BSP_USB_RegUdiDisableCB
*
* 功能描述  : 注册usb拔出完成回调
*
* 输入参数  : name OCP产生的电源名称，见pmu_mntn DTS定义
			  pFunc
*
* 输出参数  : 无
*
* 返 回 值  : 0  处理成功
			  <0 处理失败
*
* 其它说明  : kernel
*
*****************************************************************************/
int pmu_ocp_event_reg (const char * name, PMU_OCP_CALLBACK pFunc);

enum EDITION_KIND{
    EDITION_USER            = 1,
    EDITION_INTERNAL_BETA   = 2,
    EDITION_OVERSEA_BETA    = 3,
    EDITION_MAX
};
/*
 * func name: bbox_check_edition
 * func args:
 *   void
 * return:
 *   unsigned int:	return edition information
 *				0x01		User
 *				0x02		Internal BETA
 *				0x03		Oversea BETA
 */
unsigned int bbox_check_edition(void);

#endif /* HISI_AP_DRV_H */

