/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"


typedef struct
{
  
	lv_obj_t *home;
	bool home_del;
	lv_obj_t *home_arc_needle;
	lv_obj_t *home_label_digit;
	lv_obj_t *home_label_trip_num;
	lv_obj_t *home_label_power_num;
	lv_obj_t *home_digital_clock_time;
	lv_obj_t *home_bar_battery;
	lv_obj_t *home_label_ODO_num;
	lv_obj_t *home_img_right;
	lv_obj_t *home_img_left;
	lv_obj_t *home_label_WIFI;
	lv_obj_t *home_label_BT;
	lv_obj_t *home_img_moto;
	lv_obj_t *home_img_light;
	lv_obj_t *home_btn_mode_a;
	lv_obj_t *home_btn_mode_a_label;
	lv_obj_t *home_img_red;
	lv_obj_t *home_img_green;
	lv_obj_t *home_btn_mode_f;
	lv_obj_t *home_btn_mode_f_label;
	lv_obj_t *home_btn_mode_e;
	lv_obj_t *home_btn_mode_e_label;
	lv_obj_t *home_img_high_light;
	lv_obj_t *home_btn_mode_c;
	lv_obj_t *home_btn_mode_c_label;
	lv_obj_t *mode;
	bool mode_del;
	lv_obj_t *mode_tileview;
	lv_obj_t *mode_tileview_mode_a;
	lv_obj_t *mode_tileview_mode_f;
	lv_obj_t *mode_tileview_mode_c;
	lv_obj_t *mode_tileview_mode_e;
	lv_obj_t *mode_img_abg;
	lv_obj_t *mode_cont;
	lv_obj_t *mode_label_mode;
	lv_obj_t *mode_label_A;
	lv_obj_t *mode_img_nav;
	lv_obj_t *mode_spangroup_1;
	lv_span_t *mode_spangroup_1_span;
	lv_obj_t *mode_animimg_map;
	lv_obj_t *mode_label_dot;
	lv_obj_t *mode_label_fkm_num;
	lv_obj_t *mode_label_fkm;
	lv_obj_t *mode_contf;
	lv_obj_t *mode_label_fmode;
	lv_obj_t *mode_label_F;
	lv_obj_t *mode_label_speed;
	lv_obj_t *mode_label_temp_num;
	lv_obj_t *mode_img_temper;
	lv_obj_t *mode_img_fbg;
	lv_obj_t *mode_label_tempter;
	lv_obj_t *mode_cont_4;
	lv_obj_t *mode_label_cmode;
	lv_obj_t *mode_label_C;
	lv_obj_t *mode_label_ctemp_num;
	lv_obj_t *mode_label_temperature;
	lv_obj_t *mode_label_day;
	lv_obj_t *mode_img_weather;
	lv_obj_t *mode_img_cbg;
	lv_obj_t *mode_label_name;
	lv_obj_t *mode_contc;
	lv_obj_t *mode_label_prev;
	lv_obj_t *mode_label_play;
	lv_obj_t *mode_label_next;
	lv_obj_t *mode_img_music;
	lv_obj_t *mode_img_1;
	lv_obj_t *mode_img_2;
	lv_obj_t *mode_img_ebg;
	lv_obj_t *mode_label_ekm_num;
	lv_obj_t *mode_conte;
	lv_obj_t *mode_label_emode;
	lv_obj_t *mode_label_E;
	lv_obj_t *mode_label_ekm;
	lv_obj_t *mode_chart_speed;
	lv_chart_series_t *mode_chart_speed_0;
	lv_obj_t *mode_img_logo;
	lv_obj_t *mode_label_WIFI;
	lv_obj_t *mode_bar_battery;
	lv_obj_t *mode_digital_clock;
	lv_obj_t *mode_img_home;
	lv_obj_t *mode_label_BT;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, uint32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                  uint32_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                  lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_completed_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_bottom_layer(void);

void setup_ui(lv_ui *ui);

void video_play(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_home(lv_ui *ui);
void setup_scr_mode(lv_ui *ui);

LV_IMAGE_DECLARE(_moto_RGB565A8_800x480);

LV_IMAGE_DECLARE(_kmbg_RGB565A8_166x166);

LV_IMAGE_DECLARE(_battery_bak_RGB565A8_43x24);

LV_IMAGE_DECLARE(_battery_ind_RGB565A8_43x24);
LV_IMAGE_DECLARE(_direction_RGB565A8_50x42);
LV_IMAGE_DECLARE(_monitor_RGB565A8_41x37);
LV_IMAGE_DECLARE(_light_RGB565A8_46x46);
LV_IMAGE_DECLARE(_image_red_RGB565A8_350x350);
LV_IMAGE_DECLARE(_image_green_RGB565A8_350x350);
LV_IMAGE_DECLARE(_high_beam_RGB565A8_46x46);
LV_IMAGE_DECLARE(_det_bak_RGB565A8_800x480);
LV_IMAGE_DECLARE(_img_nav_1_RGB565A8_116x116);
LV_IMAGE_DECLARE(mode_animimg_mapmap_0);
LV_IMAGE_DECLARE(mode_animimg_mapmap_01);
LV_IMAGE_DECLARE(mode_animimg_mapmap_02);
LV_IMAGE_DECLARE(mode_animimg_mapmap_03);
LV_IMAGE_DECLARE(mode_animimg_mapmap_04);
LV_IMAGE_DECLARE(mode_animimg_mapmap_05);
LV_IMAGE_DECLARE(mode_animimg_mapmap_06);
LV_IMAGE_DECLARE(_tem_RGB565A8_91x123);
LV_IMAGE_DECLARE(_sunny_RGB565A8_66x66);
LV_IMAGE_DECLARE(_music_cover_1_RGB565A8_183x183);
LV_IMAGE_DECLARE(_music_cover_2_RGB565A8_183x183);
LV_IMAGE_DECLARE(_music_cover_3_RGB565A8_183x183);
LV_IMAGE_DECLARE(_logo_RGB565A8_100x35);
LV_IMAGE_DECLARE(_home_RGB565A8_43x43);

LV_FONT_DECLARE(lv_font_arial_40)
LV_FONT_DECLARE(lv_font_arial_53)
LV_FONT_DECLARE(lv_font_arial_26)
LV_FONT_DECLARE(lv_font_montserratMedium_26)
LV_FONT_DECLARE(lv_font_arial_20)
LV_FONT_DECLARE(lv_font_arial_83)
LV_FONT_DECLARE(lv_font_arial_15)
LV_FONT_DECLARE(lv_font_arial_100)
LV_FONT_DECLARE(lv_font_arial_66)
LV_FONT_DECLARE(lv_font_arial_50)
LV_FONT_DECLARE(lv_font_arial_21)


#ifdef __cplusplus
}
#endif
#endif
