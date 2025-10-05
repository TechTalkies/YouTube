#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_NONE
};

// Native global variables

extern int32_t get_var_timer_mins();
extern void set_var_timer_mins(int32_t value);
extern int32_t get_var_timer_sec();
extern void set_var_timer_sec(int32_t value);
extern bool get_var_timer_countdown();
extern void set_var_timer_countdown(bool value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/