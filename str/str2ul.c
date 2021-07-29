#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
 * para1:  source
 * para2: 返回字符串有效数字的结尾地址。如 123456fe789 则返回数字f的地址。
 * para3: source数字的进制 
 *        0 时，默认采用 10 进制转换，但如果遇到 '0x' / '0X' 前置字符则会使用 16 进制转换，
 *        遇到 '0' 前置字符则会使用 8 进制转换。
 * ret: 转换后的ul 数字， 失败则返回 0
 */
int main()
{
    char str[6];
    printf("input the str (length %d %d):\n", sizeof(str), strlen(str));
    fgets(str, sizeof(str), stdin); 
    //char str[30] = "2030300 This is test";
    char *endptr;
    long ret;
 
    ret = strtoul(str, &endptr, 0);
    printf("ul output is %lu\n", ret);
    printf("invalid character is %s\n", endptr);
 
    return(0);
}
