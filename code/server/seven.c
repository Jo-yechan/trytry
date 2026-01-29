// seven.c
#include "seven.h"
#include <wiringPi.h>
#include <unistd.h>

// WiringPi 핀 번호 기준 (A, B, C, D)
static int gpiopins[4] = {4, 1, 16, 15};

// 세븐세그먼트 초기화
void seven_segment_init(void) {
    wiringPiSetup(); // WiringPi 핀 번호 기준

    for (int i = 0; i < 4; i++)
        pinMode(gpiopins[i], OUTPUT);

    display_off(); // 초기 상태 OFF
}

// 숫자 출력 (0~9)
void display_digit(int num) {
    if (num < 0 || num > 9) num = 0;

    int number[10][4] = {
        {0,0,0,0}, // 0
        {0,0,0,1}, // 1
        {0,0,1,0}, // 2
        {0,0,1,1}, // 3
        {0,1,0,0}, // 4
        {0,1,0,1}, // 5
        {0,1,1,0}, // 6
        {0,1,1,1}, // 7
        {1,0,0,0}, // 8
        {1,0,0,1}  // 9
    };

    for (int i = 0; i < 4; i++)
        digitalWrite(gpiopins[i], number[num][i] ? HIGH : LOW);
}

// 세그먼트 끄기
void display_off(void) {
    for (int i = 0; i < 4; i++)
        digitalWrite(gpiopins[i], HIGH); // HIGH → OFF
}
