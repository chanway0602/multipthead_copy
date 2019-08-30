#include "myhead.h"
#include "pthread_pool.h"
#include "copy_dir_file.h"

 
int main(int argc, char **argv )
{
	struct pthread_pool *pool = malloc(sizeof(struct pthread_pool));
	struct sofile *so;

	struct stat *info = calloc(1, sizeof(struct stat));
	stat(argv[1], info);

	//判断是一个目录，执行复制目录函数
	if(S_ISDIR(info->st_mode))  
	{

		bool flag = thread_pool_init(20, pool);  //初始化线程池
		if(flag == false)
		{
			printf("init thread pool failed!\n");
		}

		struct  sodir *sod = malloc(sizeof(struct  sodir));
		sod->sdirname   = malloc(255);
		sod->odirname   = malloc(255);
		sod->pool           = pool;

		//记录下源目录与目标目录的路径
		sprintf(sod->sdirname,"%s/", argv[1] );
		//printf("[sdir]:%s\n", sod->sdirname);
		sprintf(sod->odirname,"%s/", argv[2] );
		//printf("[odir]:%s\n", sod->odirname);

		copydir(sod);

	}

	//判断是一个文件，执行复制文件函数
	else if(S_ISREG(info->st_mode))		
	{
		//初始化线程池
		bool flag = thread_pool_init(3, pool);  
		if(flag == false)
		{
			printf("init thread pool failed!\n");
		}

		so = malloc(sizeof(struct sofile));
		so->sfilename   = malloc(255);
		so->ofilename   = malloc(255);
		strcpy(so->sfilename, argv[1]);
		strcpy(so->ofilename, argv[2]);
		add_task(copyfile, (void*)so, pool);

	}

	free(info);

	destory_pool(pool);


	return 0;
}



