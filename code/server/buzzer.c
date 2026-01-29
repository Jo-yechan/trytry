#include "buzzer.h"
#include <wiringPi.h>
#include <softTone.h>
#include <stdio.h>
#include <pthread.h>

// ===== 설정 =====
#define TOTAL_NOTES 32
static int notes[TOTAL_NOTES] = {
    391, 391, 440, 440, 391, 391, 329,
    391, 391, 329, 329, 293, 293, 293,
    391, 391, 440, 440, 391, 391, 329,
    391, 329, 293, 329, 261, 261, 261, 261,
    0, 0
};

static int buzzer_pin = -1;
static int initialized = 0;
static volatile int playing = 0;         // 재생 중 플래그
static pthread_t melody_thread = 0;      // 재생 스레드

// 실제 멜로디 재생 함수 (스레드에서 실행)
void *melody_play_thread(void *arg) {
    (void)arg;
    for (int i = 0; i < TOTAL_NOTES && playing; i++) {
        softToneWrite(buzzer_pin, notes[i]);
        for (int j = 0; j < 28 && playing; j++) {  // 280ms
            delay(10);
        }
    }
    softToneWrite(buzzer_pin, 0);
    playing = 0;
    return NULL;
}

int buzzer_init(int wiringpi_pin) {
    if (initialized) return 0;
    if (wiringPiSetup() == -1) {
        printf("wiringPiSetup failed\n");
        return -1;
    }
    buzzer_pin = wiringpi_pin;
    if (softToneCreate(buzzer_pin) != 0) {
        printf("softToneCreate failed\n");
        return -1;
    }
    initialized = 1;
    buzzer_off();
    return 0;
}

void buzzer_on(void) {
    if (!initialized || playing) return;  // 이미 재생 중이면 무시

    playing = 1;
    pthread_create(&melody_thread, NULL, melody_play_thread, NULL);
    // 바로 리턴 → 서버 응답 가능!
}

void buzzer_off(void) {
    if (!initialized) return;
    playing = 0;
    if (melody_thread != 0) {
        pthread_join(melody_thread, NULL);  // 안전하게 기다림 (또는 detach)
        melody_thread = 0;
    }
    softToneWrite(buzzer_pin, 0);
}

void buzzer_close(void) {
    if (!initialized) return;
    buzzer_off();
    buzzer_pin = -1;
    initialized = 0;
}
