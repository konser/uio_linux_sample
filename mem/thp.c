#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>

#define LOOP_COUNT 100
#define MEMBLK_SIZE (400 * 1024 * 1024)

void * thp_map() {
    int rc = 1;
    int i, j;
    //void *ptr = MAP_FAILED;
    unsigned long  *ptr = MAP_FAILED;


        //ptr = mmap(NULL, MEMBLK_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
        ptr = mmap(NULL, MEMBLK_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) {
            perror("mmap");
        }

        //the kernel will not allocate memory without this line
        //*(char *) ptr = 1;

        //if (0 != mprotect(ptr, MEMBLK_SIZE, PROT_READ | PROT_WRITE)) {
        //    perror("mprotect");
        //}

        if (MAP_FAILED == ptr) {
            perror("mprotect");
            munmap(ptr, MEMBLK_SIZE);
	}
	for (j=0; j < MEMBLK_SIZE/sizeof(*ptr); j++) {
		*(ptr+j) = 10;
	}
	
  	while(1);
  	//sleep(2);
        //munmap(ptr, MEMBLK_SIZE);
    	//pthread_exit(0);
}

int main()
{
  int thread_num = 20;
  int i;

  while(1) {
  for(i = 0; i < thread_num; i++){
    pthread_t p;
    pthread_create(&p, NULL, thp_map, NULL);
    pthread_detach(p);
  }
  sleep(10000);
  }
}
