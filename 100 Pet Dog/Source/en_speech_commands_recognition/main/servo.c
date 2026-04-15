#include "servo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct
{
    int fl, fr, bl, br;
} legs_t;

static legs_t cur = {90, 90, 90, 90};

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static uint32_t angle_to_duty(int degrees)
{
    int pulse_us = SERVO_MIN_US + (degrees * (SERVO_MAX_US - SERVO_MIN_US)) / 180;
    return (uint32_t)((pulse_us * 16383) / 20000);
}

static void set_fl(int deg)
{
    cur.fl = deg;
    servo_set_angle(SERVO_FL_CH, deg);
}
static void set_fr(int deg)
{
    cur.fr = deg;
    servo_set_angle(SERVO_FR_CH, 180 - deg);
}
static void set_bl(int deg)
{
    cur.bl = deg;
    servo_set_angle(SERVO_BL_CH, 180 - deg);
}
static void set_br(int deg)
{
    cur.br = deg;
    servo_set_angle(SERVO_BR_CH, deg);
}

static void set_legs(legs_t p)
{
    set_fl(p.fl);
    set_fr(p.fr);
    set_bl(p.bl);
    set_br(p.br);
}

static void delay_ms(int ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }

static void pwm_off(ledc_channel_t ch)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ch, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, ch);
}

static void pwm_off_all(void)
{
    pwm_off(SERVO_FL_CH);
    pwm_off(SERVO_FR_CH);
    pwm_off(SERVO_BL_CH);
    pwm_off(SERVO_BR_CH);
}

// ---------------------------------------------------------------------------
// ease_to — reads cur automatically as starting point
// ---------------------------------------------------------------------------
static void ease_to(legs_t to, int duration_ms)
{
    legs_t from = cur;
    int steps = 20;
    for (int i = 0; i <= steps; i++)
    {
        float t = (float)i / steps;
        float e = t * t * (3.0f - 2.0f * t);
        set_fl(from.fl + (int)((to.fl - from.fl) * e));
        set_fr(from.fr + (int)((to.fr - from.fr) * e));
        set_bl(from.bl + (int)((to.bl - from.bl) * e));
        set_br(from.br + (int)((to.br - from.br) * e));
        delay_ms(duration_ms / steps);
    }
}

// Safe reset — always called before every animation
// Brings all legs to 90 slowly so the bot doesn't flip regardless of prior state
static void reset_to_neutral(void)
{
    legs_t neutral = {90, 90, 90, 90};
    ease_to(neutral, 500);
    delay_ms(150);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void servo_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_14_BIT,
        .freq_hz = SERVO_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channels[4] = {
        {.gpio_num = SERVO_FL_GPIO, .channel = SERVO_FL_CH},
        {.gpio_num = SERVO_FR_GPIO, .channel = SERVO_FR_CH},
        {.gpio_num = SERVO_BL_GPIO, .channel = SERVO_BL_CH},
        {.gpio_num = SERVO_BR_GPIO, .channel = SERVO_BR_CH},
    };
    for (int i = 0; i < 4; i++)
    {
        channels[i].speed_mode = LEDC_LOW_SPEED_MODE;
        channels[i].timer_sel = LEDC_TIMER_0;
        channels[i].intr_type = LEDC_INTR_DISABLE;
        channels[i].duty = angle_to_duty(SERVO_NEUTRAL);
        channels[i].hpoint = 0;
        ledc_channel_config(&channels[i]);
    }
}

void servo_set_angle(ledc_channel_t channel, int degrees)
{
    if (degrees < 0)
        degrees = 0;
    if (degrees > 180)
        degrees = 180;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, angle_to_duty(degrees));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}

void servo_all_neutral(void)
{
    reset_to_neutral();
    delay_ms(200);
    pwm_off_all();
}

// ---------------------------------------------------------------------------
// Animations — each starts with reset_to_neutral() for safe transitions
// Edit legs_t structs to tune angles: {FL, FR, BL, BR}, 90 = straight down
// lower = forward sweep, higher = backward sweep
// ---------------------------------------------------------------------------

void anim_good_boy(void)
{
    reset_to_neutral();

    // Wag: BL+BR alternate phase, amplitude builds then decays
    int wag_angles[] = {75, 105, 70, 110, 75, 105, 82, 98, 90};
    int wag_times[] = {120, 120, 110, 110, 100, 100, 90, 90, 80};
    int n = sizeof(wag_angles) / sizeof(wag_angles[0]);

    for (int i = 0; i < n; i++)
    {
        set_bl(wag_angles[i]);
        set_br(180 - wag_angles[i]);
        delay_ms(wag_times[i]);
    }

    ease_to((legs_t){90, 90, 90, 90}, 300);
    delay_ms(100);
    pwm_off_all();
}

void anim_sit_down(void)
{
    reset_to_neutral();

    legs_t lean = {75, 75, 90, 90};     // front dips, weight shifts forward
    legs_t sit = {90, 90, 110, 110};    // back legs fold — lower = deeper sit
    legs_t bounce = {75, 75, 120, 120}; // overshoot
    legs_t settle = {90, 90, 135, 135}; // final sit — edit 0 if too deep

    ease_to(lean, 600);
    delay_ms(200);
    ease_to(sit, 700);
    delay_ms(200);
    ease_to(bounce, 150);
    delay_ms(80);
    ease_to(settle, 150);
    delay_ms(400);
    pwm_off_all();
}

void anim_lie_down(void)
{
    reset_to_neutral();

    legs_t front = {60, 60, 90, 90};       // front sweeps forward first
    legs_t all_down = {80, 80, 60, 60};    // rear follows
    legs_t overshoot = {120, 120, 50, 50}; // brief overshoot
    legs_t settled = {180, 180, 0, 0};     // flat — edit if too extreme

    ease_to(front, 500);
    delay_ms(150);
    ease_to(all_down, 500);
    delay_ms(100);
    ease_to(overshoot, 200);
    delay_ms(80);
    ease_to(settled, 600);
    delay_ms(300);
    pwm_off_all();
}

void anim_stretch(void)
{
    reset_to_neutral();

    // Phase 1: front legs slowly reach forward (low angle = forward)
    legs_t front_stretch = {160, 160, 90, 90};
    ease_to(front_stretch, 1000); // very slow, deliberate reach
    delay_ms(600);                // hold and feel the stretch

    // Phase 2: slowly return front to neutral
    ease_to((legs_t){90, 90, 90, 90}, 800);
    delay_ms(300);

    // Phase 3: transition — rear legs slowly push back (high angle = backward)
    legs_t rear_stretch = {90, 90, 30, 30};
    ease_to(rear_stretch, 1000); // slow rear push
    delay_ms(600);               // hold

    // Phase 4: slowly return to neutral
    ease_to((legs_t){90, 90, 90, 90}, 800);
    delay_ms(200);

    pwm_off_all();
}

void anim_walk(void)
{
    reset_to_neutral();

    // Stance: slight forward lean ready to walk
    legs_t stance = {100, 100, 80, 80};
    ease_to(stance, 600);
    delay_ms(300);

    int cycles = 8;

    for (int i = 0; i < cycles; i++)
    {
        // Phase A: front pair sweeps forward, rear pair pushes back
        legs_t phase_a_front = {130, 130, 80, 80}; // front moves first
        legs_t phase_a_full = {130, 130, 60, 60};  // rear follows
        ease_to(phase_a_front, 250);
        ease_to(phase_a_full, 250);
        delay_ms(80);

        // Phase B: front pair pushes back, rear pair sweeps forward
        legs_t phase_b_front = {70, 70, 60, 60};  // front moves first
        legs_t phase_b_full = {70, 70, 110, 110}; // rear follows
        ease_to(phase_b_front, 250);
        ease_to(phase_b_full, 250);
        delay_ms(80);
    }

    ease_to(stance, 400);
    ease_to((legs_t){90, 90, 90, 90}, 500);
    delay_ms(200);
    pwm_off_all();
}

void anim_dance(void)
{
    reset_to_neutral();

    // --- Move 1: Swagger — diagonal dip, sets the rhythm ---
    legs_t swagger_a = {60, 90, 90, 60};
    legs_t swagger_b = {90, 60, 60, 90};

    for (int i = 0; i < 4; i++)
    {
        ease_to(swagger_a, 350);
        delay_ms(100);
        ease_to(swagger_b, 350);
        delay_ms(100);
    }

    ease_to((legs_t){90, 90, 90, 90}, 400);
    delay_ms(200);

    // --- Move 2: Forward lean — all legs sweep back, bot tips nose down ---
    legs_t lean_forward = {120, 120, 120, 120};
    legs_t lean_back = {60, 60, 60, 60};

    ease_to(lean_forward, 600);
    delay_ms(300);
    ease_to(lean_back, 600);
    delay_ms(300);
    ease_to(lean_forward, 500);
    delay_ms(200);
    ease_to(lean_back, 500);
    delay_ms(200);
    ease_to(lean_forward, 400);
    delay_ms(150);
    ease_to(lean_back, 400);
    delay_ms(150);

    ease_to((legs_t){90, 90, 90, 90}, 500);
    delay_ms(300);

    // --- Move 3: Lie down ---
    legs_t front = {60, 60, 90, 90};
    legs_t all_down = {80, 80, 60, 60};
    legs_t flat = {180, 180, 0, 0};

    ease_to(front, 500);
    delay_ms(150);
    ease_to(all_down, 500);
    delay_ms(100);
    ease_to(flat, 700);
    delay_ms(800); // dramatic hold

    // --- Move 4: Floor shimmy — small leg wiggles while lying flat ---
    // Front legs tap side to side, rear legs pulse
    legs_t shimmy_a = {170, 180, 0, 10};
    legs_t shimmy_b = {180, 170, 10, 0};

    for (int i = 0; i < 5; i++)
    {
        ease_to(shimmy_a, 250);
        delay_ms(50);
        ease_to(shimmy_b, 250);
        delay_ms(50);
    }

    ease_to(flat, 300);
    delay_ms(400); // pause before getting up

    // --- Move 5: Get back up ---
    ease_to(all_down, 600);
    delay_ms(150);
    ease_to(front, 600);
    delay_ms(150);
    ease_to((legs_t){90, 90, 90, 90}, 600);
    delay_ms(300);

    pwm_off_all();
}