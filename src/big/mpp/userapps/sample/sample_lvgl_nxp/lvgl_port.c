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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include "ring_buffer.h"


#if LV_COLOR_DEPTH == 16
    #define BUFFER_SIZE (width * height * 2)
#elif LV_COLOR_DEPTH == 32
    #define BUFFER_SIZE (width * height * 4)
#else
#error "Unsupported color depth"
#endif


#ifndef DIV_ROUND_UP
    #define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif


#define RT_TOUCH_EVENT_NONE              (0)   /* Touch none */
#define RT_TOUCH_EVENT_UP                (1)   /* Touch up event */
#define RT_TOUCH_EVENT_DOWN              (2)   /* Touch down event */
#define RT_TOUCH_EVENT_MOVE              (3)   /* Touch move event */
typedef unsigned int tick_t;
struct rt_touch_data
{
    uint8_t  event;                 /* 触摸事件类型 */
    uint8_t  track_id;              /* 触摸点 ID */
    uint8_t  width;                 /* 触摸宽度 */
    uint16_t x_coordinate;          /* X 坐标 */
    uint16_t y_coordinate;          /* Y 坐标 */
    tick_t   timestamp;             /* 时间戳 */
};


struct display_data {
    unsigned width;
    unsigned height;
    unsigned size;
};



k_vb_blk_handle vb_blk_handle[2];
void* vb_blk_biff_virt_addr[2];
k_video_frame_info vf_info[2];


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
static void flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map) {
    if(!lv_display_flush_is_last(disp)) return;

    static unsigned frame_count = 0;
    int buff_idx = frame_count % 2;

    lv_display_rotation_t rotation = lv_display_get_rotation(disp);
    if((rotation == LV_DISPLAY_ROTATION_270) || (rotation == LV_DISPLAY_ROTATION_90)){
        int32_t w = 800;
        int32_t h = 480;
        lv_color_format_t cf = lv_display_get_color_format(disp);
        uint32_t w_stride = lv_draw_buf_width_to_stride(w, cf);
        uint32_t h_stride = lv_draw_buf_width_to_stride(h, cf);
        lv_draw_sw_rotate(px_map, vb_blk_biff_virt_addr[buff_idx], w, h, w_stride, h_stride, rotation, cf);
    }else {
        memcpy(vb_blk_biff_virt_addr[buff_idx], px_map, BUFFER_SIZE);
    }
    kd_mpi_vo_chn_insert_frame(K_VO_OSD1 + 3, &vf_info[buff_idx]);

    // thead_csi_dcache_clean_invalid_range(px_map, dbuf[0]->size);
    // vg_lite_finish();
    // printf("flush %d\n", idx);
    //usleep(100000);
    // static struct timeval last_time;
    // struct timeval  current;
    // gettimeofday(&current, NULL);
    // uint32_t elapsed_us = (current.tv_sec - last_time.tv_sec) * 1000000 + (current.tv_usec - last_time.tv_usec);
    // last_time = current;
    // printf("flush time: %u us\n", elapsed_us);
    frame_count++;

}

static void flush_wait(lv_display_t * disp) {
    // TODO
    // usleep(100000);
}

static uint32_t tick_get_cb(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    uint64_t time_ms = t.tv_sec * 1000 + (t.tv_nsec / 1000000);
    return time_ms;
}



static int lv_port_init_k230_vb()
{
    int ret;
        // vb pool
    kd_mpi_vb_exit();
    k_vb_config config;
    k_vb_pool_config pool_config;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 1;
    config.comm_pool[0].blk_cnt = 2;
    config.comm_pool[0].blk_size = BUFFER_SIZE + 0x1000;
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;

    ret = kd_mpi_vb_set_config(&config);
    if (ret) {
        printf("kd_mpi_vb_set_config failed: %d\n", ret);
        return ret;
    }
    ret = kd_mpi_vb_init();
    if (ret) {
        printf("kd_mpi_vb_init failed: %d\n", ret);
        return ret;
    }

    for(int i = 0; i < 2; i++) {
        vb_blk_handle[i] = kd_mpi_vb_get_block(VB_INVALID_POOLID, 0, NULL);
        if (vb_blk_handle[i] == VB_INVALID_HANDLE) {
            printf("kd_mpi_vb_get_block failed\n");
            return -1;
        }
        k_u64 phys_addr = kd_mpi_vb_handle_to_phyaddr(vb_blk_handle[i]);
        // printf("phys_addr %08lx, size: %u\n", phys_addr, BUFFER_SIZE);
        vb_blk_biff_virt_addr[i] = kd_mpi_sys_mmap(phys_addr, BUFFER_SIZE);
        vf_info[i] = (k_video_frame_info) {
        .pool_id = kd_mpi_vb_handle_to_pool_id(vb_blk_handle[i]),
        .mod_id = K_ID_VO,
            .v_frame = {
            .width = width,
            .height = height,
#if LV_COLOR_DEPTH == 16
            .pixel_format = PIXEL_FORMAT_RGB_565,
            .stride = width * 2,
#elif LV_COLOR_DEPTH == 32
            .pixel_format = PIXEL_FORMAT_BGRA_8888,
            .stride = width * 4,
#endif
            .phys_addr = {phys_addr, phys_addr, phys_addr},
            }
        };

    }

    return 0;
}


static int lv_port_init_k230_vo()
{
        // vo init
    k_vo_video_osd_attr attr = {
        .display_rect = {0, 0},
        .img_size = {width, height},
#if LV_COLOR_DEPTH == 16
        .pixel_format = PIXEL_FORMAT_BGR_565,
        .stride = width * 2 / 8,
#else
        .pixel_format = PIXEL_FORMAT_BGRA_8888,
        .stride = width * 4 / 8,
#endif
        .global_alptha = 0xff,
    };
    kd_mpi_vo_set_video_osd_attr(K_VO_OSD1, &attr);
    kd_mpi_vo_osd_enable(K_VO_OSD1);
    kd_mpi_vo_enable();
    return 0;
}
static int lv_port_init_k230_connect(k_connector_type connector_type)
{
    int ret;
    k_connector_info connector_info;
    int connector_fd;
    unsigned chip_id = 0;
    memset(&connector_info, 0, sizeof(k_connector_info));
    //connect,
    //vb
     ret = kd_mpi_get_connector_info(connector_type, &connector_info);
    if (ret) {
        printf("connector type %d not found\n", connector_type);
        return ret;
    }
    if (width == 0) {
        width = connector_info.resolution.hdisplay;
    }
    if (height == 0) {
        height = connector_info.resolution.vdisplay;
    }
    printf("connector_type: %d, chip_id: %d, width: %d, height: %d\n", connector_type, chip_id, width, height);
    connector_fd = kd_mpi_connector_open(connector_info.connector_name);
    if (connector_fd < 0) {
        printf("%s, connector open failed.\n", __func__);
        return K_ERR_VO_NOTREADY;
    }
    // set connect power
    kd_mpi_connector_power_set(connector_fd, 1);
    // set connect get id
    kd_mpi_connector_id_get(connector_fd, &chip_id);
    // connector init
    kd_mpi_connector_init(connector_fd, connector_info);

    return 0;
}

static int lv_port_init_k230_disp(void)
{
    lv_display_t * disp = lv_display_create(width, height);
    struct display_data d = {
        .width = width,
        .height = height,
        .size = BUFFER_SIZE,
    };
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
    lv_display_set_driver_data(disp, &d);
    lv_display_set_flush_wait_cb(disp, flush_wait);
    lv_display_set_flush_cb(disp, flush);
    lv_display_set_resolution(disp, width, height);
    lv_display_set_dpi(disp, DIV_ROUND_UP(width * 25400, 300 * 1000));
#if LV_USE_DRAW_VG_LITE
        CHECK_ERROR(vg_lite_init(width, height));

    struct list_head* node = calloc(sizeof(struct list_head), 1);
    gbuf = &node->buffer;
    gbuf->width = width;
    gbuf->height = height;
    #if LV_COLOR_DEPTH == 16
    gbuf->format = VG_LITE_BGR565;
    gbuf->stride = gbuf->width * 2;
    #elif LV_COLOR_DEPTH == 32
    gbuf->format = VG_LITE_RGBA8888;
    gbuf->stride = gbuf->width * 4;
    #endif

    CHECK_ERROR(vg_lite_allocate(gbuf));
    list_push(&head, node);
    lv_display_set_buffers(disp, gbuf->memory, NULL, BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_DIRECT);
#else
    void* draw_buffer = malloc(BUFFER_SIZE);
    lv_display_set_buffers(disp, draw_buffer, NULL, BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_DIRECT);
#endif
    // display_commit_buffer(dbuf[0], 1920 - dbuf[0]->width, 1080 - dbuf[0]->height);

}
//lv_indev_t * indev_touchpad;
// lv_indev_t * indev_mouse;
// lv_indev_t * indev_keypad;
// lv_indev_t * indev_encoder;
//lv_indev_t * indev_button;

static int touchpad_init_k230(lv_indev_t * indev)
{
    int fd = 0;
    fd = open("/dev/touch0", O_RDONLY);
    if (fd < 0) {
        printf("open /dev/touch0 failed\n");
        return -1;
    }
    lv_indev_set_driver_data(indev, (void*)(unsigned long)fd);
    // TODO
}
static inline void touchpad_get_xy(int32_t * x, int32_t * y)
{
    //不需要旋转，lvgl内部会自己旋转；
    return ;
}

static void touchpad_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    struct rt_touch_data ev;
    int fd = (int)(uintptr_t)lv_indev_get_driver_data(indev);
    ssize_t n = read(fd, &ev, sizeof(ev));

    if(n == sizeof(ev)) {
        //printf("touch event: %d, x: %d, y: %d fd=%x n=%x %x\n", ev.event, ev.x_coordinate, ev.y_coordinate,fd,n, sizeof(ev));
        if(ev.event == RT_TOUCH_EVENT_DOWN || ev.event == RT_TOUCH_EVENT_MOVE) {
            data->state = LV_INDEV_STATE_PRESSED;
            data->point.x = ev.x_coordinate;
            data->point.y = ev.y_coordinate;
        } else {
            data->state = LV_INDEV_STATE_RELEASED;
            data->point.x = ev.x_coordinate;
            data->point.y = ev.y_coordinate;
        }
        touchpad_get_xy(&data->point.x, &data->point.y);
        //printf("touch after rotate: %d, x: %d, y: %d\n", data->state, data->point.x, data->point.y);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

}

static int touchpad_uninit_k230(lv_indev_t * indev)
{
    int fd = (int)(uintptr_t)lv_indev_get_driver_data(indev);
    close(fd);
    return 0;
    // TODO
}


// static void button_init(void);
// static void button_read(lv_indev_t * indev, lv_indev_data_t * data);
// static int8_t button_get_pressed_id(void);
// static bool button_is_pressed(uint8_t id);
static bool button_is_pressed(char c, uint8_t id)
{

    /*Your code comes here*/
    //   {260, 421},   /*Button 0 -> x:10; y:10*/
    //     {331, 421},  /*Button 1 -> x:40; y:100*/
    //     {403, 421},  /*Button 1 -> x:40; y:100*/
    //     {475, 421},  /*Button 1 -> x:40; y:100*/
    //     {21, 5},  /*Button 1 -> x:40; y:100*/


    if(c == 0 )
        return false;
    //printf("get uart data: %c for button %d\n", c, id);

    if((c == 'a' ) && (id == 0))
        return true;
    if((c == 'f' ) && (id == 1))
        return true;
    if((c == 'c' ) && (id == 2))
        return true;
    if((c == 'e' ) && (id == 3))
        return true;
    if((c == 'h' ) && (id == 4))
        return true;

    return false;
}

/*Get ID  (0, 1, 2 ..) of the pressed button*/
static int8_t button_get_pressed_id(char c)
{
    uint8_t i;


    /*Check to buttons see which is being pressed (assume there are 2 buttons)*/
    for(i = 0; i < 5; i++) {
        /*Return the pressed button's ID*/
        if(button_is_pressed(c,i)) {
            printf("button %d pressed\n", i);
            return i;
        }
    }

    /*No button pressed*/
    return -1;
}

/*Will be called by the library to read the button*/
static void button_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{

    static char last_btn = 0;
    char c = get_uart_data();
    if(c == 0 )
        return ;

    /*Get the pressed button's ID*/
    int8_t btn_act = button_get_pressed_id(c);

    if(btn_act >= 0) {
        data->state = LV_INDEV_STATE_PRESSED;
        last_btn = btn_act;
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    /*Save the last pressed button's ID*/
    data->btn_id = last_btn;
}

int lv_port_indev_init_k230()
{

    /* Register a touchpad input device */

    lv_indev_t * indev = lv_indev_create();
    touchpad_init_k230(indev);
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchpad_read);


    /*------------------
     * Button
     * -----------------*/

    // /*Initialize your button if you have*/
    // button_init();

    // /*Register a button input device*/
    lv_indev_t *indev_button = lv_indev_create();
    lv_indev_set_type(indev_button, LV_INDEV_TYPE_BUTTON);
    lv_indev_set_read_cb(indev_button, button_read);

    /*Assign buttons to points on the screen*/
    static const lv_point_t btn_points[5] = {
        {260, 421},   /*Button 0 -> x:10; y:10*/
        {331, 421},  /*Button 1 -> x:40; y:100*/
        {403, 421},  /*Button 1 -> x:40; y:100*/
        {475, 421},  /*Button 1 -> x:40; y:100*/
        {21, 5},  /*Button 1 -> x:40; y:100*/
    };
    lv_indev_set_button_points(indev_button, btn_points);


    return 0;
}


int lv_port_init_k230(k_connector_type connector_type)
{
    lv_port_init_k230_connect(connector_type);
    lv_port_init_k230_vb();
    lv_port_init_k230_vo();
    lv_port_init_k230_disp();
    lv_tick_set_cb(tick_get_cb);
    lv_port_indev_init_k230();
    return 0;
}

int lv_port_uninit_k230(void)
{
    vg_lite_close();
    kd_mpi_vo_osd_disable(K_VO_OSD1);
    kd_mpi_vo_disable();
    for(int i = 0; i < 2; i++) {
        kd_mpi_sys_munmap(vb_blk_biff_virt_addr[i], BUFFER_SIZE);
        kd_mpi_vb_release_block(vb_blk_handle[i]);
    }
    usleep(50000);
    kd_mpi_vb_exit();
    return 0;
}
