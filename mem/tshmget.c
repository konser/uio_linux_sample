#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
 
#define    ERR_DBG_PRINT(fmt, args...) \
    do \
    { \
        printf("ERR_DBG:%s(%d)-%s:\n"fmt": %s\n", __FILE__,__LINE__,__FUNCTION__,##args, strerror(errno)); \
    } while (0)
 
 
/* shmget方式只能使用默认尺寸的巨页池 */
int main(int argc, char *argv[])
{
    key_t  our_key = ftok("/wlm/hugetlb_test/test", 6);
    int shm_id = shmget(our_key, 2048*1024*5, IPC_CREAT|IPC_EXCL|SHM_HUGETLB);
    char *huge_mem = shmat(shm_id, NULL, 0);
 
 
    if (-1 == (int)our_key)
    {
        ERR_DBG_PRINT("ftok fail:");
        return 0;
    }
 
    if (-1 == shm_id)
    {
        ERR_DBG_PRINT("shmget fail:");
        return 0;
    }
 
    if (-1 == (int)(unsigned long)huge_mem)
    {
        ERR_DBG_PRINT("shmat fail:");
        return 0;
    }
 
    printf("press any key to access huge page\n");
    getchar();
 
 
    memset(huge_mem, 0, 2048*1024*5);
 
    printf("press any key to delete huge page\n");
    getchar();
 
 
    /* 下面代码一执行，内存就释放回巨页池了 */
    shmdt(huge_mem);
    if (shmctl(shm_id, IPC_RMID, NULL)==-1)
        ERR_DBG_PRINT("shmctl fail:");
 
    return 0;
}
