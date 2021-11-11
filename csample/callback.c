#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int add(int a, int b) { return a+b; } 
int sub(int a, int b) { return a-b; } 
int mul(int a, int b) { return a*b; } 
int div(int a, int b) { return a/b; }

struct operations{
	int (*ADD)(int a, int b); 
	int (*SUB)(int a, int b);  
	int (*MUL)(int a, int b);
	int (*DIV)(int a, int b);  
};

int main()
{
		int a=4,b=1,result;
		struct operations FUN = {
		    .ADD = &add,
		    .SUB = &sub,
		    .MUL = mul,
		    .DIV = div,
        };
	
		result = FUN.ADD(a,b);
		printf("result is %d\n\r",result);
		result = FUN.SUB(a,b);
		printf("result is %d\n\r",result);
		result = FUN.MUL(a,b);
		printf("result is %d\n\r",result);
		result = FUN.DIV(a,b);
		printf("result is %d\n\r",result);
}
