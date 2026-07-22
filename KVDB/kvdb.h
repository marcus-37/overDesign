#ifndef KVDB_H
#define KVDB_H
#include <string>

void lsm_init();

void real_push(const std::string& key, const std::string& value);

std::string real_get(const std::string& key);

void push(const std::string& key, const std::string& value);

std::string get(const std::string& key);

void log(const std::string& msg);

bool isBlankLine(const std::string& line);

std::string trim(const std::string& s);

void WAL();

void clearLog();

#endif // KVDB_H
