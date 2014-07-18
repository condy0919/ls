#ifndef _LS_INCLUDED_H_
#define _LS_INCLUDED_H_

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/ioctl.h>

#include <grp.h>
#include <pwd.h>

static inline int get_screen_col()
{
    int col;
#ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    col = ts.ts_cols;
#elif defined TIOCGWINSZ
    struct winsize ts;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
    col = ts.ws_col;
#endif
    return col;
}

struct ls_attr_t {
    unsigned int all: 1;
    unsigned int dir: 1;
    unsigned int inode: 1;
    unsigned int long_format: 1;
    unsigned int reverse: 1;
    unsigned int recursive: 1;
    unsigned int sort_by_size: 1;
    unsigned int one_column: 1;
    unsigned int no_sort: 1;
    unsigned int l_without_owner: 1;
    unsigned int l_without_group: 1;
    unsigned int ignore_backups: 1;
};

inline bool is_dir_file(const std::string&);
inline bool is_reg_file(const std::string&);
inline bool is_lnk_file(const std::string&);

inline bool size_cmp(const std::string&, const std::string&);

void list_all_files(const std::vector<std::string>&, const ls_attr_t&);
void walk_dir(const std::string&, const ls_attr_t&);
void pretty_print(const std::vector<std::string>&, const ls_attr_t&);

void display_usage();

#endif
