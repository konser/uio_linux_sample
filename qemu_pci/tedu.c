#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <sys/mman.h>


#define EDU_IO_SIZE 0x100
#define EDU_CARD_VERSION_ADDR  0x1
#define EDU_CARD_LIVENESS_ADDR 0x02
//#define EDU_RAISE_INT_ADDR 0x18
//#define EDU_CLEAR_INT_ADDR 0x19
//#define EDU_CARD_LIVENESS_ADDR 0x2
#define EDU_RAISE_INT_ADDR 0x60
#define EDU_CLEAR_INT_ADDR 0x64

int main()
{
    int uiofd;
    int configfd;
    int bar0fd;
    int resetfd;
    int err;
    int i;
    unsigned icount;
    unsigned char command_high;
    volatile uint32_t *bar0;

    uiofd = open("/dev/uio0", O_RDWR);
    if (uiofd < 0) {
        perror("uio open:");
        return errno;
    }

    configfd = open("/sys/class/uio/uio0/device/config", O_RDWR);
    if (configfd < 0) {
        perror("config open:");
        return errno;
    }

    /* Read and cache command value */
    err = pread(configfd, &command_high, 1, 5);
    if (err != 1) {
        perror("command config read:");
        return errno;
    }
    command_high &= ~0x4;

    /* Map edu's MMIO */
    bar0fd = open("/sys/class/uio/uio0/device/resource0", O_RDWR);
    if (bar0fd < 0) {
        perror("bar0fd open:");
        return errno;
    }

    /* mmap the device's BAR */
    bar0 = (volatile uint32_t *)mmap(NULL, EDU_IO_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, bar0fd, 0);
    if (bar0 == MAP_FAILED) {
        perror("Error mapping bar0!");
        return errno;
    }
    fprintf(stdout,  "0x%p Version = 0x%08x\n", bar0, bar0[EDU_CARD_VERSION_ADDR]);

    /* Test the invertor function */
    i = 0x4;
    //bar0[EDU_CARD_LIVENESS_ADDR] = i;
    //fprintf(stdout, "Inversion: %08X --> %08X\n", i, bar0[EDU_CARD_LIVENESS_ADDR]);

    /* Clear previous interrupt */
    bar0[EDU_CLEAR_INT_ADDR] = 0xABCDABCD;

    /* Raise an interrupt */
    bar0[EDU_RAISE_INT_ADDR] = 0xABCDABCD;

    for(i = 0; i<5; ++i) {
        /* Print out a message, for debugging. */
        if (i == 0)
            fprintf(stderr, "Started uio test driver.\n");
        else
            fprintf(stderr, "Interrupts: %d\n", icount);

        /****************************************/
        /* Here we got an interrupt from the
           device. Do something to it. */
        /****************************************/

        /* Re-enable interrupts. */
        err = pwrite(configfd, &command_high, 1, 5);
        if (err != 1) {
            perror("config write:");
            break;
        }

        /* Clear previous interrupt */
        //bar0[EDU_CLEAR_INT_ADDR] = 0xABCDABCD;

        /* Raise an interrupt */
        //bar0[EDU_RAISE_INT_ADDR] = 0xABCDABCD;

        /* Wait for next interrupt. */
        err = read(uiofd, &icount, 4);
        if (err != 4) {
            perror("uio read:");
            break;
        }

    }
    return errno;
}
