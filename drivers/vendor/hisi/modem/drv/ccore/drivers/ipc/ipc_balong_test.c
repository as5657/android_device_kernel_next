#ifdef __KERNEL__
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#define delay(a) msleep(a)
#elif defined(__OS_VXWORKS__)
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <taskLib.h>
#define delay(a) taskDelay(a)
#elif defined(__OS_RTOSCK__)
#include <sre_task.h>

#define delay(a) SRE_TaskDelay(a)
#endif
#include <bsp_om.h>
#include <bsp_ipc.h>
#include <osl_module.h>
#include <osl_thread.h>
#include "ipc_balong.h"


u32 count_num[32] = {0};
#define  TEST_COUNT  10

#ifdef __KERNEL__
#define DEST_CORE IPC_CORE_CCORE
#define TEST_INT_NUM IPC_CCPU_INT_SRC_ACPU_TEST_ENABLE
#elif defined(__OS_VXWORKS__) || defined(__OS_RTOSCK__)
#define DEST_CORE IPC_CORE_ACORE
#define TEST_INT_NUM IPC_ACPU_INT_SRC_CCPU_TEST_ENABLE
#endif
s32 ipc_test_case01(void);
s32 ipc_test_case02(void);
s32 ipc_test_case03(void);
s32 ipc_test_case04(void);
s32 ipc_test_case05(void);
s32 ipc_test_case06(void);
s32 bsp_ipc_test_init(void);
void ipc_func1(void);
void bsp_for_test_mcore(u32 n);
s32 bsp_ipc_test_to_m(void);
s32 bsp_ipc_test_from_m(void);
s32 bsp_ipc_test_spinlock(void);
s32 test_take_ipc_sem(void);
s32 ipc_test_spinlock(void);


static void bsp_for_intenable_handler(u32 n)
{
	if(MDRV_OK!= bsp_ipc_sem_take(IPC_SEM1_IPC_TEST,0))
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"int handler take  error\n");
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"int handler take  ok n = %d\n", n);
}
/********************************ST BEGIN****************************************/


s32 ipc_test_case01(void)
{
	s32 ret=0;
	ret = bsp_ipc_sem_create(IPC_SEM2_IPC_TEST);
	if(MDRV_OK!=ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s], bsp_ipc_sem_create error\n",__FUNCTION__);
		return MDRV_ERROR;
	}
	ret = bsp_ipc_sem_take(IPC_SEM2_IPC_TEST,10);
	if(MDRV_OK!=ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s], bsp_ipc_sem_take error\n",__FUNCTION__);
		return MDRV_ERROR;
	}
	ret = bsp_ipc_sem_give(IPC_SEM2_IPC_TEST);
	if(MDRV_OK!=ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s], sem_give error\n",__FUNCTION__);
		return MDRV_ERROR;
	}
	ret = bsp_ipc_int_send(DEST_CORE,TEST_INT_NUM);
	if(MDRV_OK!=ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s], int send error\n",__FUNCTION__);
		return MDRV_ERROR;
	}
	delay(100);	
	ret = bsp_ipc_sem_take(IPC_SEM2_IPC_TEST,10);
	if(MDRV_OK!=ret)
	{
		ret = bsp_ipc_sem_delete(IPC_SEM2_IPC_TEST);
		if(MDRV_OK!=ret)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s], bsp_ipc_sem_delete error\n",__FUNCTION__);
			return MDRV_ERROR;
		}
		return MDRV_OK;
	}
	else
	{
		ret = bsp_ipc_sem_delete(IPC_SEM2_IPC_TEST);
		if(MDRV_OK!=ret)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s], bsp_ipc_sem_delete error\n",__FUNCTION__);
			return MDRV_ERROR;
		} 
		return MDRV_ERROR;
	}
}

/*测试多次take和give同一个信号量*/
s32 ipc_test_case02(void)
{
	s32 ret = 0,i = 0;
	ret = bsp_ipc_sem_give(IPC_SEM1_IPC_TEST);
	if(MDRV_OK!=ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s], sem_give error\n",__FUNCTION__);
		return ret;
	}
	ret = bsp_ipc_sem_create(IPC_SEM1_IPC_TEST);
	if(MDRV_OK!=ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s]: sem_create error\n",__FUNCTION__);
		return MDRV_ERROR;
	}
	for(i=0;i<TEST_COUNT;i++)
	{
		ret = bsp_ipc_sem_take(IPC_SEM1_IPC_TEST,0);
		if(MDRV_OK!=ret)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s]: sem_take error,loop i=%d\n",__FUNCTION__,i);
			return MDRV_ERROR;
		}
		ret = bsp_ipc_sem_give(IPC_SEM1_IPC_TEST);
		if(MDRV_OK!=ret)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s]: sem_give error,loop i=%d\n",__FUNCTION__,i);
			return MDRV_ERROR;
		}
	}
	ret = bsp_ipc_sem_delete(IPC_SEM1_IPC_TEST);
	if(MDRV_OK!=ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s], sem_delete error\n",__FUNCTION__);
		return MDRV_ERROR;
	}
	return MDRV_OK;
}

/*测试bsp_ipc_spin_lock和bsp_ipc_spin_unlock函数实现*/

s32 ipc_test_case03(void)
{
	u32 ret = 0;
	unsigned long flags;
	local_irq_save(flags);
	ret = bsp_ipc_spin_lock(IPC_SEM1_IPC_TEST);
	if(ret != MDRV_OK)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s]:spinlock error\n",__FUNCTION__);
		local_irq_restore(flags);
		return MDRV_ERROR;
	}
	ret = bsp_ipc_spin_unlock(IPC_SEM1_IPC_TEST);
	if(ret != MDRV_OK)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s]:spinunlock error\n",__FUNCTION__);
		local_irq_restore(flags);
		return MDRV_ERROR;
	}
	local_irq_restore(flags);
	return MDRV_OK;
}

s32 bsp_ipc_test_init(void)
{
	s32 ret = 0;
	ret = bsp_ipc_int_connect(30,bsp_for_intenable_handler,0);
	if(ret !=MDRV_OK)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"bsp_ipc_test_init connect error\n");
		return MDRV_ERROR;
	}
	ret = bsp_ipc_int_enable(30);
	if(ret != MDRV_OK)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"bsp_ipc_test_init enable int  error\n");
		return MDRV_ERROR;
	}
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"bsp_ipc_test_init ok\n");
	return MDRV_OK;
}

void ipc_func1(void)
{
	s32 ret = 0;
	ret = bsp_ipc_sem_create(IPC_SEM1_IPC_TEST);
	if(ret!=MDRV_OK)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_func1 bsp_ipc_sem_create fail\n");
	}
	while(1)
	{
		delay(2);		
		ret = bsp_ipc_sem_take(IPC_SEM1_IPC_TEST,100);
		if(ret==MDRV_OK)
		{
			delay(2);
			ret = bsp_ipc_sem_give(IPC_SEM1_IPC_TEST);
		}
		else
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc task1 take fail\n");
		}	
	}
}
/*A核和C核各启动一个任务，同时抢占同一个信号量*/
s32 test_take_ipc_sem(void)
{
	s32 task1;
	u32  task_id = 0;
	u32  para = 0;
	
	task1	= osl_task_init("ipc_test1", 50, 0x2000, (void *)ipc_func1, (void *)&para, &task_id);
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"task1 = %d\n", task1);

	return MDRV_OK;
}

/**************************begin:测试C核与M3交互*****************************/
void bsp_for_test_mcore(u32 n)
{
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"test_mcore n = %d\n",n);
}

/*test m3 interact with c core*/
s32 bsp_ipc_test_to_m(void)
{
	(void)bsp_ipc_int_send(IPC_CORE_MCORE,IPC_MCU_INT_SRC_TEST);
	return MDRV_OK;
}
s32 bsp_ipc_test_from_m(void)
{
	(void)bsp_ipc_int_connect(0,bsp_for_test_mcore,31);
	(void)bsp_ipc_int_enable(0);
	return MDRV_OK;
}
s32 ipc_test_spinlock()
{
	u32 ret = 0;
	unsigned long flags;
	local_irq_save(flags);
	ret = bsp_ipc_spin_lock(IPC_SEM1_IPC_TEST);/*复用此信号量，测试与m3争用*/
	if(ret != MDRV_OK)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s]:spinlock error\n",__FUNCTION__);
		local_irq_restore(flags);
		return MDRV_ERROR;
	}
	delay(500);
	ret = bsp_ipc_spin_unlock(IPC_SEM1_IPC_TEST);
	if(ret != MDRV_OK)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"[%s]:spinunlock error\n",__FUNCTION__);
		local_irq_restore(flags);
		return MDRV_ERROR;
	}
	local_irq_restore(flags);
	return MDRV_OK;
}

/**************************end:测试C核与M3交互*****************************/

/************************** mdrv ipc 接口测试 ******************************/
/* 信号量创建，传入无效的信号量ID */
void ipc_api_sem_create_test_case_001(void)
{
    int          ret = 0;
	unsigned int semaphore_id = IPC_SEM_BUTTOM;

	ret = mdrv_ipc_sem_create(semaphore_id);
	if (MDRV_ERROR != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_create_test_case_001 fail\n");
		return ;
	}
	
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_create_test_case_001 ok\n");
	
    return ;
}

/* 信号量创建，传入有效的信号量ID */
void ipc_api_sem_create_test_case_002(void)
{
    int          ret = 0;
	unsigned int semaphore_id = 0;
	unsigned int fail_cnt = 0;
	unsigned int fail_id[32] = {0};
	unsigned int i = 0;

	for (semaphore_id = 0; semaphore_id < IPC_SEM_BUTTOM; semaphore_id++)
	{
		ret = mdrv_ipc_sem_create(semaphore_id);
		if (MDRV_OK != ret)
		{
            fail_cnt++;
			fail_id[fail_cnt - 1] = semaphore_id;
			continue ;
		}
	}

	if (0 != fail_cnt)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_create_test_case_002 fail fail_id:\n");
		for (i = 0; i < fail_cnt; i++)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"%d\n", fail_id[i]);
		}
		return ;
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_create_test_case_002 ok\n");
	
    return ;
}

/* 信号量删除，传入无效的信号量ID */
void ipc_api_sem_delete_test_case_001(void)
{
    int          ret = 0;
	unsigned int semaphore_id = IPC_SEM_BUTTOM;

	ret = mdrv_ipc_sem_delete(semaphore_id);
	if (MDRV_ERROR != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_delete_test_case_001 fail\n");
		return ;
	}
	
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_delete_test_case_001 ok\n");
	
    return ;
}

/* 信号量删除，传入有效的信号量ID */
void ipc_api_sem_delete_test_case_002(void)
{
    int          ret = 0;
	unsigned int semaphore_id = 0;
	unsigned int fail_cnt = 0;
	unsigned int fail_id[32] = {0};
	unsigned int i = 0;

	for (semaphore_id = 0; semaphore_id < IPC_SEM_BUTTOM; semaphore_id++)
	{
		ret = mdrv_ipc_sem_delete(semaphore_id);
		if (MDRV_OK != ret)
		{
            fail_cnt++;
			fail_id[fail_cnt - 1] = semaphore_id;
			continue ;
		}
	}

	if (0 != fail_cnt)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_delete_test_case_002 fail fail_id:\n");
		for (i = 0; i < fail_cnt; i++)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"%d\n", fail_id[i]);
		}
		return ;
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_delete_test_case_002 ok\n");
	
    return ;
}

/* 信号量释放，传入无效的信号量ID */
void ipc_api_sem_give_test_case_001(void)
{
    int          ret = 0;
	unsigned int semaphore_id = IPC_SEM_BUTTOM;

	ret = mdrv_ipc_sem_give(semaphore_id);
	if (MDRV_ERROR != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_give_test_case_001 fail\n");
		return ;
	}
	
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_give_test_case_001 ok\n");
	
    return ;
}

/* 信号量释放，传入有效的信号量ID */
void ipc_api_sem_give_test_case_002(void)
{
    int          ret = 0;
	unsigned int semaphore_id = 0;
	unsigned int fail_cnt = 0;
	unsigned int fail_id[32] = {0};
	unsigned int i = 0;

	for (semaphore_id = 0; semaphore_id < IPC_SEM_BUTTOM; semaphore_id++)
	{
		ret = mdrv_ipc_sem_give(semaphore_id);
		if (MDRV_OK != ret)
		{
            fail_cnt++;
			fail_id[fail_cnt - 1] = semaphore_id;
			continue ;
		}
	}

	if (0 != fail_cnt)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_give_test_case_002 fail fail_id:\n");
		for (i = 0; i < fail_cnt; i++)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"%d\n", fail_id[i]);
		}
		return ;
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_give_test_case_002 ok\n");
	
    return ;
}

/* 信号量获取，传入无效的信号量ID */
void ipc_api_sem_take_test_case_001(void)
{
    int          ret = 0;
	unsigned int semaphore_id = IPC_SEM_BUTTOM;

	ret = mdrv_ipc_sem_take(semaphore_id, 0);
	if (MDRV_ERROR != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_take_test_case_001 fail\n");
		return ;
	}
	
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_take_test_case_001 ok\n");
	
    return ;
}


/* 信号量获取，传入有效的信号量ID */
void ipc_api_sem_take_test_case_002(void)
{
    int          ret = 0;
	unsigned int semaphore_id = 0;
	unsigned int fail_cnt = 0;
	unsigned int fail_id[32] = {0};
	unsigned int i = 0;

	for (semaphore_id = 0; semaphore_id < IPC_SEM_BUTTOM; semaphore_id++)
	{
		ret = mdrv_ipc_sem_take(semaphore_id, 0);
		if (MDRV_OK != ret)
		{
            fail_cnt++;
			fail_id[fail_cnt - 1] = semaphore_id;
			continue ;
		}
	}

	if (0 != fail_cnt)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_take_test_case_002 fail fail_id:\n");
		for (i = 0; i < fail_cnt; i++)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"%d\n", fail_id[i]);
		}
		return ;
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_sem_take_test_case_002 ok\n");
	
    return ;
}

static unsigned int g_int_cnt[32] = {0};
/*  中断处理函数  */
void ipc_api_test_int_handler(unsigned int arg)
{
    g_int_cnt[arg]++;

    return ;
}

/* 打印各中断计数 */
void ipc_api_test_show_int_cnt(void)
{
    unsigned int i = 0;
	
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"IPC_CORE_BUTTOM = %d\n", IPC_CORE_BUTTOM);
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"IPC_INT_BUTTOM = %d\n", IPC_INT_BUTTOM);
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"IPC_SEM_BUTTOM = %d\n", IPC_SEM_BUTTOM);
	
	for (i = 0; i < 32; i++)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"g_int_cnt[%d]=%d\n", i, g_int_cnt[i]);
	}

    return ;
}

/* 中断挂接，传入无效的中断ID */
void ipc_api_int_connect_test_case_001(void)
{
    int          ret = 0;
	unsigned int int_id = IPC_INT_BUTTOM;

	ret = mdrv_ipc_int_connect(int_id, ipc_api_test_int_handler, int_id);
	if (MDRV_ERROR != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_connect_test_case_001 fail\n");
		return ;
	}
	
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_connect_test_case_001 ok\n");
	
    return ;
}

/* 中断挂接，传入有效的中断ID，将中断ID作为中断处理函数的参数 */
void ipc_api_int_connect_test_case_002(void)
{
    int          ret = 0;
	unsigned int int_id = 0;
	unsigned int fail_cnt = 0;
	unsigned int fail_id[32] = {0};
	unsigned int i = 0;

	for (int_id = 0; int_id < IPC_INT_BUTTOM; int_id++)
	{
		ret = mdrv_ipc_int_connect(int_id, ipc_api_test_int_handler, int_id);
		if (MDRV_OK != ret)
		{
            fail_cnt++;
			fail_id[fail_cnt - 1] = int_id;
			continue ;
		}
	}

	if (0 != fail_cnt)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_connect_test_case_002 fail fail_id:\n");
		for (i = 0; i < fail_cnt; i++)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"%d\n", fail_id[i]);
		}
		return ;
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_connect_test_case_002 ok\n");
	
    return ;
}


/* 中断断开，传入无效的中断ID */
void ipc_api_int_disconnect_test_case_001(void)
{
    int          ret = 0;
	unsigned int int_id = IPC_INT_BUTTOM;

	ret = mdrv_ipc_int_disconnect(int_id, ipc_api_test_int_handler, int_id);
	if (MDRV_ERROR != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_disconnect_test_case_001 fail\n");
		return ;
	}
	
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_disconnect_test_case_001 ok\n");
	
    return ;
}

/* 中断断开，传入有效的中断ID */
void ipc_api_int_disconnect_test_case_002(void)
{
    int          ret = 0;
	unsigned int int_id = 0;
	unsigned int fail_cnt = 0;
	unsigned int fail_id[32] = {0};
	unsigned int i = 0;

	for (int_id = 0; int_id < IPC_INT_BUTTOM; int_id++)
	{
		ret = mdrv_ipc_int_disconnect(int_id, ipc_api_test_int_handler, int_id);
		if (MDRV_OK != ret)
		{
            fail_cnt++;
			fail_id[fail_cnt - 1] = int_id;
			continue ;
		}
	}

	if (0 != fail_cnt)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_disconnect_test_case_002 fail fail_id:\n");
		for (i = 0; i < fail_cnt; i++)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"%d\n", fail_id[i]);
		}
		return ;
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_disconnect_test_case_002 ok\n");
	
    return ;
}

/* 中断使能，传入无效的中断ID */
void ipc_api_int_enable_test_case_001(void)
{
    int          ret = 0;
	unsigned int int_id = IPC_INT_BUTTOM;

	ret = mdrv_ipc_int_enable(int_id);
	if (MDRV_ERROR != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_enable_test_case_001 fail\n");
		return ;
	}
	
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_enable_test_case_001 ok\n");
	
    return ;
}

/* 中断使能，传入有效的中断ID */
void ipc_api_int_enable_test_case_002(void)
{
    int          ret = 0;
	unsigned int int_id = 0;
	unsigned int fail_cnt = 0;
	unsigned int fail_id[32] = {0};
	unsigned int i = 0;

	for (int_id = 0; int_id < IPC_INT_BUTTOM; int_id++)
	{
		ret = mdrv_ipc_int_enable(int_id);
		if (MDRV_OK != ret)
		{
            fail_cnt++;
			fail_id[fail_cnt - 1] = int_id;
			continue ;
		}
	}

	if (0 != fail_cnt)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_enable_test_case_002 fail fail_id:\n");
		for (i = 0; i < fail_cnt; i++)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"%d\n", fail_id[i]);
		}
		return ;
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_enable_test_case_002 ok\n");
	
    return ;
}

/* 中断非使能，传入无效的中断ID */
void ipc_api_int_disable_test_case_001(void)
{
    int          ret = 0;
	unsigned int int_id = IPC_INT_BUTTOM;

	ret = mdrv_ipc_int_disable(int_id);
	if (MDRV_ERROR != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_disable_test_case_001 fail\n");
		return ;
	}
	
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_disable_test_case_001 ok\n");
	
    return ;
}

/* 中断非使能，传入有效的中断ID */
void ipc_api_int_disable_test_case_002(void)
{
    int          ret = 0;
	unsigned int int_id = 0;
	unsigned int fail_cnt = 0;
	unsigned int fail_id[32] = {0};
	unsigned int i = 0;

	for (int_id = 0; int_id < IPC_INT_BUTTOM; int_id++)
	{
		ret = mdrv_ipc_int_disable(int_id);
		if (MDRV_OK != ret)
		{
            fail_cnt++;
			fail_id[fail_cnt - 1] = int_id;
			continue ;
		}
	}

	if (0 != fail_cnt)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_disable_test_case_002 fail fail_id:\n");
		for (i = 0; i < fail_cnt; i++)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"%d\n", fail_id[i]);
		}
		return ;
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_disable_test_case_002 ok\n");
	
    return ;
}

/* 中断发送，传入无效的核ID */
void ipc_api_int_send_test_case_001(void)
{
    int          ret = 0;
	unsigned int core_id = IPC_CORE_BUTTOM;  /* 无效的核ID */
	unsigned int int_id = 0;  /* 中断0 */

	ret = mdrv_ipc_int_send(core_id, int_id);
	if (MDRV_ERROR != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_send_test_case_001 fail\n");
		return ;
	}
	
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_send_test_case_001 ok\n");
	
    return ;
}

/* 中断发送，传入无效的中断ID */
void ipc_api_int_send_test_case_002(void)
{
    int          ret = 0;
	unsigned int core_id = IPC_CORE_ACORE;  /* A核 */
	unsigned int int_id = IPC_INT_BUTTOM;  /* 无效的中断ID */

	ret = mdrv_ipc_int_send(core_id, int_id);
	if (MDRV_ERROR != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_send_test_case_002 fail\n");
		return ;
	}
	
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_send_test_case_002 ok\n");
	
    return ;
}

/* 中断发送，传入有效的核ID，有效的中断ID */
void ipc_api_int_send_test_case_003(void)
{
    int          ret = 0;
	unsigned int core_id = IPC_CORE_ACORE;  /* C核 */
	unsigned int int_id = 0;
	unsigned int fail_cnt = 0;
	unsigned int fail_id[32] = {0};
	unsigned int i = 0;

	for (int_id = 0; int_id < IPC_INT_BUTTOM; int_id++)
	{
		ret = mdrv_ipc_int_send(core_id, int_id);
		if (MDRV_OK != ret)
		{
            fail_cnt++;
			fail_id[fail_cnt - 1] = int_id;
			continue ;
		}
	}

	if (0 != fail_cnt)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_send_test_case_003 fail fail_id:\n");
		for (i = 0; i < fail_cnt; i++)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"%d\n", fail_id[i]);
		}
		return ;
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_int_send_test_case_003 ok\n");
	
    return ;
}

/* 获取锁，传入无效的信号量ID */
void ipc_api_spin_lock_test_case_001(void)
{
	unsigned int semaphore_id = IPC_SEM_BUTTOM;

	mdrv_ipc_spin_lock(semaphore_id);
	
    return ;
}



/* 获取锁，传入有效的信号量ID */
void ipc_api_spin_lock_test_case_002(unsigned int semaphore_id)
{

	mdrv_ipc_spin_lock(semaphore_id);
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_spin_lock_test_case_002 id=%d spin_lock ok\n", semaphore_id);

    return ;
}

/* 尝试获取锁，传入无效的信号量ID */
void ipc_api_spin_trylock_test_case_001(void)
{
	unsigned int semaphore_id = IPC_SEM_BUTTOM;
	int          ret = -1;

	ret = mdrv_ipc_spin_trylock(semaphore_id);
	if (MDRV_ERROR != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_spin_trylock_test_case_001 fail\n");
		return ;
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_spin_trylock_test_case_001 ok\n");
	
    return ;
}

/* 尝试获取锁，传入有效的信号量ID */
void ipc_api_spin_trylock_test_case_002(unsigned int semaphore_id)
{
	int          ret = -1;

	ret = mdrv_ipc_spin_trylock(semaphore_id);
	if (MDRV_OK != ret)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_spin_trylock_test_case_002 sem_id %d fail\n", semaphore_id);
		return ;
	}

	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_spin_trylock_test_case_002 sem_id %d ok\n", semaphore_id);
	
    return ;
}


/* 释放锁，传入无效的信号量ID */
void ipc_api_spin_unlock_test_case_001(void)
{
	unsigned int semaphore_id = IPC_SEM_BUTTOM;

	mdrv_ipc_spin_unlock(semaphore_id);
	
    return ;
}

/* 释放锁，传入有效的信号量ID */
void ipc_api_spin_unlock_test_case_002(unsigned int semaphore_id)
{

	mdrv_ipc_spin_unlock(semaphore_id);
	bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_IPC,"ipc_api_spin_unlock_test_case_002 id=%d spin_unlock ok\n", semaphore_id);
	
    return ;
}





