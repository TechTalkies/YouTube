#pragma once
#include "driver/ledc.h"

// GPIO pins — adjust to your wiring
#define SERVO_FL_GPIO  3
#define SERVO_FR_GPIO  4
#define SERVO_BL_GPIO  5
#define SERVO_BR_GPIO  6

// LEDC channels, one per servo
#define SERVO_FL_CH    LEDC_CHANNEL_0
#define SERVO_FR_CH    LEDC_CHANNEL_1
#define SERVO_BL_CH    LEDC_CHANNEL_2
#define SERVO_BR_CH    LEDC_CHANNEL_3

// MG90S pulse range (us) — standard
#define SERVO_MIN_US   500
#define SERVO_MAX_US   2500
#define SERVO_FREQ_HZ  50

// Neutral and movement positions (degrees)
#define SERVO_NEUTRAL  90
#define SERVO_FORWARD  60   // FL, BR
#define SERVO_BACK     120  // FL, BR  (FR, BL are inverted — see servo.c)

void servo_init(void);
void servo_set_angle(ledc_channel_t channel, int degrees);
void servo_all_neutral(void);

// Animations
void anim_good_boy(void);
void anim_sit_down(void);
void anim_lie_down(void);
void anim_stretch(void);
void anim_walk(void);
void anim_dance(void);