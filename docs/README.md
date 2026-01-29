# 🔴 라즈베리 파이 GPIO 제어 시스템

라즈베리 파이의 GPIO 핀을 활용한 실시간 제어 시스템입니다. LED, 부저, 조도 센서, 7-세그먼트 디스플레이를 제어할 수 있으며, 웹 모니터링과 TCP/IP 클라이언트 명령어 제어를 지원합니다.

## 📋 목차

- [평가 항목 구현 여부](#평가-항목-구현-여부)
- [시스템 구성](#시스템-구성)
- [기능](#기능)
- [하드웨어 연결](#하드웨어-연결)
- [설치 및 빌드](#설치-및-빌드)
- [사용 방법](#사용-방법)
- [명령어 목록](#명령어-목록)
- [프로젝트 구조](#프로젝트-구조)

---

## ✅ 평가 항목 구현 여부

### 【실습주제】
- [v] **TCP를 이용한 원격 장치 제어 프로그램**
  - 클라이언트-서버 구조로 TCP/IP 소켓 통신 구현
  - 포트 9090으로 명령 수신, 포트 8080으로 웹 모니터링 제공
- [v] **서버: 라즈베리파이어**
  - 라즈베리 파이에서 서버 프로세스 실행
- [v] **클라이언트: 우분투 리눅스**
  - 우분투/리눅스 환경에서 클라이언트 실행 가능

### 【구현 내용】
- [v] **멀티 프로세스 또는 스레드 이용한 장치 제어**
  - 멀티스레드 구현:
    - 웹 서버 스레드 (`web_server_thread`)
    - 조도 센서 자동 제어 스레드 (`auto_light_control`)
    - 카운트다운 스레드 (`countdown_func`)
    - 부저 멜로디 재생 스레드 (`melody_play_thread`)
  - `pthread` 라이브러리 사용
  - `pthread_mutex`를 통한 동기화 처리

- [v] **대상 장치: LED/부저/조도센서/7세그먼트**
  - **LED**: PWM 제어로 4단계 밝기 조절 (OFF/LOW/MID/HIGH)
  - **부저**: 학교종 멜로디 32음 재생
  - **조도센서**: 디지털 입력으로 밝기 감지, 자동 LED 제어
  - **7세그먼트**: BCD 입력으로 0~9 숫자 표시 및 카운트다운

- [v] **장치 제어 프로그램은 공유 라이브러리 형식으로 작성하여 필요 시 각 장치 기능 업데이트하여 수정 각 장치 기능 업데이트하여 수정 가능하게 구성**
  - 공유 라이브러리(.so) 생성:
    - `libled.so` - LED 제어
    - `libbuzzer.so` - 부저 제어
    - `liblight.so` - 조도센서 제어
    - `libseven.so` - 7세그먼트 제어
  - 모듈별 독립적 업데이트 가능

- [v] **클라이언트 프로그램 실행 도중 강제 종료되지 않도록 시그널 처리**
  - SIGINT, SIGTERM, SIGHUP 시그널 핸들러 구현
  - 안전한 종료 처리 (리소스 정리, 소켓 종료)
  - 서버: `signal_handler()` 함수로 GPIO 정리 및 소켓 종료
  - 클라이언트: `sig_handler()` 함수로 안전한 종료 처리

- [v] **서버는 대몬(Daemon) 프로세스 형식으로 구성**
  - `become_daemon()` 함수로 데몬화 구현
  - 이중 fork()로 완전한 데몬 프로세스 생성
  - 표준 입출력 `/dev/null`로 리다이렉트
  - 백그라운드 실행으로 터미널 독립적 동작
  - 3초 대기 후 자동 데몬 전환

- [v] **빌드 자동화(make 또는 cmake 이용)**
  - Makefile 구현:
    - 서버용 Makefile: 공유 라이브러리 및 서버 바이너리 빌드
    - 클라이언트용 Makefile: 클라이언트 바이너리 빌드
  - `make`, `make run`, `make clean` 명령어 지원
  - 자동 디렉토리 생성 (`create_dirs` 타겟)

### 【장치 동작】
- [v] **LED**
  - 클라이언트에서 on/off 및 밝기(출력/중간/최저)를 설정
  - 명령어: `led on`, `led off`, `led low`, `led mid`, `led high`
  - PWM 제어로 세밀한 밝기 조절

- [v] **부저**
  - 클라이언트에서(음악)소리 on/off 제어
  - 명령어: `music on`, `music off`
  - 학교종 멜로디 32음 재생
  - 비동기 재생으로 서버 응답성 유지

- [v] **조도센서**
  - 클라이언트에서 조도 센서 값 확인
  - 명령어: `light read` (현재 값 확인)
  - 빛이 감지되지 않으면 LED on, 빛이 감지되면 LED off
  - 명령어: `light` (자동 제어 모드 시작)
  - 실시간 모니터링으로 자동 LED 제어

- [v] **7세그먼트**
  - 클라이언트에서 전송한 숫자(0~9) 표시(초 단위 시간)
  - 명령어: `seven <0-9>` (예: `seven 5`)
  - 1초 간격 페이드 감소 표시
  - 카운트다운 완료 시 자동 멜로디 재생
  - 0이 되면 부저 울림

### 【추가 기능】
- [v] **웹 브라우저를 통한 실시간 상태 모니터링**
 - HTTP 웹 서버 구현 (포트 8080)
 - 독립적인 웹 서버 스레드로 동작
 - 실시간 GPIO 상태 확인 가능:

 - LED 현재 상태 (OFF/LOW/MID/HIGH)
 - 조도 센서 값 (어두움/밝음)
 - 7세그먼트 표시 숫자 (0~9 또는 대기 중)

 - 0.1초 자동 갱신으로 실시간 모니터링
 - 반응형 다크 모드 UI 디자인
 - 모바일/PC 브라우저 모두 접근 가능
 - 주소: http://<라즈베리파이_IP>:8080

### 【제출내용】
- [v] **개발문서: 프로젝트 개요, 개발 일정, 세부 구현 내용, 문제점 및 보완 사항**
  - 본 README.md에 포함
- [v] **개발 소스코드 및 빌드 방법(README 파일 작성)**
  - 전체 소스코드 `code/` 디렉토리에 포함
  - README에 빌드 및 실행 방법 상세 기술
- [v] **실행 과정 text 파일**
  - `docs/running.txt` - 서버/클라이언트 실행 과정 캡처
- [v] **회로도**
  - `misc/connection.png` - GPIO 핀 연결 회로도
- [v] **사용자 매뉴얼**
  - `docs/manual.pdf` - 상세 실행 방법 및 사용 설명서

## 🎯 시스템 구성

### 서버 (라즈베리 파이)
- **데몬 프로세스**: 백그라운드에서 실행되는 GPIO 제어 서버
- **웹 서버**: 실시간 GPIO 상태 모니터링 (포트 8080)
- **명령 서버**: TCP/IP 클라이언트 명령 수신 (포트 9090)
- **멀티스레드**: 웹, 조도센서, 카운트다운, 멜로디 각각 독립 스레드

### 클라이언트 (우분투/리눅스)
- 네트워크를 통한 원격 GPIO 제어
- 대화형 CLI 인터페이스
- TCP/IP 소켓 통신

---

## ✨ 기능

### 1. LED 제어
- PWM을 이용한 4단계 밝기 조절
  - OFF / LOW / MID / HIGH
- 조도 센서 연동 자동 제어
- 실시간 밝기 조절 가능

### 2. 부저 (음악 재생)
- "학교종" 멜로디 재생 (32음)
- 비동기 재생으로 서버 응답성 유지
- 재생 중 즉시 정지 가능
- 카운트다운 완료 시 자동 재생

### 3. 조도 센서
- 실시간 빛 감지 (디지털 입력)
- 자동 LED 제어 모드
  - 어두우면 LED 자동 점등 (MID)
  - 밝으면 LED 자동 소등
- 수동 조도 값 확인 가능

### 4. 7-세그먼트 디스플레이
- 0~9 숫자 표시 (BCD 입력)
- 카운트다운 기능 (1초 간격)
- 카운트다운 종료 시 자동 멜로디 재생
- 실시간 웹 모니터링

### 5. 웹 모니터링
- 실시간 GPIO 상태 확인
- 0.1초 자동 갱신
- LED 상태, 조도 센서 값, 7세그먼트 표시 확인
- 모바일 접근 가능

---

## 🔌 하드웨어 연결

| 디바이스 | WiringPi 핀 | BCM 핀 | 설명 |
|---------|------------|--------|------|
| LED | 26 | GPIO 12 | PWM 제어 |
| 부저 | 29 | GPIO 21 | 멜로디 출력 |
| 조도 센서 | 14 | GPIO 11 | 디지털 입력 |
| 7-세그먼트 A | 4 | GPIO 7 | BCD 입력 |
| 7-세그먼트 B | 1 | GPIO 18 | BCD 입력 |
| 7-세그먼트 C | 16 | GPIO 4 | BCD 입력 |
| 7-세그먼트 D | 15 | GPIO 3 | BCD 입력 |

---

## 🛠 설치 및 빌드

### 필수 요구사항
```bash
# 라즈베리 파이에서 실행
sudo apt-get update
sudo apt-get install -y wiringpi gcc make
```

### 서버 빌드 및 실행

#### 1. 서버 디렉토리로 이동
```bash
cd code/server
```

#### 2. 빌드
```bash
make
```

빌드 결과물:
- `../../exec/server/server` - 서버 실행 파일
- `../../exec/lib/libled.so` - LED 제어 공유 라이브러리
- `../../exec/lib/libbuzzer.so` - 부저 제어 공유 라이브러리
- `../../exec/lib/libseven.so` - 7세그먼트 제어 공유 라이브러리
- `../../exec/lib/liblight.so` - 조도센서 제어 공유 라이브러리

#### 3. 서버 실행
```bash
sudo make run
# 또는
sudo ../../exec/server/server
```

서버 시작 시 표시되는 정보:
- 🔴 라즈베리 파이 IP 주소
- 🌐 웹 포트: 8080
- 📡 명령 포트: 9090
- ⏳ 3초 후 자동 데몬 전환

### 클라이언트 빌드 및 실행

#### 1. 클라이언트 디렉토리로 이동
```bash
cd code/client
```

#### 2. 빌드
```bash
make
```

빌드 결과물:
- `../../exec/client/client` - 클라이언트 실행 파일

#### 3. 클라이언트 실행
```bash
../../exec/client/client <라즈베리파이_IP>

# 예시
../../exec/client/client 192.168.1.100
```

### 빌드 정리
```bash
# 서버 정리
cd code/server
make clean

# 클라이언트 정리
cd code/client
make clean
```

---

## 🚀 사용 방법

### 1. 서버 시작
```bash
sudo make run
```

서버가 시작되면 다음 정보가 표시됩니다:
```
==========================================
🔴 라즈베리 파이 GPIO 서버 시작됨
🌐 IP 주소: 192.168.1.100
==========================================
⏳ 3초 후 백그라운드 데몬으로 전환됩니다...
```

3초 후 자동으로 데몬 프로세스로 전환됩니다.

### 2. 웹 모니터링 접근
브라우저에서 접속:
```
http://<라즈베리파이_IP>:8080
```

실시간으로 다음 정보 확인:
- LED 상태 (OFF/LOW/MID/HIGH)
- 조도 센서 값 (어두움/밝음)
- 7세그먼트 표시 (숫자 또는 대기 중)

### 3. 클라이언트 연결
```bash
../../exec/client/client <라즈베리파이_IP>

# 예시
../../exec/client/client 192.168.1.100
```

클라이언트 실행 시 도움말 자동 표시:
```
==========================================================
            라즈베리 파이 GPIO 제어 클라이언트
==========================================================
사용 가능한 명령어:
  ...
==========================================================
연결 대상 서버: 192.168.1.100:9090
> 
```

### 4. 서버 종료
```bash
sudo pkill -SIGINT server
```

---

## 📝 명령어 목록

### LED 제어
```bash
led on          # LED 켜기 (기본 밝기 MID)
led off         # LED 끄기
led low         # LED 약하게 (PWM 70%)
led mid         # LED 중간 (PWM 40%)
led high        # LED 강하게 (PWM 0% - 최대 밝기)
```

### 조도 센서
```bash
light           # 자동 제어 모드 시작
                # (어두우면 LED MID 켜짐, 밝으면 꺼짐)
light read      # 현재 조도 값 확인
                # 반환값: 0 = 밝음, 1 = 어두움
```

### 음악 재생
```bash
music on        # 학교종 멜로디 재생 (비동기)
music off       # 멜로디 즉시 멈춤
```

### 7-세그먼트 카운트다운
```bash
seven <0-9>     # 숫자부터 카운트다운 후 멜로디 재생
                
# 예시
seven 5         # 5 → 4 → 3 → 2 → 1 → 0 → 🎵 멜로디
seven 9         # 9부터 카운트다운
```

### 클라이언트 제어 명령어
```bash
help            # 도움말 다시 보기
quit            # 클라이언트 종료
exit            # 클라이언트 종료
Ctrl+C          # 강제 종료
```

---

## 📂 프로젝트 구조

```
.
├── code/                        # 소스 코드 디렉토리
│   ├── client/                  # 클라이언트 소스
│   │   ├── client.c             # TCP/IP 클라이언트 메인
│   │   ├── client.o             # 컴파일된 오브젝트 파일
│   │   └── Makefile             # 클라이언트 빌드 설정
│   │
│   └── server/                  # 서버 소스
│       ├── server.c             # 메인 서버 (데몬, 웹, 명령 처리)
│       ├── server.o             # 컴파일된 오브젝트 파일
│       ├── led.c / led.h        # LED PWM 제어 모듈
│       ├── led.o                # LED 오브젝트 파일
│       ├── buzzer.c / buzzer.h  # 부저 멜로디 재생 모듈
│       ├── buzzer.o             # 부저 오브젝트 파일
│       ├── seven.c / seven.h    # 7-세그먼트 디스플레이 모듈
│       ├── seven.o              # 7세그먼트 오브젝트 파일
│       ├── light.c / light.h    # 조도 센서 모듈
│       ├── light.o              # 조도센서 오브젝트 파일
│       └── Makefile             # 서버 빌드 설정
│
├── exec/                        # 실행 파일 디렉토리 (빌드 결과물)
│   ├── server/
│   │   └── server               # 서버 실행 파일
│   ├── client/
│   │   └── client               # 클라이언트 실행 파일
│   └── lib/                     # 공유 라이브러리
│       ├── libled.so            # LED 제어 라이브러리
│       ├── libbuzzer.so         # 부저 제어 라이브러리
│       ├── libseven.so          # 7세그먼트 제어 라이브러리
│       └── liblight.so          # 조도센서 제어 라이브러리
│
├── docs/                        # 문서 디렉토리
│   ├── manual.pdf               # 사용자 매뉴얼 (실행 방법)
│   └── running.txt              # 실행 과정 텍스트 (클라이언트/서버 실행 예시)
│
├── misc/                        # 기타 파일
│   └── connection.png           # 하드웨어 회로도
│
└── README.md                    # 프로젝트 설명서 (본 문서)
```

### 디렉토리 설명

#### `code/`
- 모든 소스 코드 파일 포함
- `client/`: 클라이언트 관련 소스 및 빌드 파일
- `server/`: 서버 및 GPIO 제어 모듈 소스

#### `exec/`
- 빌드 후 생성되는 실행 파일 및 라이브러리
- `make` 명령 실행 시 자동 생성
- `server/`: 서버 바이너리
- `client/`: 클라이언트 바이너리
- `lib/`: 공유 라이브러리 (.so 파일)

#### `docs/`
- **manual.pdf**: 상세한 실행 방법 및 사용 설명서
- **running.txt**: 실제 서버/클라이언트 실행 화면 캡처

#### `misc/`
- **connection.png**: GPIO 핀 연결 회로도
  - LED, 부저, 조도센서, 7세그먼트 연결 방법 표시

---

## 🔧 주요 기능 상세 설명

### 1. 멀티스레딩 구조

#### 스레드 구성
| 스레드 | 함수명 | 역할 | 실행 주기 |
|--------|--------|------|-----------|
| 웹 서버 | `web_server_thread` | HTTP 요청 처리 및 상태 페이지 제공 | 요청 시 |
| 조도 센서 | `auto_light_control` | 자동 LED 제어 모드 관리 | 100ms |
| 카운트다운 | `countdown_func` | 7세그먼트 타이머 관리 | 1초 |
| 멜로디 재생 | `melody_play_thread` | 비동기 음악 재생 | 280ms/음 |

#### 동기화 메커니즘
- `pthread_mutex_t mutex`: 공유 자원 보호
  - `current_digit`: 7세그먼트 현재 표시 값
  - `light_test_mode`: 조도 센서 모드 플래그
  - LED 상태 변경
- 스레드 안전한 시작/종료 처리
- 카운트다운 중복 실행 방지

### 2. 데몬화 프로세스

#### `become_daemon()` 구현 단계
1. **첫 번째 fork()**: 부모 프로세스 종료
2. **setsid()**: 새로운 세션 생성, 터미널 분리
3. **SIGHUP 무시**: 터미널 종료 시그널 차단
4. **두 번째 fork()**: 세션 리더 제거
5. **umask(0)**: 파일 권한 마스크 초기화
6. **chdir("/")**: 작업 디렉토리 루트로 변경
7. **파일 디스크립터 정리**: 모든 열린 파일 닫기
8. **표준 입출력 리다이렉트**: `/dev/null`로 연결

#### 데몬 전환 타이밍
- 서버 시작 후 IP 주소 표시
- 3초 대기 (사용자 확인)
- 자동 데몬 전환
- 이후 SIGINT 핸들러 재설정

### 3. 공유 라이브러리 설계

#### 모듈별 독립성
각 장치 제어 모듈은 독립적인 `.so` 파일로 컴파일:
- **장점**:
  - 모듈별 개별 업데이트 가능
  - 재컴파일 시간 단축
  - 메모리 효율성
  - 코드 재사용성 향상

#### 라이브러리 인터페이스
```c
// LED 모듈
int led_init(int wiringpi_pin);
void led_set_level(led_level_t level);
void led_on(void);
void led_off(void);
void led_close(void);

// 부저 모듈
int buzzer_init(int wiringpi_pin);
void buzzer_on(void);   // 비동기 재생
void buzzer_off(void);  // 즉시 정지
void buzzer_close(void);

// 조도 센서 모듈
int light_sensor_init(int wiringpi_pin);
int light_sensor_read(void);  // 0=밝음, 1=어두움
void light_sensor_close(void);

// 7세그먼트 모듈
void seven_segment_init(void);
void display_digit(int num);  // 0~9
void display_off(void);
```

### 4. 시그널 처리

#### 서버 시그널 핸들러
```c
// 무시되는 시그널 (데몬 안정성)
signal(SIGTERM, SIG_IGN);   // kill 명령
signal(SIGHUP, SIG_IGN);    // 터미널 종료
signal(SIGTSTP, SIG_IGN);   // Ctrl+Z
signal(SIGQUIT, SIG_IGN);   // Ctrl+\

// 처리되는 시그널
signal(SIGINT, signal_handler);  // Ctrl+C - 안전한 종료
```

#### 안전한 종료 절차
1. `is_run = 0` 설정 → 모든 스레드 루프 종료
2. GPIO 리소스 정리:
   - `led_close()`
   - `buzzer_close()`
   - `light_sensor_close()`
3. 소켓 종료:
   - `shutdown(server_sock, SHUT_RDWR)`
   - `close(server_sock)`
4. 프로세스 종료

#### 클라이언트 시그널 핸들러
- SIGINT 처리: 안전한 종료 메시지 출력
- SIGTERM, SIGHUP 무시: 의도하지 않은 종료 방지

### 5. 웹 모니터링 시스템

#### 실시간 업데이트
- HTTP/1.1 프로토콜
- `<meta http-equiv="refresh" content="0.1">` - 0.1초 자동 갱신
- UTF-8 인코딩으로 한글 지원

#### 표시 정보
- **LED 상태**: OFF / LOW / MID / HIGH
- **조도 센서**: 어두움 / 밝음
- **7세그먼트**: 현재 숫자 또는 "대기 중"

#### 반응형 디자인
- 다크 모드 UI
- 테이블 레이아웃
- 모바일 브라우저 지원

---

## 🔒 보안 및 권한

### 실행 권한
- 서버는 `sudo` 권한으로 실행 필요 (GPIO 접근)
- wiringPi 라이브러리는 root 권한 요구

### 포트 관리
- 자동 포트 충돌 해결:
  ```bash
  fuser -k -n tcp 8080 9090
  ```
- SO_REUSEADDR 옵션으로 빠른 재시작 지원

### 데몬 보안
- 표준 입출력 `/dev/null`로 리다이렉트
- 작업 디렉토리 루트(`/`)로 변경
- umask(0)로 파일 권한 제어

---

## 🐛 문제 해결

### 1. 포트 충돌
**증상**: "Address already in use" 오류

**해결**:
```bash
# 자동 해결 (서버 시작 시 자동 실행됨)
sudo fuser -k -n tcp 8080 9090

# 수동 확인
sudo netstat -tulpn | grep -E '8080|9090'
```

### 2. 권한 오류
**증상**: "Permission denied" 또는 GPIO 접근 불가

**해결**:
```bash
# sudo로 실행
sudo ../../exec/server/server

# 또는 setuid 비트 설정 (권장하지 않음)
sudo chown root:root ../../exec/server/server
sudo chmod +s ../../exec/server/server
```

### 3. wiringPi 오류
**증상**: "wiringPiSetup failed"

**해결**:
```bash
# wiringPi 설치 확인
gpio -v

# 핀 상태 확인
gpio readall

# wiringPi 재설치
sudo apt-get install --reinstall wiringpi
```

### 4. 클라이언트 연결 실패
**증상**: "서버 연결 실패"

**해결**:
- IP 주소 확인:
  ```bash
  hostname -I
  ```
- 서버 실행 확인:
  ```bash
  ps aux | grep server
  ```
- 방화벽 확인:
  ```bash
  sudo ufw status
  sudo ufw allow 8080/tcp
  sudo ufw allow 9090/tcp
  ```

### 5. 데몬 프로세스 종료
**증상**: 데몬이 백그라운드에서 계속 실행 중

**해결**:
```bash
# 프로세스 찾기
ps aux | grep server

# SIGINT로 안전하게 종료
sudo pkill -SIGINT server

# 강제 종료 (권장하지 않음)
sudo pkill -9 server
```

---

## 📊 성능 및 제약사항

### 성능 특성
- **웹 갱신 주기**: 0.1초 (10 FPS)
- **조도 센서 체크**: 100ms (light_test_mode 시)
- **카운트다운 정확도**: ±10ms 이내
- **멜로디 재생**: 32음, 총 약 9초
- **TCP 응답 시간**: <10ms (로컬 네트워크)

### 제약사항
- 동시 클라이언트 연결: 순차 처리 (동시 연결 미지원)
- 7세그먼트: 0~9 단일 자릿수만 표시
- 멜로디: 1개 고정 (학교종)
- wiringPi 핀 번호 고정 (수정 시 재컴파일 필요)

### 리소스 사용
- **메모리**: 약 5MB (데몬 프로세스)
- **CPU**: 평균 5% 미만 (라즈베리 파이 4 기준)
- **네트워크**: 웹 모니터링 시 약 1KB/s

---

## 🎓 학습 포인트

### 시스템 프로그래밍
- 멀티스레드 프로그래밍 (`pthread`)
- 데몬 프로세스 구현
- 시그널 핸들링
- 소켓 프로그래밍 (TCP/IP)

### 임베디드 시스템
- GPIO 제어 (wiringPi)
- PWM 신호 생성
- 디지털 입출력
- 센서 인터페이싱

### 소프트웨어 공학
- 모듈화 설계
- 공유 라이브러리 구조
- Makefile 작성
- 버전 관리

---

## 📄 라이선스

이 프로젝트는 교육 목적으로 제작되었습니다.