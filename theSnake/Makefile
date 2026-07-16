# ============================================================
# theSnake — Makefile
# ============================================================

# --- 跨平台删除命令 ---
ifeq ($(OS),Windows_NT)
RM = del /Q
else
RM = rm -f
endif

# --- 工具链 ---
CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra -I./../SFML-3.1.0/include
LDFLAGS   = -L./../SFML-3.1.0/lib
LDLIBS    = -lsfml-graphics -lsfml-window -lsfml-system

# --- 目标 ---
TARGET    = theSnake
OBJS      = main.o food.o snake.o states.o

# --- 默认目标 ---
all: $(TARGET)

# --- 链接 ---
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

# --- 编译规则 ---
main.o: main.cpp main.h food.h snake.h render.h states.h
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

food.o: food.cpp food.h main.h
	$(CXX) $(CXXFLAGS) -c food.cpp -o food.o

snake.o: snake.cpp snake.h main.h
	$(CXX) $(CXXFLAGS) -c snake.cpp -o snake.o

states.o: states.cpp states.h food.h snake.h main.h
	$(CXX) $(CXXFLAGS) -c states.cpp -o states.o

# --- 清理 ---
clean:
	$(RM) *.o $(TARGET)*

# --- 运行 ---
run: $(TARGET)
	./$(TARGET)

# --- 重新构建 ---
rebuild: clean all

