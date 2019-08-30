#include "pthread_pool.h"
#include "myhead.h"

//添加任务节点函数
void add_task(void *(*task)(void *), void *arg, struct pthread_pool *pool)
{
	//创建新的节点并初始化
	struct task *new=malloc(sizeof(struct task));
	new->task = task;
	new->arg = arg;
	new->next = NULL;

	//把新节点挂到链表末尾
	pthread_mutex_lock(&(pool->mutex));	

	struct task *newhead = pool->head;

	//原来的任务链表没有节点
	if(pool->head->next == NULL)		
	{
		pool->head->next = new;
	}
	//原来的任务链表有节点
	else			
	{
		while(newhead->next != NULL)
		{
			newhead = newhead->next;
		}
		newhead->next = new;
	}

	(pool->cur_queue_size)++;		//链表节点数加一
	pthread_mutex_unlock(&(pool->mutex));
	pthread_cond_signal(&(pool->cond));
	//printf("add task success!\n");

}


//线程池的初始化函数
bool thread_pool_init(int pthread_num, struct pthread_pool *pool)
{

	//互斥锁初始化
	int ret = pthread_mutex_init(&(pool->mutex),NULL);
	if(ret != 0)
	{
		perror("mutex init failed!");
		return false;
	}
	//条件变量初始化
	ret = pthread_cond_init(&(pool->cond),NULL);
	if(ret != 0)
	{
		perror("cond init failed!");
		return false;
	}

	pool->head = malloc(sizeof(struct task));
	pool->pthread_num = pthread_num;
	pool->cur_queue_size = 0;
	pool->tids = malloc(sizeof(pthread_t) * pthread_num);
	pool->shutdown = false;

	//创建多条线程
	int i;
	for(i=0; i<pthread_num; i++)
	{
		ret = pthread_create(&(pool->tids[i]), NULL, pfunc, (void*)pool);
		if(ret != 0)
		{
			perror("pthread create failed!");
			return false;
		}
	}

	return true;

}


//子线程功能函数
void *pfunc(void *arg)
{
	struct pthread_pool *pool = (struct pthread_pool *)arg;

	while(1)
	{

		pthread_mutex_lock(&(pool->mutex));
		while(pool->cur_queue_size == 0 && !(pool->shutdown))
		{
			//printf("[LINE]:%d\n", __LINE__);
			pthread_cond_wait(&(pool->cond), &(pool->mutex));
		}

		if(pool->cur_queue_size == 0 && pool->shutdown == true)
		{
			//printf("[LINE]:%d\n", __LINE__);
			pthread_mutex_unlock(&(pool->mutex));
			pthread_exit(NULL);
		}

		//取任务链表节点
		struct task *ptask;	
		ptask = pool->head->next;
		pool->head->next = pool->head->next->next;
		pool->cur_queue_size--;
		pthread_mutex_unlock(&(pool->mutex));

		//执行节点对应功能函数
		(*(ptask->task))(ptask->arg);
		//printf("[tid]:%ld\n", (long int)syscall(224));

		//释放抠出来的节点的空间
		free(ptask);
	}

	pthread_exit(NULL);

}


//销毁锁，条件变量，回收资源
void destory_pool(struct pthread_pool *pool)
{
	pool->shutdown = true;
    pthread_cond_broadcast(&(pool->cond));

    int i, error;
    for(i=0; i<pool->pthread_num; i++)
    {
    	error = pthread_join(pool->tids[i], NULL); //等待线程退出
    	if(error != 0)
    	{
    		printf("join tids[%d] error: %s\n", i, strerror(error));
    	}
    	else
    		printf("[task %d][%u] is joined\n", i+1, (unsigned)pool->tids[i]);
    }

	pthread_mutex_destroy(&(pool->mutex));
	pthread_cond_destroy  (&(pool->cond));
	free(pool->tids);
	free(pool->head);
	free(pool);
}





