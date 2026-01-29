CC = gcc
CFLAGS = -Wall -Wextra -pthread -fPIC
LIBS = -lwiringPi

# 절대 경로로 확실하게 고정
BASE_DIR = $(abspath $(shell pwd)/../../exec)
LIB_DIR  = $(BASE_DIR)/lib
SRV_DIR  = $(BASE_DIR)/server

# 타겟 파일 리스트
LIB_TARGETS = $(LIB_DIR)/libled.so $(LIB_DIR)/libbuzzer.so $(LIB_DIR)/libseven.so $(LIB_DIR)/liblight.so
SERVER_TARGET = $(SRV_DIR)/server

# 기본 타겟
all: create_dirs $(LIB_TARGETS) $(SERVER_TARGET)

# 디렉토리 생성 (반드시 필요)
create_dirs:
	mkdir -p $(LIB_DIR) $(SRV_DIR)

# 각 공유 라이브러리 빌드 (패턴 대신 명시적 규칙 사용)
$(LIB_DIR)/libled.so: led.c led.h
	$(CC) $(CFLAGS) -shared -o $@ led.c $(LIBS)

$(LIB_DIR)/libbuzzer.so: buzzer.c buzzer.h
	$(CC) $(CFLAGS) -shared -o $@ buzzer.c $(LIBS)

$(LIB_DIR)/libseven.so: seven.c seven.h
	$(CC) $(CFLAGS) -shared -o $@ seven.c $(LIBS)

$(LIB_DIR)/liblight.so: light.c light.h
	$(CC) $(CFLAGS) -shared -o $@ light.c $(LIBS)

# 서버 빌드
$(SERVER_TARGET): server.c led.c buzzer.c seven.c light.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# 정리 (현재 디렉토리의 .o와 .so, 그리고 exec 디렉토리 삭제)
clean:
	rm -f *.o *.so
	rm -rf $(BASE_DIR)
	@echo "정리 완료"

run: $(SERVER_TARGET)
	sudo $(SERVER_TARGET)

.PHONY: all clean run create_dirs
