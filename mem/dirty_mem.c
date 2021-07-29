#include <stdlib.h>
#include <stdio.h>

int main()
{
    long int buf_length;
    long int buf_num;
    char c[100];
    buf_length = 4096;
    int compare_result;
 
 
    printf("Enter memory you want to test: ");
    gets(c);
    if (strcmp(c,"128m")==0)
        compare_result = 1;
    else if(strcmp(c,"256m")==0)
        compare_result = 2;
    else if(strcmp(c,"512m")==0)
        compare_result = 3;
    else if(strcmp(c,"1g")==0)
        compare_result = 4;
    else if(strcmp(c,"2g")==0)
        compare_result = 5;
    else
        compare_result = 0;
 
    switch(compare_result)
        {
        case 1:
                buf_num = 32768;
                printf("start generate 128M memory r/w load, ctrl+c to quit. \n");
                break;
        case 2:
                buf_num = 65536;
                printf("start generate 256M memory r/w load, ctrl+c to quit. \n");
                break;
        case 3:
                buf_num = 131072;
                printf("start generate 512M memory r/w load, ctrl+c to quit. \n");
                break;
        case 4:
                buf_num = 262144;
                printf("start generate 1G memory r/w load, ctrl+c to quit. \n");
                break;
        case 5:
                buf_num = 524288;
                printf("start generate 2G memory r/w load, ctrl+c to quit. \n");
                break;
        default:
                buf_num = 0;
                printf("please input following size 128m, 256m, 512, 1g, 2g. ");
                break;
        }
 
    if(buf_num!=0){
        printf("use free -h to monitor ");
        char *buf = (char *) calloc(buf_num, buf_length);
        while (1) {
                long int i;
                for (i = 0; i < buf_num * 4 ; i++) {
                 buf[i * buf_length / 4 ]++;
                }
                printf(".");
        }
    }
    else{
        printf("please input correct size. \n");
    }
}
