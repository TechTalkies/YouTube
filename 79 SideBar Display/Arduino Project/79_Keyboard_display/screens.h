#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *you_tube_page;
    lv_obj_t *weather_page;
    lv_obj_t *timer_page;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *timer_roller_mins;
    lv_obj_t *timer_roller_sec;
    lv_obj_t *timer_switch_reverse;
    lv_obj_t *timer_button_start;
    lv_obj_t *timer_button_close;
    lv_obj_t *label_main_yt;
    lv_obj_t *label_main_yt_1;
    lv_obj_t *img_main_weather;
    lv_obj_t *label_main_weather;
    lv_obj_t *main_label_wifi;
    lv_obj_t *main_label_refreshed;
    lv_obj_t *label_yt_title;
    lv_obj_t *label_yt_subs;
    lv_obj_t *label_yt_views;
    lv_obj_t *label_yt_videos;
    lv_obj_t *img_weather_icon;
    lv_obj_t *label_weather_title;
    lv_obj_t *label_weather_desc;
    lv_obj_t *label_weather_temp;
    lv_obj_t *label_weather_hum;
    lv_obj_t *label_weather_wind;
    lv_obj_t *timer_label_min;
    lv_obj_t *timer_label_sec;
    lv_obj_t *timer_label_start;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_YOU_TUBE_PAGE = 2,
    SCREEN_ID_WEATHER_PAGE = 3,
    SCREEN_ID_TIMER_PAGE = 4,
};

void create_screen_main();
void tick_screen_main();

void create_screen_you_tube_page();
void tick_screen_you_tube_page();

void create_screen_weather_page();
void tick_screen_weather_page();

void create_screen_timer_page();
void tick_screen_timer_page();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/