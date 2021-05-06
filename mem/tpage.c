#include <stdio.h>
#include <stdlib.h>
#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE-1))

#define HOST_PAGE_ALIGN(addr) (((addr) + PAGE_SIZE -1) & PAGE_MASK)

int main() {
    printf("%lx\n", 0x200);
    printf("%lx\n", HOST_PAGE_ALIGN(0x20000));
    printf("%lx\n", HOST_PAGE_ALIGN(0x10000));
    printf("%lx\n", HOST_PAGE_ALIGN(0x40000));
}
