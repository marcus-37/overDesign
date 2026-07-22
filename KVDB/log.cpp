#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <mutex>
#include "kvdb.h"

std::mutex logMutex;
// 判断一行是否只包含空白字符（用于格式校验）
bool isBlankLine(const std::string& line)
{
    for (char c : line)
        if (c != ' ' && c != '\t' && c != '\r')
            return false;
    return true;
}

// 去除字符串首尾空白
std::string trim(const std::string& s)
{
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r'))
        ++start;
    size_t end = s.size();
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r'))
        --end;
    return s.substr(start, end - start);
}

// 获取当前时间字符串
static std::string currentTime()
{
    std::time_t t = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H:%M:%S", std::localtime(&t));
    return buf;
}

// 写入日志
void log(const std::string& msg)
{
    logMutex.lock();
    std::ofstream logFile("Log.txt", std::ios::app);
    if (logFile.is_open())
    {
        logFile << "[" << currentTime() << "] " << msg << " Txend" << std::endl;
    }
    logMutex.unlock();
}

// 清空日志文件
void clearLog()
{
    logMutex.lock();
    std::ofstream("Log.txt", std::ios::trunc).close();
    logMutex.unlock();
}

// 计算日志文件行数
static int countLogLines()
{
    std::ifstream file("Log.txt");
    std::string line;
    int count = 0;
    while (std::getline(file, line))
        if (!line.empty()) ++count;
    return count;
}

void push(const std::string& key, const std::string& value);
std::string get(const std::string& key);

void WAL()
{
    std::ifstream logFile("Log.txt");
    std::string line;

    while (std::getline(logFile, line))   // 逐行读取直到结束
    {
        if (line.empty()) continue;

        // 检查是否为 PUSH 操作
        if (line.find(" PUSH ") == std::string::npos)
            continue;

        // 检查末尾是否有 Txend
        size_t txendPos = line.rfind("Txend");
        if (txendPos == std::string::npos)
        {
            // 没有 Txend → 不完整的事务，清空日志后退出
            clearLog();
            return;
        }

        // 解析 key="..."
        size_t ks = line.find("key=\"");
        if (ks == std::string::npos) continue;
        ks += 5;
        size_t ke = line.find("\"", ks);
        if (ke == std::string::npos) continue;

        // 解析 value="..."
        size_t vs = line.find("value=\"", ke);
        if (vs == std::string::npos) continue;
        vs += 7;
        // 从 Txend 往前找闭合引号
        size_t ve = line.rfind("\"", txendPos - 1);
        if (ve == std::string::npos || ve <= vs) continue;
        // 去除 value 末尾的空格
        while (ve > vs && line[ve - 1] == ' ') --ve;

        std::string key   = line.substr(ks, ke - ks);
        std::string value = line.substr(vs, ve - vs);

        push(key, value);
    }
    clearLog();
}
