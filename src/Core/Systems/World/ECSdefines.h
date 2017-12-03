#pragma once

#include <cstring>
#include <map>
#include <vector>

struct cmp_str { bool operator()(const char *a, const char *b) const { return std::strcmp(a, b) < 0; } };

typedef std::pair<char*, unsigned int> ECShandle;
typedef std::map<char*, std::vector<unsigned int>, cmp_str> ECShandle_map;