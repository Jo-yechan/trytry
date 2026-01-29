#ifndef BUZZER_H
#define BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

int buzzer_init(int wiringpi_pin);
void buzzer_on(void);      // 비블로킹 시작
void buzzer_off(void);     // 즉시 멈춤
void buzzer_close(void);

#ifdef __cplusplus
}
#endif
#endif
