/*
 * # kill -9 9531
 * # ps -aux |grep 953
 * root      9530  0.0  0.0   4172   696 pts/0    S+   17:04   0:00 ./zom
 * root      9531  0.0  0.0      0     0 pts/0    Z+   17:04   0:00 [zom] <defunct>
 * 9531 become zombie status
 */

#include<stdio.h>
#include<string.h>
#include<signal.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<unistd.h>
int main(){
    int pid;
    if((pid = fork()) == 0){
        while(1){
            printf("Child: %d\n", getpid());
            sleep(1);
        }
    }
    else{
        int status;
        pid_t w;
        while(1){
            printf("Parent: %d\n", getpid());
            sleep(10);
        }
    }
}
