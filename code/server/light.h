#ifndef LIGHT_H
#define LIGHT_H

// 조도 센서 초기화
// wiringPi 번호를 인자로 받음
int light_sensor_init(int wiringpi_pin);

// 조도 센서 값 읽기
// 반환값:
//   1 (HIGH) : 밝음
//   0 (LOW)  : 어두움
int light_sensor_read(void);

// 조도 센서 종료 (선택)
void light_sensor_close(void);

#endif // LIGHT_H
