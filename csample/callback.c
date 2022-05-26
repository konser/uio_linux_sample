#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef int32_t (wlm_cb_func) (uint32_t *t1);

static int32_t get_value(uint32_t *t1) {
	*t1 = 88;
	printf("%d\n", *t1);
}

struct wlm_op {
		wlm_cb_func *cb;
};

int add_op(int a, int b) { return a+b; } 
int sub_op(int a, int b) { return a-b; } 
int mul_op(int a, int b) { return a*b; } 
int div_op(int a, int b) { return a/b; }

struct wlm_ops{
	int (*add)(int a, int b); 
	int (*sub)(int a, int b);  
	int (*mul)(int a, int b);
	int (*div)(int a, int b);  
};

static op_register(struct wlm_ops *ops) {
	ops->add = add_op;
	ops->sub = sub_op;
	ops->mul = mul_op;
	ops->div = div_op;
}	

int main()
{
		int a=8,b=2,result;
		struct wlm_ops function;
		memset(&function, 0, sizeof(function));
		/*
		 * callback register
		 */
		op_register(&function);
		/* 
		struct wlm_ops function = {
		    .add = &add_op,
		    .sub = &sub_op,
		    .mul = mul_op,
		    .div = div_op,
        };
		*/	
		result = function.add(a,b);
		printf("result is %d\n\r",result);
		result = function.sub(a,b);
		printf("result is %d\n\r",result);
		result = function.mul(a,b);
		printf("result is %d\n\r",result);
		result = function.div(a,b);
		printf("result is %d\n\r",result);
		/* 
		 * callback init
		 */
		struct wlm_op wop = {
			.cb = get_value,
		};
		int32_t t1 = 0;

		/*
		 * callback
		 */
		if(wop.cb)
			wop.cb(&t1);
}
