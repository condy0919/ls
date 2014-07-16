#include "ls.h"

/*
 *	default: sort by name
 */

int (*cmp)(const char*, const char*);

int main(int argc, char* argv[])
{
	struct ls_attr_t attr;
	init_attribute(&attr);

	int ch;
	// parse the parameters
	while ((ch = getopt(argc, argv, "adilrRSh")) != -1)
		switch (ch) {
		case 'a':// print all files
			attr.all = 1;
			break;
		case 'd':// print the directory's info
			attr.dir = 1;
			break;
		case 'i':// print the inode number
			attr.inode = 1;
			break;
		case 'l':// detail info
			attr.long_format = 1;
			break;
		case 'r':// reverse output
			attr.reverse = 1;
			break;
		case 'R':// recursive traverse
			attr.recursive = 1;
			break;
		case 'S':// sort by size
			attr.sort_by_size = 1;
			break;
		case 'h':// usage
			display_usage();
			exit(0);
			break;
		}

	if (attr.sort_by_size)
		cmp = &size_cmp_for_file;
	else
		cmp = &strcmp;

	char* str_array[BUFSIZ] = { 0 };
	int array_size = 0;
	// parse the filename and sort the file by character
	for (int i = 1; i < argc; ++i)
		if (argv[i][0] != '-') {
			int j;
			++array_size;

			if (!attr.reverse)
				for (j = array_size - 1; j > 0 && cmp(argv[i], str_array[j - 1]) > 0; --j)
					str_array[j] = str_array[j - 1];
			else
				for (j = array_size - 1; j > 0 && cmp(argv[i], str_array[j - 1]) < 0; --j)
					str_array[j] = str_array[j - 1];

			str_array[j] = argv[i];
		}

	list_all_files(str_array, array_size, &attr);

	return 0;
}


void list_all_files(char* str_array[], int n, struct ls_attr_t* p)
{
	int i;
	struct stat buf;

	for (i = 0; i < n; ++i) {
		if (stat(str_array[i], &buf) == -1) {
			printf("list_all_files\n");
			perror("stat");
			exit(1);
		}
		if (n != 1 && is_dir_file(str_array[i]) && !p->dir)
			printf("%s:\n", str_array[i]);

		if (S_ISDIR(buf.st_mode) && !p->dir)
			walk_dir(str_array[i], record_file, p);
		else if (S_ISDIR(buf.st_mode))
			record_file(str_array[i], p);
		else if (S_ISREG(buf.st_mode))
			record_file(str_array[i], p);
	}
	
	// default(./)
	if (n == 0 && !p->dir) {
		walk_dir(".", record_file, p);
		return;
	} else if (n == 0)
		record_file(".", p);
	
	if (list_size == 0)
		return;

	// print the remaining regular file
	// sort the file names by alpha order
	for (int i = 0; i < list_size; ++i) {
		for (int j = 0; j < list_size - i - 1; ++j) {
			if (p->reverse) {
				if (cmp(file_name_list[j], file_name_list[j + 1]) > 0)
					swap(file_name_list[j], file_name_list[j + 1], char*);
			} else {
				if (cmp(file_name_list[j], file_name_list[j + 1]) < 0)
					swap(file_name_list[j], file_name_list[j + 1], char*);
			}
		}
	}

	// print the filename sorted
	print_files(p);
	list_size = 0;
}

void record_file(char* path_name, struct ls_attr_t* p)
{
	insert_string(path_name);
}

void walk_dir(char* path_name, void (*record_file)(char*, struct ls_attr_t*), struct ls_attr_t* p)
{
	DIR* dir;
	struct dirent* entry;
	char old_entry[BUFSIZ];

	getcwd(old_entry, BUFSIZ);


	if ((dir = opendir(path_name)) == NULL) {
		printf("walk_dir\n");
		perror("opendir");
		exit(1);
	}

	chdir(path_name);

	while ((entry = readdir(dir)) != NULL) {
		if (!p->all && entry->d_name[0] == '.')
			continue;
		else if (is_dir_file(entry->d_name) && (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0))
			record_file(entry->d_name, p);
			//continue;
		else if (is_reg_file(entry->d_name))
			record_file(entry->d_name, p);
		else if (p->recursive)
			//walk_dir(entry->d_name, record_file, p);
			record_file(entry->d_name, p);
		else
			record_file(entry->d_name, p);
	}
	// output all the file sorted
	
	// sort the file names by alpha order
	for (int i = 0; i < list_size; ++i) {
		for (int j = 0; j < list_size - i - 1; ++j) {
			if (!p->reverse) {
				if (cmp(file_name_list[j], file_name_list[j + 1]) > 0)
					swap(file_name_list[j], file_name_list[j + 1], char*);
			} else {
				if (cmp(file_name_list[j], file_name_list[j + 1]) < 0)
					swap(file_name_list[j], file_name_list[j + 1], char*);
			}
		}
	}
	if (p->recursive)
		printf("%s:\n", path_name);
	print_files(p);
	list_size = 0;

	chdir(old_entry);
	closedir(dir);
}


/*
 *	get screen's col
 */
int get_screen_col()
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


void display_usage()
{
	printf("Usage：ls [options]... [file]...\n"
			"List information about the FILEs (the current directory by default).\n"
			"\n"
			"Options:\n"
			"-a			do not ignore entries starting with .\n"
			"-d			list directory entries instead of contents,\n"
			"			  and do not dereference symbolic links\n"
			"-h			display this help and exit\n"
			"-i			print the index number of each file\n"
			"-l			use a long listing format\n"
			"-r			reverse order while sorting\n"
			"-R			list subdirectories recursively\n"
			"-S			sort by file size\n"
		  );
}

void init_attribute(struct ls_attr_t* p)
{
	memset(p, 0, sizeof(struct ls_attr_t));
}

int size_cmp_for_file(const char* file1, const char* file2)
{
	struct stat buf1, buf2;

	if (stat(file1, &buf1) == -1) {
		printf("size_cmp_for_file\n");
		perror("stat");
		exit(1);
	}

	if (stat(file2, &buf2) == -1) {
		printf("size_cmp_for_file\n");
		perror("stat");
		exit(1);
	}

	return buf2.st_size - buf1.st_size;
}

int is_reg_file(char* name)
{
	struct stat buf;
	
	if (stat(name, &buf) == -1) {
		printf("is_reg_file\n");
		perror("stat");
		exit(1);
	}
	if (S_ISREG(buf.st_mode))
		return 1;
	return 0;
}

int is_dir_file(char* name)
{
	struct stat buf;
	
	if (stat(name, &buf) == -1) {
		printf("is_dir_file\n");
		printf("%s\n", name);
		perror("stat");
		exit(1);
	}
	if (S_ISDIR(buf.st_mode))
		return 1;
	return 0;

}

/*
 *	print the buf content
 */
void print_files(struct ls_attr_t* p)
{
	int* col_line_max = malloc(sizeof(int) * list_size);
	int* len = malloc(sizeof(int) * list_size);
	int i, k;
	int size;
	int v, s;
	int LIMIT = get_screen_col();
	struct stat buf;

	// initialize the col_line_max
	for (int i = 0; i < list_size; ++i)
		len[i] = strlen(file_name_list[i]);

	for (size = 1; size < list_size; ++size) {
		for (i = 0, k = 0; i < list_size - size; i += size, ++k) {
			v = max(len, i, i + size) + 2;// +2 for pretty print
			col_line_max[k] = v;
		}
		v = max(len, i, list_size);
		col_line_max[k++] = v;
		s = sum(col_line_max, k);
		if (s <= LIMIT)
			break;
	}
	// calculate the format lenght done
	
	encode_display(file_name_list, list_size, col_line_max, size, p);

	free(col_line_max);
	free(len);
}


void encode_display(char* file_name_list[], int n, int col_line_max[], int per_size, struct ls_attr_t* p)
{
	int i, j, k;
	char* dir_file_name[BUFSIZ] = { NULL };
	int dir_file_name_list_size = 0;
	struct stat buf;
	memset(&buf, 0, sizeof(struct stat));

	if (n > 0 && stat(file_name_list[0], &buf) == -1) {
		printf("encode_display\n");
		perror("stat");
		exit(1);
	}

	if (p->long_format) {
		int limit[4];

		// determine the edge
		limit[0] = log10(buf.st_nlink) + 1;
		limit[1] = strlen(getpwuid(buf.st_uid)->pw_name);
		limit[2] = strlen(getgrgid(buf.st_gid)->gr_name);
		limit[3] = log10(buf.st_size) + 1;
		for (i = 0; i < n; ++i) {
			stat(file_name_list[i], &buf);
			if (limit[0] < log10(buf.st_nlink) + 1)
				limit[0] = log10(buf.st_nlink) + 1;
			if (limit[1] < strlen(getpwuid(buf.st_uid)->pw_name))
				limit[1] = strlen(getpwuid(buf.st_uid)->pw_name);
			if (limit[2] < strlen(getgrgid(buf.st_gid)->gr_name))
				limit[2] = strlen(getgrgid(buf.st_gid)->gr_name);
			if (limit[3] < log10(buf.st_size) + 1)
				limit[3] = log10(buf.st_size) + 1;
		}
		
		// process the directory
		for (j = 0; j < n; ++j) {
			print_file_detail_info(file_name_list[j], limit, 4, p);
			if (is_dir_file(file_name_list[j]) && !p->dir)
				dir_file_name[dir_file_name_list_size++] = file_name_list[j];
		}
	} else {
		for (i = 0; i < per_size; ++i, putchar('\n'))
			for (j = i, k = 0; j < n; ++k, j += per_size) {
				int max_line = col_line_max[k];
				int len = strlen(file_name_list[j]);
				if (p->inode)
					printf("%-8lu ", buf.st_ino);
				printf("%s%*c", file_name_list[j], max_line - len, ' ');

				if (is_dir_file(file_name_list[j]) && !p->dir)
					dir_file_name[dir_file_name_list_size++] = file_name_list[j];
			}
	}
	if (!p->recursive)
		return;
	list_size = 0;
	putchar('\n');
	for (i = 0; i < dir_file_name_list_size; ++i)
		if (strcmp(dir_file_name[i], ".") != 0 && strcmp(dir_file_name[i], "..") != 0)
			walk_dir(dir_file_name[i], record_file, p);
	list_size = 0;
}


int max(int len[], int st, int ed)
{
	int i;
	int max_t = len[st];

	for (i = st + 1; i < ed; ++i)
		if (max_t < len[i])
			max_t = len[i];

	return max_t;
}

int sum(int a[], int n)
{
	int sum = 0;
	int i;

	for (i = 0; i < n; ++i)
		sum += a[i];
	return sum;
}

void print_file_detail_info(char* name, int limit[], int n, struct ls_attr_t* p)
{
	char time_buf[32];
	struct stat buf;
	struct passwd* psd;
	struct group* grp;

	if (stat(name, &buf) == -1) {
		printf("%s\t", name);
		printf("print_file_detail_info\n");
		perror("stat");
		exit(1);
	}

	if (p->inode)
		printf("%-8lu ",buf.st_ino); 

	// print file's mode
	if (S_ISLNK(buf.st_mode))
		putchar('l');
	else if (S_ISREG(buf.st_mode))
		putchar('-');
	else if (S_ISDIR(buf.st_mode))
		putchar('d');
	else if (S_ISCHR(buf.st_mode))
		putchar('c');
	else if (S_ISBLK(buf.st_mode))
		putchar('b');
	else if (S_ISFIFO(buf.st_mode))
		putchar('f');
	else
		putchar('?');
	//else if (S_ISSOCK(buf.st_mode))
	//	putchar('s');
	//	// not supported by C99


	// print user's permisson
	if (buf.st_mode & S_IRUSR)
		putchar('r');
	else
		putchar('-');
	if (buf.st_mode & S_IWUSR)
		putchar('w');
	else
		putchar('-');
	if (buf.st_mode & S_IXUSR)
		putchar('x');
	else
		putchar('-');

	// print group's permisson
	if (buf.st_mode & S_IRGRP)
		putchar('r');
	else
		putchar('-');
	if (buf.st_mode & S_IWGRP)
		putchar('w');
	else
		putchar('-');
	if (buf.st_mode & S_IXGRP)
		putchar('x');
	else
		putchar('-');

	// print other's permisson
	if (buf.st_mode & S_IROTH)
		putchar('r');
	else
		putchar('-');
	if (buf.st_mode & S_IWOTH)
		putchar('w');
	else
		putchar('-');
	if (buf.st_mode & S_IXOTH)
		putchar('x');
	else
		putchar('-');

	putchar(' ');

	// print onwer and group
	psd = getpwuid(buf.st_uid);
	grp = getgrgid(buf.st_gid);
	printf("%*lu ", limit[0], buf.st_nlink);
	printf("%*s ", limit[1], psd->pw_name);
	printf("%*s ", limit[2], grp->gr_name);

	if (buf.st_size != 0)
		printf("%*ld", limit[3], buf.st_size);
	else
		printf("%ld", buf.st_size);

	strcpy(time_buf, ctime(&buf.st_mtime));
	time_buf[strlen(time_buf) - 1] = '\0';
	printf(" %s", time_buf);

	// print file name
	printf(" %s\n", name);
}

