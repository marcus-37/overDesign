#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <cstddef>
#include <mutex>
#include <thread>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include "kvdb.h"

// ============================================================
// 多级 LSM-Tree 存储引擎
//   所有文件统一 data_xyz 命名:
//     data_{seq:02d}{level:1d}
//     data_000 (level=0,seq=0)   data_001 (level=1,seq=0)
//     data_010 (level=0,seq=1)   data_011 (level=1,seq=1)
//   每层最大文件大小: BASE_MAX_SIZE × 4^level
//   >4 文件时触发 merge 到下层，本层 seq 重置为 0
// ============================================================

#ifndef SIZE
#define SIZE 64
#endif

#define LEVEL0_MAX_FILES 4
#define BASE_MAX_SIZE (1 * 1024 * 1024)

// ---------- 文件表条目 ----------
struct FileEntry
{
    int  level;       // 层级 0,1,2,...
    int  seq;         // 层级内序号
    long size;        // 字节数
    std::string path;
};

// ---------- 全局状态 ----------
static std::mutex g_cacheMtx;
static std::vector<std::pair<std::string, std::string>> g_cache;
static size_t g_cacheBytes = 0;

static std::mutex g_tableMtx;
static std::vector<FileEntry> g_files;
static int g_level0Seq = -1;

// ============================================================
//  工具: 文件名 ↔ level/seq
// ============================================================

static std::string makePath(int level, int seq)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "data_%02d%d", seq, level);
    return buf;
}

static bool parsePath(const std::string& path, int& level, int& seq)
{
    if (path.compare(0, 5, "data_") != 0) return false;
    std::string r = path.substr(5);
    if (r.size() != 3) return false;
    seq   = std::atoi(r.substr(0, 2).c_str());
    level = r[2] - '0';
    return true;
}

static std::string lockPath(const std::string& dp) { return dp + ".lock"; }

static long maxSizeForLevel(int level)
{
    long s = BASE_MAX_SIZE;
    for (int i = 0; i < level; ++i) s *= 4;
    return s;
}

// ============================================================
//  文件锁
// ============================================================

static bool tryLock(const std::string& path)
{
    std::string lp = lockPath(path);
    std::ifstream chk(lp);
    if (chk.is_open()) { chk.close(); return false; }
    std::ofstream f(lp);
    return f.is_open();
}

static void unlockFile(const std::string& path)
{ std::remove(lockPath(path).c_str()); }

static void lockWait(const std::string& path)
{
    while (!tryLock(path))
        std::this_thread::sleep_for(std::chrono::microseconds(100));
}

// ============================================================
//  文件表 (table.txt) 格式: seq:size:level  (x:y:z)
// ============================================================

static void saveTable()
{
    std::lock_guard<std::mutex> lock(g_tableMtx);
    std::ofstream tbl("table.txt");
    if (!tbl.is_open()) return;
    for (auto& e : g_files)
        tbl << e.seq << ":" << e.size << ":" << e.level << "\n";
}

static void loadTable()
{
    std::lock_guard<std::mutex> lock(g_tableMtx);
    g_files.clear();
    g_level0Seq = -1;
    std::ifstream tbl("table.txt");
    if (!tbl.is_open()) return;
    std::string line;
    while (std::getline(tbl, line))
    {
        if (line.empty()) continue;
        int sq, lv; long sz;
        if (std::sscanf(line.c_str(), "%d:%ld:%d", &sq, &sz, &lv) != 3) continue;
        FileEntry e{lv, sq, sz, makePath(lv, sq)};
        g_files.push_back(e);
        if (lv == 0 && sq > g_level0Seq) g_level0Seq = sq;
    }
}

static long getFileSize(const std::string& path)
{
    std::ifstream f(path, std::ios::ate | std::ios::binary);
    return f.is_open() ? (long)f.tellg() : 0;
}

static void removeFileEntry(const std::string& path)
{
    std::lock_guard<std::mutex> lock(g_tableMtx);
    for (auto it = g_files.begin(); it != g_files.end(); ++it)
        if (it->path == path) { g_files.erase(it); break; }
}

static void upsertFileEntry(int level, int seq, long size)
{
    std::lock_guard<std::mutex> lock(g_tableMtx);
    for (auto& e : g_files)
        if (e.level == level && e.seq == seq) { e.size = size; return; }
    g_files.push_back({level, seq, size, makePath(level, seq)});
    if (level == 0 && seq > g_level0Seq) g_level0Seq = seq;
}

// ============================================================
//  读取一个 data 文件的所有 KV
// ============================================================

struct KvEntry
{
    std::string key, value;
    int srcLevel, srcSeq, position;
};

static std::vector<KvEntry> readAllEntries(const std::string& path)
{
    std::vector<KvEntry> res;
    std::ifstream f(path);
    if (!f.is_open()) return res;
    std::string k, v;
    int pos = 0, lv, sq;
    parsePath(path, lv, sq);
    while (std::getline(f, k))
    {
        if (k.empty()) continue;
        if (!std::getline(f, v)) break;
        if (!k.empty() && k.back() == '\r') k.pop_back();
        if (!v.empty() && v.back() == '\r') v.pop_back();
        res.push_back({k, v, lv, sq, pos++});
    }
    return res;
}

// ============================================================
//  WAL 恢复
// ============================================================

static void walRecover()
{
    WAL();
}

// ============================================================
//  mergeLevel — 将 srcLevel 合并到 srcLevel+1
//  每层最多 LEVEL0_MAX_FILES 个文件，超限触发递归合并
// ============================================================

static void mergeLevel(int srcLevel)
{
    int tgtLevel = srcLevel + 1;

    // 1. 收集 srcLevel 和 tgtLevel 的全部 KV
    std::vector<KvEntry> all;
    std::vector<std::string> oldPaths;
    {
        std::lock_guard<std::mutex> lock(g_tableMtx);
        for (auto& fe : g_files)
        {
            if (fe.level == srcLevel || fe.level == tgtLevel)
            {
                auto e = readAllEntries(fe.path);
                all.insert(all.end(), e.begin(), e.end());
                oldPaths.push_back(fe.path);
            }
        }
    }
    if (all.empty()) return;

    // 2. 去重: level 最小 → seq 最大 → position 最大
    std::map<std::string, KvEntry> dedup;
    for (auto& e : all)
    {
        auto it = dedup.find(e.key);
        if (it == dedup.end()) { dedup[e.key] = e; continue; }
        auto& o = it->second;
        bool newer = false;
        if (e.srcLevel < o.srcLevel) newer = true;
        else if (e.srcLevel == o.srcLevel)
        {
            if (e.srcSeq > o.srcSeq) newer = true;
            else if (e.srcSeq == o.srcSeq && e.position > o.position) newer = true;
        }
        if (newer) it->second = e;
    }

    // 3. 写入 tgtLevel
    std::vector<std::pair<std::string, std::string>> sorted;
    for (auto& kv : dedup) sorted.push_back({kv.first, kv.second.value});

    int tgtSeq = 0;
    long maxSz = maxSizeForLevel(tgtLevel);

    // 找 tgtLevel 已有最大 seq，优先追加未满文件
    {
        std::lock_guard<std::mutex> lock(g_tableMtx);
        int bestSeq = -1;
        for (auto& fe : g_files)
            if (fe.level == tgtLevel)
            {
                if (fe.seq >= tgtSeq) tgtSeq = fe.seq + 1;
                if (fe.size < maxSz && fe.seq > bestSeq) bestSeq = fe.seq;
            }
        if (bestSeq >= 0) tgtSeq = bestSeq;
    }

    auto it = sorted.begin();
    while (it != sorted.end())
    {
        std::string fp = makePath(tgtLevel, tgtSeq);
        long curSz = getFileSize(fp);

        if (curSz >= maxSz)
        {
            tgtSeq++;
            if (tgtSeq >= 100) { tgtLevel++; tgtSeq = 0; maxSz = maxSizeForLevel(tgtLevel); }
            continue;
        }

        lockWait(fp);
        std::ofstream out(fp, std::ios::app);
        while (it != sorted.end())
        {
            size_t rb = it->first.size() + it->second.size() + 2;
            if (curSz + (long)rb > maxSz) break;
            out << it->first << "\n" << it->second << "\n";
            curSz += (long)rb;
            ++it;
        }
        out.close();
        unlockFile(fp);
        upsertFileEntry(tgtLevel, tgtSeq, getFileSize(fp));
        saveTable();
    }

    // 4. 删除旧文件（srcLevel + 旧的 tgtLevel）
    for (auto& p : oldPaths)
    {
        // 跳过刚写入的 tgtLevel 文件
        bool isNew = false;
        for (int s = 0; s <= tgtSeq; ++s)
            if (p == makePath(tgtLevel, s)) { isNew = true; break; }
        if (isNew) continue;

        std::remove(p.c_str());
        std::remove(lockPath(p).c_str());
        removeFileEntry(p);
    }
    saveTable();

    // 5. 重置 srcLevel seq
    if (srcLevel == 0) g_level0Seq = -1;

    // 6. 递归检查 tgtLevel 是否也超限
    int tgtCnt = 0;
    {
        std::lock_guard<std::mutex> lock(g_tableMtx);
        for (auto& fe : g_files)
            if (fe.level == tgtLevel) tgtCnt++;
    }
    if (tgtCnt > LEVEL0_MAX_FILES)
        mergeLevel(tgtLevel);
}

// ============================================================
//  lsm_init
// ============================================================

void lsm_init()
{
    loadTable();

    if (g_level0Seq < 0)
    {
        for (int lv = 0; lv <= 9; ++lv)
            for (int sq = 0; sq < 100; ++sq)
            {
                std::string p = makePath(lv, sq);
                std::ifstream f(p);
                if (!f.is_open()) break;
                long sz = 0; f.seekg(0, std::ios::end); sz = f.tellg();
                g_files.push_back({lv, sq, sz, p});
                if (lv == 0 && sq > g_level0Seq) g_level0Seq = sq;
            }
        if (g_level0Seq >= 0 || !g_files.empty()) saveTable();
    }

    for (auto& e : g_files)
    {
        long actual = getFileSize(e.path);
        if (actual != e.size) e.size = actual;
    }
    if (!g_files.empty()) saveTable();
    if (g_level0Seq < 0) g_level0Seq = 0;

    walRecover();
}

// ============================================================
//  sync — 缓存刷入 level 0 + 触发 merge 检查
// ============================================================

static void sync_locked()
{
    if (g_cache.empty()) return;

    int seq = g_level0Seq < 0 ? 0 : g_level0Seq;
    std::string fpath = makePath(0, seq);
    long curSz = getFileSize(fpath);

    lockWait(fpath);
    std::ofstream out(fpath, std::ios::app);
    if (!out.is_open())
    {
        unlockFile(fpath);
        seq++; fpath = makePath(0, seq);
        lockWait(fpath);
        out.open(fpath, std::ios::app);
        curSz = 0;
    }

    for (auto& kv : g_cache)
    {
        size_t rb = kv.first.size() + kv.second.size() + 2;
        if (curSz + (long)rb > BASE_MAX_SIZE)
        {
            out.close(); unlockFile(fpath);
            upsertFileEntry(0, seq, getFileSize(fpath));
            seq++; fpath = makePath(0, seq);
            lockWait(fpath);
            out.open(fpath, std::ios::app);
            curSz = 0;
        }
        out << kv.first << "\n" << kv.second << "\n";
        curSz += (long)rb;
    }
    out.close(); unlockFile(fpath);
    upsertFileEntry(0, seq, getFileSize(fpath));
    if (seq > g_level0Seq) g_level0Seq = seq;
    saveTable();
    g_cache.clear();
    g_cacheBytes = 0;
    clearLog();

    // 触发 merge 检查（level 0 > 4 则合并到 level 1，会递归）
    int cnt0 = 0;
    {
        std::lock_guard<std::mutex> lock(g_tableMtx);
        for (auto& fe : g_files) if (fe.level == 0) cnt0++;
    }
    if (cnt0 > LEVEL0_MAX_FILES) mergeLevel(0);
}

static void sync()
{
    std::lock_guard<std::mutex> lock(g_cacheMtx);
    sync_locked();
}

// ============================================================
//  push
// ============================================================

void push(const std::string& key, const std::string& value)
{
    size_t kvBytes = key.size() + value.size();
    {
        std::lock_guard<std::mutex> lock(g_cacheMtx);
        g_cache.push_back({key, value});
        g_cacheBytes += kvBytes;
        if (g_cacheBytes >= SIZE * 1024) { sync_locked(); return; }
    }
}

// ============================================================
//  get — 先查 level 小(0)的，同 level 内 seq 从大到小
// ============================================================

std::string get(const std::string& key)
{
    // 1. 内存缓存
    {
        std::lock_guard<std::mutex> lock(g_cacheMtx);
        for (auto it = g_cache.rbegin(); it != g_cache.rend(); ++it)
            if (it->first == key) return it->second;
    }

    // 2. 按 level 0→1→2→...、同 level seq 降序、文件内底部→顶部
    std::lock_guard<std::mutex> lock(g_tableMtx);
    for (int lv = 0; lv <= 9; ++lv)
    {
        std::vector<FileEntry> lf;
        for (auto& fe : g_files)
            if (fe.level == lv) lf.push_back(fe);
        std::sort(lf.begin(), lf.end(),
                  [](const FileEntry& a, const FileEntry& b) { return a.seq > b.seq; });
        for (auto& fe : lf)
        {
            auto entries = readAllEntries(fe.path);
            for (auto it = entries.rbegin(); it != entries.rend(); ++it)
                if (it->key == key) return it->value;
        }
    }
    return "";
}


// ============================================================
//  real_push / real_get
// ============================================================

void real_push(const std::string& key, const std::string& value)
{
    log("PUSH key=\"" + key + "\" value=\"" + value + " \"");
    push(key, value);
}

std::string real_get(const std::string& key)
{
    std::string res = get(key);
    if (res != "")
    {
        log("GET key=\"" + key + "\" -> found");
        return res;
    }
    log("GET key=\"" + key + "\" -> not found");
    return "";
}


