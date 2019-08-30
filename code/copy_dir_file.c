#include "copy_dir_file.h"
#include "myhead.h"

//文件复制任务函数
void *copyfile(void *arg)
{
	
	struct sofile *sop=(struct sofile *)arg;

	int fd1 = open(sop->sfilename, O_RDONLY);	//以只读方式打开
	if(fd1 == -1)
	{
		perror("open file1 failed!");
		exit(0);
	}

	int fd2 = open(sop->ofilename, O_WRONLY|O_CREAT|O_TRUNC , 0777);  //以只写方式打开，如果文件不存在则创建,如果文件被新建，指定其权限为0666
	if(fd2 == -1)
	{
		perror("open file2 failed!");
		exit(0);
	}

	//printf("[%d]\n", __LINE__);
	char buf[100];
	int n, m;
	while(1)
	{
		n = read(fd1, buf, 100);
		if(n == -1)
		{
			perror("read failed");
			exit(0);
		}
		if(n == 0)
		{
			break;
		}

		char *tmp = buf;
		while(n > 0)
		{
			m = write(fd2, tmp, n);
			tmp += m;
			n -= m;
		}
	}

	close(fd1);
	close(fd2);

	free(sop->sfilename);
	free(sop->ofilename);
	free(sop);
}


//目录复制任务函数
void copydir(struct  sodir *arg)
{
	struct  sodir *sod = arg;
	struct  sofile *so  = NULL;

	//如果目标目录不存在，则创建他
	if(access(sod->odirname, F_OK))
	{
		mkdir(sod->odirname,0777);
	}

	// 将相关的绝对路径都要记下来
	char *ori_path = (char *)get_current_dir_name();		//记录原始路径
	//printf("ori_path: %s\n", ori_path);
	
	chdir(sod->sdirname);	//进入源目录
	char *src_path = (char *)get_current_dir_name();	//记录源路径
	//printf("src_path: %s\n", src_path);

	chdir(ori_path);		//返回原始路径
	chdir(sod->odirname);		//进入目标路径
	char *dst_path = (char *)get_current_dir_name();		//记录目标路径
	//printf("dst_path: %s\n", dst_path);
	chdir(ori_path);		//回到原始路径

	DIR *dp = opendir(sod->sdirname);
	if(dp == NULL)
	{
		perror("opendir failed");
		exit(0);
	}

	struct dirent *ep;
	while(1)
	{
		ep = readdir(dp);
		if(ep == NULL)
			break;

		// 跳过..和. 隐藏目录
		if(!strcmp(ep->d_name, ".") || !strcmp(ep->d_name, ".."))
			continue;

		struct stat *info = calloc(1, sizeof(struct stat));
		chdir(src_path);	//进入源路径
		lstat(ep->d_name, info);

		if(S_ISDIR(info->st_mode))		//判断是一个目录
		{
			chdir(src_path);

			char d[255], s[255];
			strncpy(d, dst_path, 100);
			strncpy(s, src_path, 100);
			strcat(d, "/");
			strcat(s, "/");
			strcat(d, ep->d_name);
			strcat(s, ep->d_name);

			struct  sodir *sodd = malloc(sizeof(struct  sodir));
			sodd->sdirname    = malloc(255);
			sodd->odirname    = malloc(255);
			sodd->pool = sod->pool;
			strcpy(sodd->sdirname, s);
			strcpy(sodd->odirname, d);

			//printf("[sdir]%s\n", sodd->sdirname);
			//printf("[odir]%s\n", sodd->odirname);

			copydir(sodd);
		}

		else if(S_ISLNK(info->st_mode))		//判断是链接文件
		{
			char link[1024];
			bzero(link, 1024);

			so = malloc(sizeof(struct sofile));
			so->sfilename  = malloc(1024);
			so->ofilename  = malloc(1024);

			sprintf(so->sfilename, "%s/%s", src_path, ep->d_name);
			sprintf(so->ofilename, "%s/%s", dst_path, ep->d_name);

			readlink(so->sfilename, link, 1024);
			symlink(link, so->ofilename);

			free(so->ofilename);
			free(so->sfilename);
			free(so);

		}



		else //if(S_ISREG(info->st_mode))		//判断是一个普通文件
		{

			so = malloc(sizeof(struct sofile));
			so->sfilename   = malloc(255);
			so->ofilename   = malloc(255);
			so->pool  =  sod->pool;
			sprintf(so->sfilename, "%s/%s", src_path, ep->d_name);
			//printf("[so->sfilename]:%s\n", so->sfilename);

			sprintf(so->ofilename, "%s/%s", dst_path, ep->d_name);
			//printf("[so->ofilename]:%s\n", so->ofilename);
			add_task(copyfile, (void*)so, so->pool);

		}

/*		else
		{
			continue;
		}	*/

		free(info);

	}

	closedir(dp);

	free(sod->sdirname);
	free(sod->odirname);
	free(sod);


}


