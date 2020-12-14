#include<stdio.h>
#include<string.h>
#include<signal.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<unistd.h>
int main(){
    int pid = fork();
    if(pid == 0){
        while(1){
            printf("Child: %d\n", getpid()), pid;
            sleep(1);
        }
    }
    else{
        int status;
        pid_t w;
        while(1){
            printf("Parent: %d %d\n", getpid(), pid);
            w = waitpid(pid, &status,WCONTINUED | WUNTRACED);
            /* WNOHANG will exit parent and return status=0 */
            //w = waitpid(pid, &status, WNOHANG);
            
            if(WIFEXITED(status)){ /* kill -SIGSTOP pid */
                printf("子进程正常退出，状态码: %d\n", WEXITSTATUS(status));
                exit(0);
            } else if(WIFSIGNALED(status)){ /* kill -SIGTERM/-SIGQUIT pid */
                printf("子进程被信号杀死了! 信号值: %d\n", WTERMSIG(status));
                exit(0);
            } else if(WIFSTOPPED(status)){ /* kill -SIGSTOP pid */
                printf("子进程被信号暂停了! 信号值: %d\n", WSTOPSIG(status));
            } else if(WIFCONTINUED(status)){ /* kill -SIGCONT pid */
                printf("子进程又恢复继续运行了\n");
            }
        }
    }
}
