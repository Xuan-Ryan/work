#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// 子執行緒函數
void *child(void *data)
{
    int i;
    //char *str = (char *)data; // 取得輸入資料
    for (i = 0; i < 3; ++i)
    {
        printf("%d\n", data); // 每秒輸出文字
        sleep(1);
    }
    pthread_exit(NULL); // 離開子執行緒
}

// 主程式
int main()
{
    int i;
    pthread_t t;                                // 宣告 pthread 變數
    pthread_create(&t, NULL, child, (void *)1); // 建立子執行緒

    // 主執行緒工作
    for (i = 0; i < 3; ++i)
    {
        printf("Master\n"); // 每秒輸出文字
        sleep(1);
    }

    pthread_join(t, NULL); // 等待子執行緒執行完成
    return 0;
}