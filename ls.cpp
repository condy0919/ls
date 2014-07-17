#include "ls.hpp"

bool (*cmp)(const std::string&, const std::string&);

int main(int argc, char* argv[])
{
    ls_attr_t attr = {0};

    // parse the parameters
    int ch;
    while ((ch = getopt(argc, argv, "1aBdfgGhilrRS")) != -1) {
        switch (ch) {
        case 'a': // print all files
            attr.all = 1;
            break;
        case 'B': // ignore backups
            attr.ignore_backups = 1;
            break;
        case 'd': // print the directory's info
            attr.dir = 1;
            break;
        case 'f': // no sort
            attr.no_sort = 1;
            break;
        case 'g': // long format without owner
            attr.l_without_owner = 1;
            break;
        case 'G': // long format without group
            attr.l_without_group = 1;
            break;
        case 'i': // print the inode number
            attr.inode = 1;
            break;
        case 'l': // detail info
            attr.long_format = 1;
            break;
        case 'r': // reverse output
            attr.reverse = 1;
            break;
        case 'R': // recursive traverse
            attr.recursive = 1;
            break;
        case 'S': // sort by size
            attr.sort_by_size = 1;
            break;
        case '1': // one column
            attr.one_column = 1;
            break;
        case 'h': // usage
            display_usage();
            std::exit(0);
            break;
        }
    }

    // default: alpha order
    cmp = attr.sort_by_size ? size_cmp : [](const std::string& a, const std::string& b) { return a < b; };

    std::vector<std::string> files;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-')
            files.push_back(argv[i]);
    }
    if (!attr.no_sort)
        std::sort(files.begin(), files.end(), cmp);
    if (attr.reverse)
        std::reverse(files.begin(), files.end());

    list_all_files(files, attr);
    return 0;
}

void list_all_files(const std::vector<std::string>& files, const ls_attr_t& attr)
{
    struct stat buf;
    std::vector<std::string> collector;
    for (int i = 0; i < files.size(); ++i) {
        if (stat(files[i].c_str(), &buf) == -1) {
            std::printf("list_all_files\n");
            std::perror("stat");
            std::exit(1);
        }
        if (files.size() != 1 && is_dir_file(files[i]) && !attr.dir)
            std::cout << files[i] << ":\n";

        if (S_ISDIR(buf.st_mode) && !attr.dir)
            walk_dir(files[i], attr);
        else if (is_dir_file(files[i]) || is_reg_file(files[i]))
            collector.push_back(files[i]);
    }
    if (collector.size() > 0)
        pretty_print(collector, attr);

    // default (./)
    if (files.size() == 0 && !attr.dir)
        walk_dir(".", attr);
    else if (files.size() == 0)
        pretty_print(std::vector<std::string>(1, "."), attr);
}

void walk_dir(const std::string& path_name, const ls_attr_t& attr)
{
    DIR* dir;
    struct dirent* entry;
    char old_entry[BUFSIZ];

    getcwd(old_entry, BUFSIZ);

    if ((dir = opendir(path_name.c_str())) == NULL) {
        std::printf("walk_dir\n");
        std::perror("opendir");
        std::exit(1);
    }

    chdir(path_name.c_str());

    std::vector<std::string> collector;
    while ((entry = readdir(dir)) != NULL) {
        if (!attr.all && entry->d_name[0] == '.')
            continue;
        if (attr.ignore_backups && entry->d_name[0] == '~')
            continue;
        if (is_dir_file(entry->d_name) && (std::string(".") == entry->d_name || std::string("..") == entry->d_name))
            collector.push_back(entry->d_name);
        else if (is_reg_file(entry->d_name))
            collector.push_back(entry->d_name);
        else
            collector.push_back(entry->d_name);
    }
    if (!attr.no_sort)
        std::sort(collector.begin(), collector.end(), cmp);
    if (attr.reverse)
        std::reverse(collector.begin(), collector.end());

    if (attr.recursive)
        std::cout << std::string(old_entry) + "/" + path_name + ":\n";
    pretty_print(collector, attr);
    
    // recursive
    if (attr.recursive) {
        for (int i = 0; i < collector.size(); ++i) {
            if (!is_dir_file(collector[i]) || collector[i] == "." || collector[i] == "..")
                continue;
            std::puts("");
            walk_dir(collector[i], attr);
        }
    }

    chdir(old_entry);
    closedir(dir);
}

void pretty_print(const std::vector<std::string>& files, const ls_attr_t& attr)
{
    struct predicate {
        static bool P(const std::vector< std::vector<int> >& max_between, int size, int col) {
            int s = 0, i;
            for (i = 0; i + size - 1 < max_between.size(); i += size)
                s += max_between[i][i + size - 1] + 2;
            if (i < max_between.size())
                s += max_between[i][max_between.size() - 1] + 1;
            return s <= col;
        }
    };

    if (attr.long_format) {
        std::size_t width[4] = {0}, total_size = 0;
        struct stat buf;
        for (int i = 0; i < files.size(); ++i) {
            if (stat(files[i].c_str(), &buf) == -1) {
                std::printf("pretty_print");
                std::perror("stat");
                std::exit(1);
            }
            total_size += buf.st_blocks;
            width[0] = std::max(width[0], static_cast<std::size_t>(std::log10(buf.st_nlink) + 1));
            width[1] = std::max(width[1], std::strlen(getpwuid(buf.st_uid)->pw_name));
            width[2] = std::max(width[2], std::strlen(getgrgid(buf.st_gid)->gr_name));
            width[3] = std::max(width[3], static_cast<std::size_t>(std::log10(buf.st_size) + 1));
        }
        std::cout << "total " << total_size / 2 << std::endl;
        for (int i = 0; i < files.size(); ++i) {
            if (stat(files[i].c_str(), &buf) == -1) {
                std::printf("%s\t", files[i].c_str());
                std::printf("pretty_print\n");
                std::perror("stat");
                std::exit(1);
            }
            if (attr.inode)
                std::printf("%-8lu", buf.st_ino);

            // print file's mode
            if (S_ISLNK(buf.st_mode))
                std::putchar('l');
            else if (S_ISREG(buf.st_mode))
                std::putchar('-');
            else if (S_ISDIR(buf.st_mode))
                std::putchar('d');
            else if (S_ISCHR(buf.st_mode))
                std::putchar('c');
            else if (S_ISBLK(buf.st_mode))
                std::putchar('b');
            else if (S_ISFIFO(buf.st_mode))
                std::putchar('f');
            else
                std::putchar('?');

            // print user's permission
            std::putchar(buf.st_mode & S_IRUSR ? 'r' : '-');
            std::putchar(buf.st_mode & S_IWUSR ? 'w' : '-');
            std::putchar(buf.st_mode & S_IXUSR ? 'x' : '-');

            // print group's permission
            std::putchar(buf.st_mode & S_IRGRP ? 'r' : '-');
            std::putchar(buf.st_mode & S_IWGRP ? 'w' : '-');
            std::putchar(buf.st_mode & S_IXGRP ? 'x' : '-');

            // print other's permission
            std::putchar(buf.st_mode & S_IROTH ? 'r' : '-');
            std::putchar(buf.st_mode & S_IWOTH ? 'w' : '-');
            std::putchar(buf.st_mode & S_IXOTH ? 'x' : '-');

            std::putchar(' ');
            // print owner and group
            std::printf("%*lu ", width[0], buf.st_nlink);
            if (!attr.l_without_owner)
                std::printf("%*s ", width[1], getpwuid(buf.st_uid)->pw_name);
            if (!attr.l_without_group)
                std::printf("%*s ", width[2], getgrgid(buf.st_gid)->gr_name);
            if (buf.st_size != 0)
                std::printf("%*ld", width[3], buf.st_size);
            else
                std::printf("%ld", buf.st_size);

            // time
            std::string time = std::string(ctime(&buf.st_mtime));
            time.pop_back();
            std::cout << " " << time << " ";

            // file name
            std::cout << files[i] << std::endl;
        }
        return;
    }

    std::vector< std::vector<int> > max_between(files.size());
    for (int i = 0; i < max_between.size(); ++i)
        max_between[i].resize(files.size());

    for (int i = 0; i < files.size(); ++i)
        max_between[i][i] = files[i].length();
    for (int len = 1; len < files.size(); ++len) {
        for (int i = 0; i + len < files.size(); ++i) {
            int j = i + len;
            max_between[i][j] = std::max(static_cast<int>(std::max(files[i].length(), files[j].length())),
                                         max_between[i + 1][j - 1]);
        }
    }

    int size;
    for (size = 1; size < files.size(); ++size)
        if (predicate::P(max_between, size, get_screen_col()))
            break;
    if (attr.one_column)
        size = files.size();
    for (int i = 0; i < size; ++i) {
        for (int j = i, c = 0; j < files.size(); j += size, ++c) {
            int extra_width = (j + size < files.size()) ? 2 : 1;
            int end = std::min(static_cast<std::size_t>(c * size + size - 1), files.size() - 1);
            std::printf("%*s", -(max_between[c * size][end] + extra_width), files[j].c_str());
        }
        std::puts("");
    }
}


void display_usage()
{
    std::printf("Usage：ls [options]... [file]...\n"
                "List information about the FILEs (the current directory by default).\n"
                "\n"
                "Options:\n"
                "-a         do not ignore entries starting with .\n"
                "-d         list directory entries instead of contents,\n"
                "             and do not dereference symbolic links\n"
                "-h         display this help and exit\n"
                "-i         print the index number of each file\n"
                "-l         use a long listing format\n"
                "-r         reverse order while sorting\n"
                "-R         list subdirectories recursively\n"
                "-S         sort by file size\n"
                "-1         list one file per line\n"
                "-B         do not list implied entries ending with ~\n"
                "-f         do not sort, enable -aU, disable -ls --color\n"
                "-g         like -l, but do not list owner\n"
                "-G         in a long listing, don't print group names\n");
}

bool is_reg_file(const std::string& name)
{
    struct stat buf;
    if (stat(name.c_str(), &buf) == -1) {
        std::printf("is_reg_file\n");
        std::perror("stat");
        std::exit(1);
    }
    return S_ISREG(buf.st_mode);
}

bool is_dir_file(const std::string& name)
{
    struct stat buf;
    if (stat(name.c_str(), &buf) == -1) {
        std::printf("is_dir_file\n");
        std::printf("%s\n", name.c_str());
        std::perror("stat");
        std::exit(1);
    }
    return S_ISDIR(buf.st_mode);
}

bool size_cmp(const std::string& file1, const std::string& file2)
{
    struct stat buf1, buf2;
    if (stat(file1.c_str(), &buf1) == -1) {
        std::printf("size_cmp\n");
        std::perror("stat");
        std::exit(1);
    }
    if (stat(file2.c_str(), &buf2) == -1) {
        std::printf("size_cmp");
        std::perror("stat");
        std::exit(1);
    }
    return buf1.st_size < buf2.st_size;
}
