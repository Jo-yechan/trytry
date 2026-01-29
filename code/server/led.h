#ifndef LED_H
#define LED_H

#ifdef __cplusplus
extern "C" {
#endif

// LED 밝기 단계
typedef enum {
    LED_OFF = 0,
    LED_LOW,
    LED_MID,
    LED_HIGH
} led_level_t;

// 초기화 (wiringPi 번호)
int led_init(int wiringpi_pin);

// ON (이전 밝기 유지)
void led_on(void);

// OFF
void led_off(void);

// 밝기 설정
void led_set_level(led_level_t level);

// 현재 밝기 확인
extern led_level_t led_get_level(void);

// 종료
void led_close(void);

#ifdef __cplusplus
}
#endif

#endif
