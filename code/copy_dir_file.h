#ifndef		_COPY_DIR_FILE_H_
#define		_COPY_DIR_FILE_H_


#include "myhead.h"
#include "pthread_pool.h"

//文件路径结构体
struct sofile
{
	char *sfilename;
	char *ofilename;
	struct pthread_pool *pool;
};


//目录路径结构体
struct  sodir
{
	char *sdirname;
	char *odirname;
	struct pthread_pool *pool;
};



void *copyfile(void *arg);
void copydir(struct  sodir *arg);



#endif


