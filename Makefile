# Compiler and compilation options
# 编译器和编译选项
CC = g++
CFLAGS = -Wall -Werror -std=c++17 -I.
LIBS = -lcurl

# Target file name
# 目标文件名
TARGET = bing-wallpaper-macos

# Check processor architecture
# 检查处理器架构
ifeq ($(shell uname -p), arm)
	CFLAGS += -I/opt/homebrew/include/
else
	CFLAGS += -I/usr/local/include/
endif

# Source file list	
# 源文件列表
SRCS = bing-wallpaper-macos.cpp bing.cpp wallpaper.cpp

# Generate corresponding object file list for each source file
# 根据源文件生成对应的目标文件列表
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# Compile each source file into corresponding object file
# 编译每个源文件为对应的目标文件
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
