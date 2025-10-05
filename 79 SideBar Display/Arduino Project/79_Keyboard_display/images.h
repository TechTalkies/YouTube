#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_01d;
extern const lv_img_dsc_t img_02d;
extern const lv_img_dsc_t img_03d;
extern const lv_img_dsc_t img_09d;
extern const lv_img_dsc_t img_11d;
extern const lv_img_dsc_t img_13d;
extern const lv_img_dsc_t img_50d;
extern const lv_img_dsc_t img_timer;
extern const lv_img_dsc_t img_yt;
extern const lv_img_dsc_t img_thermometer;
extern const lv_img_dsc_t img_humidity;
extern const lv_img_dsc_t img_wind;
extern const lv_img_dsc_t img_refresh1;
extern const lv_img_dsc_t img_refresh2;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[14];


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/