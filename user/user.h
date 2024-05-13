#define NULL ((void *) 0)

#define bool _Bool
#define true 1
#define false 0

#define SEEK_SET        0
#define STDIN_FILENO    0
#define FSM_FIRST_FIT   0
#define SEEK_CUR        1
#define STDOUT_FILENO   1
#define FSM_BEST_FIT    1
#define SEEK_END        2
#define STDERR_FILENO   2
#define FSM_WORST_FIT   2

struct stat;

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(const char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void fprintf(int, const char*, ...);
void printf(const char*, ...);
char* gets(char*, int max);
int fgets(int fd, char*, int max);
int getline(char **lineptr, uint *n, int fd);
uint strlen(const char*);
void* memset(void*, int, uint);
int atoi(const char*);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);

// umalloc.c
void malloc_print(void);
void malloc_leaks(void);
void malloc_scribble(void);
void *malloc(uint);
void *calloc(uint, uint);
void *realloc(void*, uint);
void free(void*);
void malloc_name(void*, char*);
void malloc_setfsm(int);
