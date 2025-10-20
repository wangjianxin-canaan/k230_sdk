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
#include "main.h"


#include "gui_guider.h"
// s

void create_detach_thread(thread_func_t func)
{
    pthread_t thread;
    // pthread_attr_t attr;
    // struct sched_param param;

    //pthread_attr_init(&attr);
    //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // pthread_attr_setstacksize(&attr, 4 * 1024);
    // pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    // param.sched_priority = 10;
    // pthread_attr_setschedparam(&attr, &param);
    pthread_create(&thread, NULL, func, NULL);
    //pthread_attr_destroy(&attr);
    pthread_detach(thread);

}




char get_uart_data(void)
{
    uint8_t data;
    if(ring_buffer_read(&uart_ring_buffer_rcv, &data)){
        return data;
    }
    return 0;
}
