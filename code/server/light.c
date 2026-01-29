#include <wiringPi.h>
#include "light.h"

static int cds_pin = -1;

int light_sensor_init(int wiringpi_pin)
{
    cds_pin = wiringpi_pin;

    // pinMode만 설정 (풀업/풀다운 건드리지 않음)
    pinMode(cds_pin, INPUT);

    return 0;
}

int light_sensor_read(void)
{
    if (cds_pin < 0)
        return -1;

    // 회로 풀업 기준
    // HIGH : 밝음
    // LOW  : 어두움
    return digitalRead(cds_pin);
}

void light_sensor_close(void)
{
    if (cds_pin >= 0)
        pinMode(cds_pin, INPUT);
}
