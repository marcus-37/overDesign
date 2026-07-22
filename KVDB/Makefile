# ============================================================
# KVDB — Makefile
# ============================================================

# --- 跨平台删除命令 ---
ifeq ($(OS),Windows_NT)
RM = del /Q
else
RM = rm -f
endif

# --- 工具链 ---
CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra -pthread

# --- 目标 ---
TARGET    = kvdb
BENCH     = kvdb_benchmark

# --- 默认目标 ---
all: $(TARGET)

# --- 链接（单文件，编译链接一步完成） ---
$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET)

# --- 性能测试（链接 LSM 引擎 + log 工具） ---
$(BENCH): benchmark.cpp LSM.cpp log.cpp kvdb.h
	$(CXX) $(CXXFLAGS) -DKVDB_BENCHMARK -DSIZE=64 benchmark.cpp LSM.cpp log.cpp -o $(BENCH)

bench: $(BENCH)

# --- 清理 ---
clean:
	$(RM) $(TARGET).exe $(TARGET) $(BENCH).exe $(BENCH) lsm_bigtest.exe

# --- 运行 ---
run: $(TARGET)
	./$(TARGET)

# --- 运行性能测试（默认10秒，8写2读） ---
run-bench: $(BENCH)
	./$(BENCH) 8 2 10

# --- 重新构建 ---
rebuild: clean all
