/*
 * mmap 详解
 * https://www.cnblogs.com/huxiao-tee/p/4660352.html
 * points
 * size must the multiple Page-Size (4KB)
 *
 */
#include <sys/mman.h> /* for mmap and munmap */
#include <sys/types.h> /* for open */
#include <sys/stat.h> /* for open */
#include <fcntl.h>     /* for open */
#include <unistd.h>    /* for lseek and write */
#include <stdio.h>
#include <stdlib.h>
 
int main(int argc, char **argv)
{
    int fd;
    char *mapped_mem, * p;
    int flength = 1024;
    void *start_addr = "wlm";
    
    p = (char *)malloc(sizeof(char) * 10); 
    
    fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    flength = lseek(fd, 1, SEEK_END);
    write(fd, "\0", 1); /* 在文件最后添加一个空字符，以便下面printf正常工作 */
    lseek(fd, 0, SEEK_SET);
    mapped_mem = mmap(start_addr, flength, PROT_READ,        //允许读
    //mapped_mem = mmap(NULL, flength, PROT_READ,        //允许读
      	MAP_PRIVATE,       //不允许其它进程访问此内存区域
        	fd, 0);
    
    printf("%p %p %p %p\n", start_addr, &start_addr, "wlm", &"wlm");  
    /* 使用映射区域. mmap申请的地址在 stack 和 heap 之间 */
    printf("%s %p %p %p %p %p\n", mapped_mem, &fd, mapped_mem, p,  &mapped_mem, &p); /* 为了保证这里工作正常，参数传递的文件名最好是一个文本文件 */
    close(fd);
    munmap(mapped_mem, flength);
    free(p);
    return 0;
}
