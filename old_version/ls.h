#ifndef _LS_INCLUDED
#define _LS_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <math.h>

#include <grp.h>
#include <pwd.h>

#define swap(a, b, type)	\
	do {\
		type temp;\
		temp = (a);\
		(a) = (b);\
		(b) = temp;\
	} while (0)


struct ls_attr_t {
	unsigned int all:1;
	unsigned int dir:1;
	unsigned int inode:1;
	unsigned int long_format:1;
	unsigned int reverse:1;
	unsigned int recursive:1;
	unsigned int sort_by_size:1;
};



// store file_names in a directory
char* file_name_list[BUFSIZ];
int list_size = 0;

/*
 *	IMPLEMETATION
 */
static inline void insert_string(char* x)
{
	if (list_size > BUFSIZ) {
		printf("list full\n");
		exit(1);
	}

	file_name_list[list_size++] = x;
}

static inline void remove_string()
{
	if (list_size <= 0) {
		printf("list empty");
		exit(1);
	}

	--list_size;
}

static inline int is_list_empty()
{
	return list_size == 0;
}


/*
 *	INTERFACE
 */
int is_dir_file(char* name);
int is_reg_file(char* name);

// for pretty display
int max(int len[], int st, int ed);
int sum(int a[], int n);
void encode_display(char* file_name_list[], int n, int col_line_max[], int per_size, struct ls_attr_t* p);
void print_files(struct ls_attr_t* p);
//void print_file_name_format(char* name, int limit);
void print_file_detail_info(char* name, int limit[], int n, struct ls_attr_t* p);


// for sorting the file list
int size_cmp_for_file(const char* file1, const char* file2);

//void output_permission(unsigned int permission);


// some tips
int get_screen_col();
void display_usage();

// record all files sorted
void list_all_files(char* str_array[], int n, struct ls_attr_t* p);
void walk_dir(char* path_name, void (*record_file)(char*, struct ls_attr_t*), struct ls_attr_t* p);
//void print_dir(char* path_name, void (*record_file)(char*, struct ls_attr_t*), struct ls_attr_t* p);
void record_file(char* path_name, struct ls_attr_t* p);

//void show_file(char* path_name, void (*walk_dir)(char*, void*, struct ls_attr_t*), struct ls_attr_t* p);

// set zero
void init_attribute(struct ls_attr_t* p);

#endif

