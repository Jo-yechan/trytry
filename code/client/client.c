#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#define CMD_PORT 9090

static volatile int running = 1;

// 도움말 메뉴 출력 함수
void print_menu() {
    printf("==========================================================\n");
    printf("            라즈베리 파이 GPIO 제어 클라이언트\n");
    printf("==========================================================\n");
    printf("사용 가능한 명령어:\n\n");
    printf("  led on          - LED 켜기 (기본 밝기)\n");
    printf("  led off         - LED 끄기\n");
    printf("  led low         - LED 약하게\n");
    printf("  led mid         - LED 중간\n");
    printf("  led high        - LED 강하게\n\n");
    printf("  light           - 조도 센서 테스트 모드 시작\n");
    printf("                    (어두우면 LED MID 켜짐, 밝으면 꺼짐)\n");
    printf("  light read      - 현재 조도 값 확인\n\n");
    printf("  music on        - 학교종 멜로디 재생\n");
    printf("  music off       - 멜로디 즉시 멈춤\n\n");
    printf("  seven <0-9>     - 예: seven 5 → 5부터 카운트다운 후 멜로디\n\n");
    printf("  help            - 이 도움말 다시 보기\n");
    printf("  quit 또는 exit  - 클라이언트 종료\n");
    printf("  Ctrl+C          - 강제 종료\n");
    printf("==========================================================\n\n");
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        running = 0;
        printf("\nReceived SIGINT, exiting...\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("사용법: %s <라즈베리파이_IP>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];

    // 신호 처리
    signal(SIGINT, sig_handler);
    signal(SIGTERM, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    // 1. 처음 실행 시 메뉴 출력
    print_menu();
    printf("연결 대상 서버: %s:%d\n", server_ip, CMD_PORT);

    while (running) {
        printf("> ");  // 프롬프트
        fflush(stdout);

        char cmd[256] = {0};
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) break;
        cmd[strcspn(cmd, "\n")] = 0; // 개행 제거

        if (strlen(cmd) == 0) continue;

        // 2. 로컬 명령어 처리 (quit, exit, help)
        if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
            printf("클라이언트 종료. Good bye!\n");
            break;
        }

        if (strcmp(cmd, "help") == 0) {
            print_menu();
            continue;
        }

        // 3. 서버로 명령어 전송 (TCP/IP)
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            printf("소켓 생성 실패\n");
            continue;
        }

        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(CMD_PORT);
        addr.sin_addr.s_addr = inet_addr(server_ip);

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            printf("서버 연결 실패. IP를 확인하거나 서버가 실행 중인지 확인하세요.\n");
            close(sock);
            continue;
        }

        // 명령어 전송
        write(sock, cmd, strlen(cmd));

        // 서버 응답 수신
        char response[512] = {0};
        int len = read(sock, response, sizeof(response) - 1);
        if (len > 0) {
            response[len] = 0;
            printf("%s", response);
        }

        close(sock);
    }

    printf("클라이언트가 종료되었습니다.\n");
    return 0;
}
