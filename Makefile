# 源文件和目标文件定义
SRCS_C   := src/globals.c src/encoder_color_line_ilv.c src/decoder_color_line_ilv.c
OBJS_C   := $(patsubst src/%.c, %.o, $(SRCS_C))
SRCS_CPP := src/rgbTileProc.cpp src/main.cpp src/codec.cpp
OBJS_CPP := $(patsubst src/%.cpp, %.o, $(SRCS_CPP))
TARGET   := fblcd.out

# 编译器参数设置
CFLAGS    := -Wall -g

# 编译规则设置
$(TARGET): $(OBJS_C) $(OBJS_CPP)
	g++ $^ -o $@ $(CFLAGS)

%.o: src/%.c
	gcc -c $< -o $@

%.o: src/%.cpp
	g++ -c $< -o $@

rgbTileProc.o: src/rgbTileProc.cpp src/globals.h src/encoder_color_line_ilv.h src/decoder_color_line_ilv.h
## 调试规则设置
.PHONY: debug

debug:
	gdb ./$(TARGET)

## 测试规则设置
.PHONY: test

test: $(TARGET)
	./$(TARGET) -en ./res/sample01.bmp ./res/compress/samole01.jlcd

.PHONY: clean

clean:
	rm -f $(OBJS_C) $(OBJS_CPP) $(TARGET)