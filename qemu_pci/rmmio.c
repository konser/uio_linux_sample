#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include<sys/io.h>


unsigned char* mmio_mem;

void die(const char* msg)
{
    perror(msg);
    exit(-1);
}



void mmio_write(uint32_t addr, uint32_t value)
{
    *((uint32_t*)(mmio_mem + addr)) = value;
}

uint32_t mmio_read(uint32_t addr)
{
    printf("0x%X : %08X\n", addr, *((uint32_t*)(mmio_mem + addr)));
}

int main(int argc, char *argv[])
{
    // Open and map I/O memory for the strng device
    int mmio_fd = open("/sys/devices/pci0000:00/0000:00:05.0/resource0", O_RDWR | O_SYNC);
    if (mmio_fd == -1)
        die("mmio_fd open failed");

    // map one page 4096(0x1000) byte
    mmio_mem = mmap(0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, mmio_fd, 0);
    if (mmio_mem == MAP_FAILED)
        die("mmap mmio_mem failed");

    printf("GVA is %p\n", mmio_mem);

    //get the edu 0x0 value
    mmio_read(0x0);

    // ~ operation
    //  printf %x `echo $((~0x222))`  ->  fffddd
    mmio_write(0x4, 0x222);
    mmio_read(0x4);

    // edu factorial computation 阶乘运算
    mmio_write(0x8, 0x4);
    mmio_read(0x8);

	// raise the irq
	mmio_write(0x60, 0x111);

    mmio_write(0x20, 0x1);
    mmio_read(0x20);

    mmio_write(0x20, 0x0);
    mmio_read(0x20);
	//mmio_write(0x64, 0x111);
}
