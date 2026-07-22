// ============================================================
// KVDB 性能测试 — 模拟应用端高并发多写入少读取
// 直接调用 real_push() / real_get() 接口，不上锁模拟真实调用场景
// ============================================================
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <random>
#include <iomanip>
#include <algorithm>
#ifdef _WIN32
#include <direct.h>
#endif
#include "kvdb.h"

// ---------- 配置参数 ----------
struct Config
{
    int   writerCount    = 8;       // 写入线程数
    int   readerCount    = 2;       // 读取线程数
    int   testSeconds    = 10;      // 测试持续秒数
    int   keyLength      = 8;       // 随机 key 长度
    int   valueLength    = 64;      // 随机 value 长度
    int   readBatchSize  = 5;       // 每次读取尝试几个 key
};

// ---------- 全局状态 ----------
static std::mutex               g_keyMutex;       // 仅保护 key 列表
static std::vector<std::string> g_keys;           // 已写入的 key（供读线程使用）

static std::atomic<long long> g_totalWrites{0};
static std::atomic<long long> g_totalReads{0};
static std::atomic<long long> g_successfulReads{0};
static std::atomic<bool>      g_running{true};

// ---------- 工具函数 ----------

// 生成随机字符串
static std::string randomString(int length)
{
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_int_distribution<int> dist(0, sizeof(charset) - 2);

    std::string s;
    s.reserve(length);
    for (int i = 0; i < length; ++i)
        s += charset[dist(rng)];
    return s;
}

// 时间戳（毫秒）
static long long nowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// ---------- 写入线程 ----------
static void writerWorker(int id, const Config& cfg)
{
    std::mt19937 rng(std::random_device{}() + id * 12345);

    while (g_running.load(std::memory_order_relaxed))
    {
        std::string key   = randomString(cfg.keyLength);
        std::string value = randomString(cfg.valueLength);

        // 直接调用应用层 push 接口
        real_push(key, value);

        // 记录 key 供读线程使用（仅这一处需要锁）
        {
            std::lock_guard<std::mutex> lock(g_keyMutex);
            g_keys.push_back(key);
        }

        g_totalWrites.fetch_add(1, std::memory_order_relaxed);
    }
}

// ---------- 读取线程 ----------
static void readerWorker(int id, const Config& cfg)
{
    std::mt19937 rng(std::random_device{}() + id * 54321);

    while (g_running.load(std::memory_order_relaxed))
    {
        // 从已写入的 key 中随机取一批
        std::vector<std::string> batch;
        {
            std::lock_guard<std::mutex> lock(g_keyMutex);
            if (g_keys.empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            int n = std::min(cfg.readBatchSize, (int)g_keys.size());
            std::sample(g_keys.begin(), g_keys.end(),
                        std::back_inserter(batch), n, rng);
        }

        // 直接调用应用层 get 接口
        for (const auto& key : batch)
        {
            std::string value = real_get(key);
            g_totalReads.fetch_add(1, std::memory_order_relaxed);
            if (!value.empty())
                g_successfulReads.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

// 同时输出到控制台和文件
static void dualOut(std::ostream& os, const std::string& s)
{
    std::cout << s;
    os << s;
}

// ---------- 统计打印线程 ----------
static void reporter(const Config& cfg)
{
    std::ofstream resultFile("BenchResult.txt");
    if (!resultFile.is_open())
    {
        std::cerr << "Error: cannot create BenchResult.txt" << std::endl;
        return;
    }

    long long startMs = nowMs();
    long long lastWrites = 0;
    long long lastReads  = 0;
    long long lastTime   = startMs;

    std::stringstream header;
    header << "\n========== KVDB 性能测试 ==========\n"
           << "  写入线程: " << cfg.writerCount << "\n"
           << "  读取线程: " << cfg.readerCount << "\n"
           << "  测试时长: " << cfg.testSeconds << " 秒\n"
           << "  Key 长度: " << cfg.keyLength << "\n"
           << "  Value长度: " << cfg.valueLength << "\n"
           << "====================================\n\n"
           << std::left
           << std::setw(10) << "时间(s)"
           << std::setw(16) << "写入总数"
           << std::setw(16) << "写入/s"
           << std::setw(16) << "读取总数"
           << std::setw(16) << "读取/s"
           << std::setw(16) << "读取命中率"
           << "\n"
           << std::string(90, '-') << "\n";
    dualOut(resultFile, header.str());

    int elapsed = 0;
    while (elapsed < cfg.testSeconds)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++elapsed;

        long long now      = nowMs();
        long long curWrites = g_totalWrites.load(std::memory_order_relaxed);
        long long curReads  = g_totalReads.load(std::memory_order_relaxed);
        long long curHits   = g_successfulReads.load(std::memory_order_relaxed);

        double writePerSec = (curWrites - lastWrites) * 1000.0 / (now - lastTime);
        double readPerSec  = (curReads  - lastReads)  * 1000.0 / (now - lastTime);
        double hitRate     = (curReads > 0)
                             ? 100.0 * curHits / curReads : 0.0;

        std::stringstream row;
        row << std::left
            << std::setw(10) << elapsed
            << std::setw(16) << curWrites
            << std::setw(16) << std::fixed << std::setprecision(1) << writePerSec
            << std::setw(16) << curReads
            << std::setw(16) << std::fixed << std::setprecision(1) << readPerSec
            << std::setw(16) << std::fixed << std::setprecision(1) << hitRate << "%"
            << "\n";
        dualOut(resultFile, row.str());

        lastWrites = curWrites;
        lastReads  = curReads;
        lastTime   = now;
    }

    g_running.store(false, std::memory_order_relaxed);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // ---------- 最终汇总 ----------
    long long endMs     = nowMs();
    double totalSec     = (endMs - startMs) / 1000.0;
    long long finalWrites = g_totalWrites.load(std::memory_order_relaxed);
    long long finalReads  = g_totalReads.load(std::memory_order_relaxed);
    long long finalHits   = g_successfulReads.load(std::memory_order_relaxed);
    double hitRate       = (finalReads > 0) ? 100.0 * finalHits / finalReads : 0.0;

    std::stringstream summary;
    summary << "\n========== 测试结果 ==========\n"
            << "  运行时间:     " << std::fixed << std::setprecision(1) << totalSec << " s\n"
            << "  总写入数:     " << finalWrites << "\n"
            << "  总读取数:     " << finalReads << "\n"
            << "  读取命中:     " << finalHits << "\n"
            << "  读取命中率:   " << std::fixed << std::setprecision(1) << hitRate << "%\n"
            << "  平均写入/s:   " << std::fixed << std::setprecision(0)
                                 << finalWrites / totalSec << "\n"
            << "  平均读取/s:   " << std::fixed << std::setprecision(0)
                                 << finalReads / totalSec << "\n"
            << "===============================\n";
    dualOut(resultFile, summary.str());
}

// ---------- 主函数 ----------
int main(int argc, char* argv[])
{
    // 确保工作目录是 KVDB 文件夹
    {
        std::string self(argv[0]);
        size_t sep = self.find_last_of("\\/");
        if (sep != std::string::npos)
        {
            std::string dir = self.substr(0, sep);
            // 只在 Windows 上用 _chdir
#ifdef _WIN32
            _chdir(dir.c_str());
#endif
        }
    }

    Config cfg;

    // 简单命令行参数解析
    if (argc >= 2) cfg.writerCount   = std::stoi(argv[1]);
    if (argc >= 3) cfg.readerCount   = std::stoi(argv[2]);
    if (argc >= 4) cfg.testSeconds   = std::stoi(argv[3]);
    if (argc >= 5) cfg.keyLength     = std::stoi(argv[4]);
    if (argc >= 6) cfg.valueLength   = std::stoi(argv[5]);

    // 清理所有 data_* 文件、锁文件、文件表
    for (int n = 0; n < 1000; ++n)
    {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "data_%03d", n);
        std::remove(buf);
        std::string lk = std::string(buf) + ".lock";
        std::remove(lk.c_str());
    }
    // 旧格式 data_N
    for (int i = 0; i < 100; ++i)
    {
        std::string d = "data_" + std::to_string(i);
        std::remove(d.c_str());
        std::remove((d + ".lock").c_str());
    }
    std::remove("table.txt");
    std::ofstream("Log.txt", std::ios::trunc);

    // LSM 引擎初始化（扫描文件表 + WAL 恢复）
    lsm_init();

    // 启动写入线程
    std::vector<std::thread> writers;
    for (int i = 0; i < cfg.writerCount; ++i)
        writers.emplace_back(writerWorker, i, std::ref(cfg));

    // 启动读取线程
    std::vector<std::thread> readers;
    for (int i = 0; i < cfg.readerCount; ++i)
        readers.emplace_back(readerWorker, i, std::ref(cfg));

    // 启动统计线程（阻塞直到测试结束）
    reporter(cfg);

    // 等待所有线程退出
    for (auto& t : writers) t.join();
    for (auto& t : readers) t.join();

    return 0;
}
