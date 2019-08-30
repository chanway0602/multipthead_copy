#ifndef		_PTHREAD_POOLLIB_H_
#define		_PTHREAD_POOLLIB_H_


#include "myhead.h"
#include "copy_dir_file.h"


//线程池结构体
struct pthread_pool
{
	pthread_mutex_t mutex;
	pthread_cond_t   cond;
	pthread_t            *tids;   //malloc(sizeof(pthread_t)*n)
	
	struct  task  *head;
	int  pthread_num;
	int  cur_queue_size;

	bool shutdown;

};

//任务节点结构体
struct task
{
	void  *(*task)(void *arg);
	void  *arg;
	struct  task *next;

};



bool thread_pool_init(int pthread_num, struct pthread_pool *pool);
void add_task(void *(*task)(void *), void *arg, struct pthread_pool *pool);
void *pfunc(void *arg);
void destory_pool(struct pthread_pool *pool);






#endif


