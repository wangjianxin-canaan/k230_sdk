#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <poll.h>
#include "ring_buffer.h"
#include <src/core/lv_global.h>

pthread_cond_t buffer_not_empty; // 条件变量：缓冲区不为空

extern bool flag_running;
#define	IOC_SET_BAUDRATE            _IOW('U', 0x40, int)

struct uart_configure
{
    rt_uint32_t baud_rate;

    rt_uint32_t data_bits               :4;
    rt_uint32_t stop_bits               :2;
    rt_uint32_t parity                  :2;
    rt_uint32_t fifo_lenth              :2;
    rt_uint32_t auto_flow               :1;
    rt_uint32_t reserved                :21;
};

typedef enum _uart_parity
{
    UART_PARITY_NONE,
    UART_PARITY_ODD,
    UART_PARITY_EVEN
} uart_parity_t;

typedef enum _uart_receive_trigger
{
    UART_RECEIVE_FIFO_1,
    UART_RECEIVE_FIFO_8,
    UART_RECEIVE_FIFO_16,
    UART_RECEIVE_FIFO_30,
} uart_receive_trigger_t;



void * uart_recv(void*)
{
    int fd;
    struct pollfd fds[1];
    int ret = 0, cnt = 0;



    printf(" [app] open uart2.....\n");
    fd = open("/dev/uart2", O_RDWR);
    if (fd < 0){
        printf("open dev uart2 failed!\n");
        return NULL;
    }

    struct uart_configure config = {
        .baud_rate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = UART_PARITY_NONE,
        .fifo_lenth = UART_RECEIVE_FIFO_16,
        .auto_flow = 0,
    };


    if (ioctl(fd, IOC_SET_BAUDRATE, &config)){
        printf("uart2 ioctl failed!\n");
    }

    //write(fd, send, 512);


    fds[0].fd = fd;
    fds[0].events = POLLIN;
    printf(" [app] ........... poll \n");

    while(flag_running){
        char buff[512];
        //pthread_mutex_lock(&mutex);
        if (poll(fds, 1, -1) > 0 && fds[0].revents & POLLIN){
            ret = read(fd, buff, sizeof(buff));
            if(ret > 0){
                for(int i = 0; i < ret; i++){
                    //printf("%c", buff[i] & 0xff);
                    ring_buffer_write(&uart_ring_buffer, buff[i] & 0xff );
                }
                pthread_cond_signal(&buffer_not_empty);
            }
        }
    }

    close(fd);
    pthread_cond_destroy(&buffer_not_empty);
    pthread_exit(NULL);
}

//pthread_create();
int uart_init(void)
{
    pthread_cond_init(&buffer_not_empty, NULL);
    ring_buffer_init(&uart_ring_buffer);
    create_detach_thread(uart_recv);
    create_detach_thread((thread_func_t)proc_uart_data);
}
