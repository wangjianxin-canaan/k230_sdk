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
#include "gui_guider.h"
// s

extern pthread_cond_t buffer_not_empty;
extern lv_ui guider_ui;
extern bool flag_running;
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

void data_2_ui(char data)
{
    switch (data) {
        case 'a':
            ui_load_scr_animation(&guider_ui, &guider_ui.mode, guider_ui.mode_del, &guider_ui.home_del, setup_scr_mode, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, false, true);
            lv_obj_set_tile(guider_ui.mode_tileview, guider_ui.mode_tileview_mode_a, LV_ANIM_OFF);
            break;
        case 'f':
            ui_load_scr_animation(&guider_ui, &guider_ui.mode, guider_ui.mode_del, &guider_ui.home_del, setup_scr_mode, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, false, true);
            lv_obj_set_tile(guider_ui.mode_tileview, guider_ui.mode_tileview_mode_f, LV_ANIM_OFF);
            break;
        case 'e':
            ui_load_scr_animation(&guider_ui, &guider_ui.mode, guider_ui.mode_del, &guider_ui.home_del, setup_scr_mode, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, false, true);
            lv_obj_set_tile(guider_ui.mode_tileview, guider_ui.mode_tileview_mode_e, LV_ANIM_OFF);
            break;
        case 'c':
            ui_load_scr_animation(&guider_ui, &guider_ui.mode, guider_ui.mode_del, &guider_ui.home_del, setup_scr_mode, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, false, true);
            lv_obj_set_tile(guider_ui.mode_tileview, guider_ui.mode_tileview_mode_c, LV_ANIM_OFF);
            break;
        case 'h':
            ui_load_scr_animation(&guider_ui, &guider_ui.home, guider_ui.home_del, &guider_ui.mode_del, setup_scr_home, LV_SCR_LOAD_ANIM_NONE, 100, 0, false, true);
            break;
        default:
            printf("unknown data %c\n", data);
            break;
    }

}

void * proc_uart_data(void *arg)
{
    uint8_t data;
    //printf("proc_uart_data started\n");
    static pthread_mutex_t mutex;
     pthread_mutex_init(&mutex, NULL);

    while(flag_running){
        if(ring_buffer_read(&uart_ring_buffer, &data)){
            data_2_ui(data);
            // process data
            //printf("have data\n");
            //usleep(100);
            printf("%c", data);
        }else{
            // buffer empty, sleep a while
            //usleep(1000);
            //printf("buffer empty, wait...\n");
            pthread_cond_wait(&buffer_not_empty, &mutex);
        }
    }
    pthread_exit(NULL);
}
