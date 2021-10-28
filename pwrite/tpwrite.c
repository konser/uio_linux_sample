/* 
 * 调用pread相当于顺序调用lseek和read,但pread和这种调用又有重大区别：
 * 调用pread时，无法中断其定位和读操作，（lseek和read相当于一个原子操作）
 * 不更新文件指针
 * 因为lseek与read之间，可能会出现非预期的效果，所以定义pread。
 * 随机访问的话，pread/pwrite比较方便
 *
 * /

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
 
#define BUFFSIZE 256
int main(void)
{
    char        pathname[] = "/tmp/myfile";   /*待操作文件路径*/
    int         f_id;                         /*文件描述符*/
 
    off_t       f_offset;                     /*文件指针偏移量*/
 
    ssize_t     nwrite;                       /*实际写入的字节数*/
    char        buf[BUFFSIZE] = "0123456789abcd"; /*待写入数据*/
    size_t      nbytes;                       /*准备写入的字节数*/
 
/*打开文件，获取文件标识符*/
    f_id = open(pathname, O_RDWR | O_CREAT);
    if (f_id == -1) {
        printf("open error for %s\n", pathname);
        return 1;
    }
 
/*把文件指针移动到文件开始处*/
    f_offset = lseek(f_id, 0, SEEK_SET);
    if (f_offset == -1) {
        printf("lseek error for %s\n", pathname);
        return 2;
    }
 
/*写入7个字节数据[0-6]*/
    nbytes = 7;
    nwrite = write(f_id, buf, nbytes);
    if (nwrite == -1) {
        printf("write error for %s\n", pathname);
        return 3;
    }
 
/*=======调用pwrite从第一个字节后面写入四个字节数据[abcd]=======*/
    nbytes = 4;
    nwrite = pwrite(f_id, (buf + 10), nbytes, 1);
    if (nwrite == -1) {
        printf("pwrite error for %s\n", pathname);
        return 4;
    }
 
/*
 * 再写入3个字节数据[7-9], 还是在6后面
 * 说明pwirte不会改变文件指针
 * /
    nbytes = 3;
    nwrite = write(f_id, (buf + 7), nbytes); 
    if (nwrite == -1) {
        printf("write error for %s\n", pathname);
        return 5;  
    }
 
/*关闭文件*/
    close(f_id);
 
    return 0;
}
