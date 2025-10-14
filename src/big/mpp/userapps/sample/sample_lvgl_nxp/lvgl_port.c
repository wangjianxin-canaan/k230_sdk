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

extern unsigned width , height , buffer_size;
struct display_data {
    unsigned width;
    unsigned height;
    unsigned size;
};
#ifndef DIV_ROUND_UP
    #define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif
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
    frame_count++;

    // unsigned idx = frame_count % BUFFER_COUNT;

    k_vb_blk_handle handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, 0, NULL);
    if (handle == VB_INVALID_HANDLE) {
        printf("kd_mpi_vb_get_block failed\n");
        return;
    }
    k_u64 phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    // printf("phys_addr %08lx, size: %u\n", phys_addr, buffer_size);
    // FIXME: use gpu
    void* virt_addr = kd_mpi_sys_mmap(phys_addr, buffer_size);
    memcpy(virt_addr, px_map, buffer_size);
    kd_mpi_sys_munmap(virt_addr, buffer_size);
    k_video_frame_info vf_info = {
        .pool_id = kd_mpi_vb_handle_to_pool_id(handle),
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
    kd_mpi_vo_chn_insert_frame(K_VO_OSD1 + 3, &vf_info);
    kd_mpi_vb_release_block(handle);

    // thead_csi_dcache_clean_invalid_range(px_map, dbuf[0]->size);
    // vg_lite_finish();
    // printf("flush %d\n", idx);
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
int lvgl_hal_init(void)
{

    lv_display_t * disp = lv_display_create(width, height);
    struct display_data d = {
        .width = width,
        .height = height,
        .size = buffer_size,
    };
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
    lv_display_set_buffers(disp, gbuf->memory, NULL, buffer_size, LV_DISPLAY_RENDER_MODE_DIRECT);
#else
    void* draw_buffer = malloc(buffer_size);
    lv_display_set_buffers(disp, draw_buffer, NULL, buffer_size, LV_DISPLAY_RENDER_MODE_DIRECT);
#endif
    // display_commit_buffer(dbuf[0], 1920 - dbuf[0]->width, 1080 - dbuf[0]->height);
    lv_tick_set_cb(tick_get_cb);
}

int k230_gui_driver_init_vb()
{
    int ret;
        // vb pool
    kd_mpi_vb_exit();
    k_vb_config config;
    k_vb_pool_config pool_config;
    buffer_size = width * height * 4;
    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 1;
    config.comm_pool[0].blk_cnt = 5;
#if LV_COLOR_DEPTH == 16
    buffer_size = width * height * 2;
#elif LV_COLOR_DEPTH == 32
    buffer_size = width * height * 4;
#else
#error "Unsupported color depth"
#endif
    config.comm_pool[0].blk_size = buffer_size + 0x1000;
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
    return 0;
}


int k230_gui_driver_init_vo()
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
int k230_gui_driver_init_connect(k_connector_type connector_type)
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
int k230_gui_driver_init(k_connector_type connector_type)
{
    k230_gui_driver_init_connect(connector_type);
    k230_gui_driver_init_vb();
    k230_gui_driver_init_vo();
    return 0;
}

int k230_gui_driver_uninit(void)
{
    vg_lite_close();
    kd_mpi_vo_osd_disable(K_VO_OSD1);
    kd_mpi_vo_disable();
    usleep(50000);
    kd_mpi_vb_exit();
    return 0;
}
