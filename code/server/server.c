#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <wiringPi.h>
#include "led.h"
#include "buzzer.h"
#include "light.h"
#include "seven.h"

#define WEB_PORT 8080
#define CMD_PORT 9090
#define BUZZER_PIN 29
#define LED_PIN 26
#define LIGHT_SENSOR_PIN 14

static int server_sock = -1;
static int current_digit = -1;
static pthread_t countdown_thread = 0;
static int light_test_mode = 0;
static volatile int is_run = 1;
static pthread_t web_thread;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void become_daemon() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    if (setsid() < 0) exit(EXIT_FAILURE);
    signal(SIGHUP, SIG_IGN);
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    umask(0);
    chdir("/");
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) close(x);
    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
}

void print_local_ip() {
    struct ifaddrs *ifaddr, *ifa;
    char ip[INET_ADDRSTRLEN] = "0.0.0.0";
    if (getifaddrs(&ifaddr) == -1) {
        strcpy(ip, "127.0.0.1");
    } else {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) continue;
            if (strcmp(ifa->ifa_name, "lo") == 0) continue;
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
            if (strncmp(ifa->ifa_name, "eth", 3) == 0 || strncmp(ifa->ifa_name, "wlan", 4) == 0) break;
        }
        freeifaddrs(ifaddr);
    }
    printf("\n==========================================\n");
    printf("🔴 라즈베리 파이 GPIO 서버 시작됨\n");
    printf("🌐 IP 주소: %s\n", ip);
    printf("==========================================\n");
    printf("⏳ 3초 후 백그라운드 데몬으로 전환됩니다...\n");
    fflush(stdout);
    sleep(3);
}

void *countdown_func(void *arg) {
    int digit = *(int*)arg;
    free(arg);
    for (int i = digit; i >= 0 && is_run; i--) {
        pthread_mutex_lock(&mutex);
        current_digit = i;
        pthread_mutex_unlock(&mutex);
        display_digit(i);
        sleep(1);
    }
    if (is_run) buzzer_on();
    pthread_mutex_lock(&mutex);
    current_digit = -1;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void start_countdown(int digit) {
    if (countdown_thread != 0) {
        pthread_cancel(countdown_thread);
        pthread_join(countdown_thread, NULL);
    }
    int *d = malloc(sizeof(int));
    *d = digit;
    pthread_create(&countdown_thread, NULL, countdown_func, d);
}

void *auto_light_control(void *arg) {
    (void)arg;
    int last_light = -1;
    while (is_run) {
        if (light_test_mode) {
            int light = light_sensor_read();
            if (light != last_light) {
                if (light == 1) led_set_level(LED_MID);
                else led_off();
                last_light = light;
            }
            delay(100);
        } else delay(1000);
    }
    return NULL;
}

int send_web_page(FILE* fp) {
    led_level_t lvl = led_get_level();
    const char* led_str = (lvl == LED_OFF) ? "OFF" : (lvl == LED_LOW) ? "LOW" : (lvl == LED_MID) ? "MID" : "HIGH";
    int light_val = light_sensor_read();
    const char* light_str = (light_val == 1) ? "어두움" : "밝음";
    int seven_val = current_digit;
    char seven_display[50];
    if (seven_val >= 0) snprintf(seven_display, sizeof(seven_display), "%d 카운트다운", seven_val);
    else snprintf(seven_display, sizeof(seven_display), "대기 중");

    char html[4096];
    snprintf(html, sizeof(html),
        "<!DOCTYPE html><html lang=\"ko\"><head><meta charset=\"UTF-8\">"
        "<title>GPIO 실시간 모니터</title><meta http-equiv=\"refresh\" content=\"0.1\">"
        "<style>"
        "body {font-family: 'Malgun Gothic', Arial; text-align: center; background: #1e1e1e; color: white;}"
        "h1 {font-size: 48px; margin-top: 50px;}"
        ".container {max-width: 800px; margin: auto;}"
        "table {width: 100%%; font-size: 36px; border-collapse: collapse; margin: 30px 0;}"
        "td {padding: 25px; border: 2px solid #444; background: #333;}"
        ".label {background: #555; width: 40%%;}"
        ".value {font-weight: bold; color: #0f0;}"
        "</style></head><body><div class=\"container\">"
        "<h1>🔴 라즈베리 파이 GPIO 상태</h1>"
        "<table>"
        "<tr><td class=\"label\">LED 상태</td><td class=\"value\">%s</td></tr>"
        "<tr><td class=\"label\">조도 센서</td><td class=\"value\">%s</td></tr>"
        "<tr><td class=\"label\">Seven Segment</td><td class=\"value\">%s</td></tr>"
        "</table><p style=\"color:#888\">0.1초마다 자동 갱신 중</p></div>"
        "</body></html>", led_str, light_str, seven_display);

    fprintf(fp, "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n%s", (long)strlen(html), html);
    fflush(fp);
    return 0;
}

void* web_server_thread(void* arg) {
    (void)arg;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(WEB_PORT);
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock, 5);
    while (is_run) {
        int client = accept(sock, NULL, NULL);
        if (client < 0) continue;
        char buf[1024];
        if (recv(client, buf, sizeof(buf), 0) > 0 && strstr(buf, "GET")) {
            FILE* fp = fdopen(dup(client), "w");
            if (fp) { send_web_page(fp); fclose(fp); }
        }
        close(client);
    }
    close(sock);
    return NULL;
}

void handle_command(char *cmd, int client_sock) {
    char response[256] = {0};
    pthread_mutex_lock(&mutex);

    // 1. LED 관련 명령 (상세 레벨 포함)
    if (strncmp(cmd, "led on", 6) == 0) {
        light_test_mode = 0; led_on(); 
        strcpy(response, "LED turned on\n");
    } else if (strncmp(cmd, "led off", 7) == 0) {
        light_test_mode = 0; led_off(); 
        strcpy(response, "LED turned off\n");
    } else if (strncmp(cmd, "led low", 7) == 0) {
        light_test_mode = 0; led_set_level(LED_LOW);
        strcpy(response, "LED set to low\n");
    } else if (strncmp(cmd, "led mid", 7) == 0) {
        light_test_mode = 0; led_set_level(LED_MID);
        strcpy(response, "LED set to mid\n");
    } else if (strncmp(cmd, "led high", 8) == 0) {
        light_test_mode = 0; led_set_level(LED_HIGH);
        strcpy(response, "LED set to high\n");
    } 
    // 2. 부저(음악) 관련 명령
    else if (strncmp(cmd, "music on", 8) == 0) {
        buzzer_on(); strcpy(response, "Music started\n");
    } else if (strncmp(cmd, "music off", 9) == 0) {
        buzzer_off(); strcpy(response, "Music stopped\n");
    } 
    // 3. 조도 센서 관련 명령 (단순 모드 전환 vs 값 읽기)
    else if (strncmp(cmd, "light read", 10) == 0) {
        int val = light_sensor_read();
        snprintf(response, sizeof(response), "Light value: %d\n", val);
    } else if (strncmp(cmd, "light", 5) == 0) {
        light_test_mode = 1; 
        strcpy(response, "Light sensor test mode started\n");
    } 
    // 4. 세그먼트 카운트다운
    else if (strncmp(cmd, "seven ", 6) == 0) {
        int digit = atoi(cmd + 6);
        if (digit >= 0 && digit <= 9) {
            start_countdown(digit);
            snprintf(response, sizeof(response), "Seven segment countdown: %d\n", digit);
        } else {
            strcpy(response, "Invalid digit (0-9 only)\n");
        }
    } else {
        strcpy(response, "Unknown command\n");
    }

    pthread_mutex_unlock(&mutex);
    write(client_sock, response, strlen(response));
}

void signal_handler(int sig) {
    (void)sig;
    is_run = 0;
    led_close();
    buzzer_close();
    light_sensor_close();
    if (server_sock >= 0) {
        shutdown(server_sock, SHUT_RDWR);
        close(server_sock);
    }
    exit(0);
}

int main() {
    // SIGINT 제외 모든 종료 신호 무시
    signal(SIGTERM, SIG_IGN); 
    signal(SIGHUP,  SIG_IGN); 
    signal(SIGTSTP, SIG_IGN); 
    signal(SIGQUIT, SIG_IGN); 

    system("fuser -k -n tcp 8080 9090 > /dev/null 2>&1");
    print_local_ip();
    
    // 3초 대기 후 데몬 전환
    become_daemon(); 

    // 데몬 전환 후 SIGINT 핸들러 재설정
    signal(SIGINT, signal_handler); 

    if (wiringPiSetup() < 0) return 1;
    led_init(LED_PIN); buzzer_init(BUZZER_PIN);
    light_sensor_init(LIGHT_SENSOR_PIN); seven_segment_init();
    led_off();

    pthread_t light_thread;
    pthread_create(&light_thread, NULL, auto_light_control, NULL);
    pthread_create(&web_thread, NULL, web_server_thread, NULL);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in cmd_addr = {0};
    cmd_addr.sin_family = AF_INET;
    cmd_addr.sin_addr.s_addr = INADDR_ANY;
    cmd_addr.sin_port = htons(CMD_PORT);
    bind(server_sock, (struct sockaddr*)&cmd_addr, sizeof(cmd_addr));
    listen(server_sock, 5);

    while (is_run) {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock < 0) continue;
        char buf[256] = {0};
        int len = read(client_sock, buf, sizeof(buf)-1);
        if (len > 0) {
            buf[len] = '\0';
            char *nl = strchr(buf, '\n');
            if (nl) *nl = '\0';
            handle_command(buf, client_sock);
        }
        close(client_sock);
    }
    return 0;
}
