#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
 
#define    ERR_DBG_PRINT(fmt, args...) \
    do \
    { \
        printf("ERR_DBG:%s(%d)-%s:\n"fmt": %s\n", __FILE__,__LINE__,__FUNCTION__,##args, strerror(errno)); \
    } while (0)
 
 
/* mmap方式可以使用指定尺寸的巨页池 */
int main(int argc, char *argv[])
{
    printf("Welcome to mmap example:%d\n", getpid());
    //int ret = system("mount -t hugetlbfs none /wlm/hugetlb_test -o pagesize=2048K");
    //int ret = system("mount -t hugetlbfs hugetlbfs /wlm/hugetlb_test -o pagesize=2048K");
    //int ret1 = system("echo 10000 >  /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages");
    int fd = open("/wlm/hugetlb_test/hehe.txt", O_CREAT | O_RDWR, 0755);
    char *huge_mem = mmap(NULL, 2048*1024*5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    //char *huge_mem = mmap(NULL, 2048*1024*5, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, fd, 0);
 
 
    //if (ret<0)
    //{
    //    ERR_DBG_PRINT("mount fail:");
    //    return 0;
    //}
 
    if (fd<0)
    {
        ERR_DBG_PRINT("open fail:");
        return 0;
    }
 
    if (-1 == (int)(unsigned long)huge_mem)
    {
        ERR_DBG_PRINT("mmap fail:");
        return 0;
    }
 
    printf("Program Allocate :%p\n", huge_mem);
    int fd1 = open("/wlm/hugetlb_test/h1.txt", O_CREAT | O_RDWR, 0755);
    char *huge_mem1 = mmap(NULL, 2048*1024*5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    printf("Program Allocate :%p\n", huge_mem1);
    printf("press any key to access huge page\n");
    getchar();
 
 
    memset(huge_mem, 0, 2048*5);
 
    printf("press any key to delete huge page\n");
    getchar();
 
    /* 下面代码一执行，内存就释放回巨页池了 */
    if (unlink("/wlm/hugetlb_test/hehe.txt")==-1)
        ERR_DBG_PRINT("unlink fail:");
 
    if (unlink("/wlm/hugetlb_test/h1.txt")==-1)
        ERR_DBG_PRINT("unlink fail:");
    return 0;
}
