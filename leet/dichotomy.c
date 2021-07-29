#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* 
 * dichtomy search the target index
 * importaint point
 * half = left + (right - left) / 2
 * letf = half + 1;
 */
int search(int* nums, int numsSize, int target){
    int left = 0;
    int right = numsSize-1;
    
    while(left < right) {
        int half = left + (right - left)/2;

        if (target < nums[half]) {
            right = half;
        }else if (target > nums[half]) {
            left = half + 1;
        } else {
            return half;
        }
    }
    /*
     * just target in the list 
     *
    if(target == nums[left]) return left;
    return -1; */
    
    /* include target not in the list will
     * return the insert index of the target
     */
    if(target <= nums[left]) return left;
    else return left + 1;
}


int main() {
    int nums[6] = {-1,0,3,5,9,12};
    printf("%d %d\n", sizeof(nums), strlen(nums));
    int target = 9;
    printf("%d\n", search(nums, 6, target));
}
