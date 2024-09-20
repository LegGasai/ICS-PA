#include <stdio.h>
#include <assert.h>
#include <NDL.h>

int main() {


    int cnt = 0;
    NDL_Init(0);
    uint32_t start = 0;
    while (1) {
        uint32_t now = NDL_GetTicks();
        if(start == 0 || now - start >= 1000){
            printf("Every 1 sec, this is %d th, now: %u\n", cnt++, now);
            start = now;
            if(cnt>=10) {
                break;
            }
        }
    }
    return 0;

}