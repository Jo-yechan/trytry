#include <wiringPi.h>
#include <softPwm.h>
#include "led.h"

#define LED_PIN 26

static int led_pin = -1;
static led_level_t current_level = LED_OFF;
static int initialized = 0;

// PWM 단계 값 (0~100)
static const int pwm_table[] = {
    100,    // OFF
    70,   // LOW
    40,   // MID
    0   // HIGH
};

int led_init(int wiringpi_pin)
{
    if (wiringPiSetup() == -1) {
        return -1;
    }

    led_pin = wiringpi_pin;

    if (!initialized) {
        pinMode(led_pin, OUTPUT);
        softPwmCreate(led_pin, 0, 100); // 초기값 0, 최대값 100
        initialized = 1;
    }

    current_level = LED_OFF;
    return 0;
}

void led_set_level(led_level_t level)
{
    if (led_pin < 0) return;
    if (level < LED_OFF || level > LED_HIGH) return;

    softPwmWrite(led_pin, pwm_table[level]);
    current_level = level;
}

void led_on(void)
{
    if (current_level == LED_OFF) {
        led_set_level(LED_MID); // 기본 ON 밝기
    } else {
        led_set_level(current_level); // 이전 밝기 유지
    }
}

void led_off(void)
{
    led_set_level(LED_OFF);
}

led_level_t led_get_level(void)
{
    return current_level;
}

void led_close(void)
{
    if (led_pin >= 0) {
        softPwmWrite(led_pin, 0);
        pinMode(led_pin, INPUT);
        initialized = 0;
        led_pin = -1;
        current_level = LED_OFF;
    }
}
