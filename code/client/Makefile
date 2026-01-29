CC = gcc
CFLAGS = -Wall -Wextra -pthread
LIBS = -lwiringPi

# 경로 설정
BASE_DIR = $(shell pwd)/../../exec
CLI_DIR  = $(BASE_DIR)/client

TARGET = $(CLI_DIR)/client
SRCS = client.c
OBJS = $(SRCS:.c=.o)

all: create_dirs $(TARGET)

# 디렉토리 자동 생성
create_dirs:
	mkdir -p $(CLI_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	$(TARGET)

.PHONY: all clean run create_dirs
