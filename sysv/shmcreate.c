#include "common.h"
/*
 * touch /home/t1; ./shmcreate 1024 /home/t1 1
 * touch /home/t2; ./shmcreate 1024 /home/t2 1
 */
int main(int argc, char **argv)
{
    int length = atoi(argv[1]);
    char *file = argv[2];
    int id = atoi(argv[3]);
    int oflag = IPC_CREAT | SHM_RW_PERMISSION;
    //int shmid = shmget(ftok(FTOK_FILE, FTOK_ID), length, oflag);
    int shmid = shmget(ftok(file, id), length, oflag);

    if (shmid >= 0)
    {
        printf("shmget create success, shmid = %d file = %s\n", shmid, file);
    }

    return 0;
}
