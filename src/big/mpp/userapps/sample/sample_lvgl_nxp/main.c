
#include "k_connector_comm.h"
#include "k_vo_comm.h"
#include "lv_conf.h"
#include <signal.h>
#include <signal.h>
#include <src/core/lv_global.h>
#include <src/core/lv_obj.h>
#include <src/core/lv_obj_pos.h>
#include <src/display/lv_display.h>
#include <src/misc/lv_types.h>
#include <src/widgets/label/lv_label.h>
#include <stdio.h>
#include <stdlib.h>
//#include <vector_graphic/lv_demo_vector_graphic.h>
#include <vg_lite.h>
#include <vg_lite_util.h>
#include <unistd.h>
//#include "demos/lv_demos.h"
#include "mpi_connector_api.h"
#include <poll.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <mpi_vb_api.h>
#include <mpi_vo_api.h>
#include <mpi_sys_api.h>

#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"
#include <pthread.h>
#include "main.h"
#include "ring_buffer.h"

lv_ui guider_ui;
bool flag_running = true;
unsigned width = 0, height = 0;




static void sighandler(int signum)
{
    flag_running = false;
}
#if LV_USE_DRAW_VG_LITE  //not use;

// struct display_buffer* dbuf[BUFFER_COUNT];
char *error_type[] = {
    "VG_LITE_SUCCESS",
    "VG_LITE_INVALID_ARGUMENT",
    "VG_LITE_OUT_OF_MEMORY",
    "VG_LITE_NO_CONTEXT",
    "VG_LITE_TIMEOUT",
    "VG_LITE_OUT_OF_RESOURCES",
    "VG_LITE_GENERIC_IO",
    "VG_LITE_NOT_SUPPORT",
};
#define CHECK_ERROR(err) if ((err) != VG_LITE_SUCCESS) { printf("%s: %s\n", #err, error_type[err]); return -1; }
vg_lite_buffer_t* gbuf;


static const char *getenv_default(const char *name, const char *dflt) {
    return getenv(name) ? : dflt;
}

struct list_head {
    struct list_head * next, * prev;
    vg_lite_buffer_t buffer;
};

static struct list_head head = { NULL, NULL};

static void list_push(struct list_head * list, struct list_head * node) {
    node->next = list->next;
    node->prev = list;
    if (list->next) {
        list->next->prev = node;
    }
    list->next = node;
}

static void list_remove(struct list_head * node) {
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
}

vg_lite_buffer_t* vg_find_buffer(const void* p) {
    struct list_head* node = head.next;
    while (node) {
        if (node->buffer.memory == p) {
            return &node->buffer;
        }
        node = node->next;
    }
    return 0;
}

void * vg_allocate_buffer(size_t size) {
    struct list_head* node = calloc(sizeof(struct list_head), 1);
    node->buffer.width = size;
    node->buffer.height = 1;
    node->buffer.format = VG_LITE_A8;
    vg_lite_error_t err = vg_lite_allocate(&node->buffer);
    memset(node->buffer.memory, 0, size);
    if (err != VG_LITE_SUCCESS) {
        free(node);
        printf("vg_lite_allocate failed: %d\n", err);
        return NULL;
    }
    // printf("[GPU] allocate size %lu, err %d, ptr %p\n", size, err, node->buffer.memory);
    list_push(&head, node);
    return node->buffer.memory;
}

void vg_free_buffer(void * p) {
    struct list_head* node = head.next;
    while (node) {
        if (node->buffer.memory == p) {
            // printf("[GPU] free %p\n", node->buffer.memory);
            vg_lite_free(&node->buffer);
            list_remove(node);
            free(node);
            return;
        }
        node = node->next;
    }
    printf("lv_free_core: buffer not found\n");
}

uint32_t linux_get_idle(void)
{
    return 0;
    // struct rusage state;
    // static struct timeval last_time;
    // static uint64_t last_used_us;
    // struct timeval current_time;

    // if (getrusage(RUSAGE_SELF, &state)) {
    //     perror("getrusage");
    //     return 100;
    // }
    // gettimeofday(&current_time, NULL);
    // uint64_t duration_us = (current_time.tv_sec - last_time.tv_sec) * 1000000 + (current_time.tv_usec - last_time.tv_usec);
    // last_time = current_time;
    // uint64_t used_us = state.ru_utime.tv_sec * 1000000 + state.ru_utime.tv_usec + state.ru_stime.tv_sec * 1000000 + state.ru_stime.tv_usec;
    // uint64_t current_used_us = used_us - last_used_us;
    // last_used_us = used_us;
    // return (duration_us - current_used_us) * 100 / duration_us;
}
#endif

int lvgl_demo_test(void)
{
    // lv_demo_widgets();
    // lv_demo_widgets_start_slideshow();
    // lv_demo_music();
    // lv_demo_vector_graphic_buffered();
    // lv_demo_benchmark();

    // void load_scene(uint32_t scene);
    // load_scene(6);

    // void moving_wallpaper_cb(void);
    // moving_wallpaper_cb();

    // lv_obj_t* label = lv_label_create(lv_screen_active());
    // lv_label_set_text(label, "Hello, world!");

    // lv_obj_t* label;
    // lv_obj_t * btn1 = lv_button_create(lv_screen_active());
    // // lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
    // lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -300);
    // lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

    // label = lv_label_create(btn1);
    // lv_label_set_text(label, "Button");
    // lv_obj_center(label);

    // lv_obj_t * btn2 = lv_button_create(lv_screen_active());
    // // lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
    // lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
    // lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    // lv_obj_set_height(btn2, LV_SIZE_CONTENT);

    // label = lv_label_create(btn2);
    // lv_label_set_text(label, "Toggle");
    // lv_obj_center(label);
    return 0;
}

int main(int argc, char *argv[]) {
    int c, ret;
    k_connector_type connector_type = ST7701_V1_MIPI_2LAN_480X800_30FPS;

    while ((c = getopt(argc, argv, "d:W:H:h")) != -1) {
        switch (c) {
            case 'd':
                connector_type = atoi(optarg);
                break;
            case 'W':
                width = atoi(optarg);
                break;
            case 'H':
                height = atoi(optarg);
                break;
            case 'h':
                printf("Usage: %s [-d <display>] [-W <width>] [-H <height>]\n", argv[0]);
                return 0;
            default:
                printf("Usage: %s [-h]\n", argv[0]);
                return 1;
        }
    }
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);
    lv_init();
    lv_port_init_k230(connector_type);

    //printf("f=%s l=%d\n", __FUNCTION__, __LINE__);
    setup_ui(&guider_ui);
    events_init(&guider_ui);
    custom_init(&guider_ui);

    // struct timeval start, current;
    // gettimeofday(&start, NULL);

    uart_init();

    while(flag_running) {
        uint32_t idle_time = lv_timer_handler(); /*Returns the time to the next timer execution*/
        if (idle_time > 65536) {
            // error
            idle_time = 0;
        }
        // printf("idle_time: %u\n", idle_time);
        usleep(idle_time * 1000);
        //proc_uart_data(NULL);
        // gettimeofday(&current, NULL);
        // uint32_t elapsed_us = (current.tv_sec - start.tv_sec) * 1000000 + (current.tv_usec - start.tv_usec);
        // lv_obj_set_pos(btn1, 20, (elapsed_us / 30000) % 480);
        // lv_obj_align(btn1, LV_ALIGN_CENTER, 0, (elapsed_us / 30000) % 100);
    }
    lv_port_uninit_k230();
    return 0;
}
