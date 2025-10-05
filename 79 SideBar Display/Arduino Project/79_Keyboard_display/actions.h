#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_timer_change_mins(lv_event_t * e);
extern void action_timer_change_sec(lv_event_t * e);
extern void action_main_refresh(lv_event_t * e);
extern void action_timer_start(lv_event_t * e);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/