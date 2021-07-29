/*
 * f(1)=1       1
 * f(2)=1+1=2   2   
 * f(3)=1+1+1=1+2=2+1   3  = f(2)+f(1)
 * f(4)=1*4=2*2=2_1_1=1_2_1=1+1+2 5 =f(3)+f(2)
 * f(n)=f(n-1)+f(n-2)
 */
#include <stdio.h>
#include <stdlib.h>

int climbStairs(int n) {
    int p = 0, q = 0, r = 1;
    for (int i=1; i<=n; i++) {
        p = q;
        q = r;
        r = p + q;
    }
    return r;
}

int climb2(int cnt) {
    int ret = 0;
    if (cnt == 1) return 1;
    else if (cnt == 2) return 2;
    else  ret += climb2(cnt-1) + climb2(cnt -2);
    return ret;
}

int main() {
    char in[20];
    gets(in);
    printf("%d\n", climbStairs(atoi(in)));
    printf("%d\n", climb2(atoi(in)));
    return 1;
}
