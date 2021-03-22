#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>



#define SRV_PORT 9999

void sys_err(const char *str)
{
	perror(str);
	exit(1);
}


int main(int argc,char *argv[])
{
	int lfd,cfd;
	pid_t pid;
	struct sockaddr_in srv_addr,clt_addr;
	socklen_t clt_addr_len;
	char buf[BUFSIZ];
	int ret,i;
	
	bzero(&srv_addr,sizeof(srv_addr));
	
	srv_addr.sin_family=AF_INET;
	srv_addr.sin_port=htons(SRV_PORT);
	srv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	
	lfd=socket(AF_INET,SOCK_STREAM,0);
	if (lfd==-1) sys_err("socket error");

	
	bind(lfd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));
	listen(lfd,128);
	
	clt_addr_len=sizeof(clt_addr);
	
	//write(STDOUT_FILENO,"hello\n",6);
	
	while (1)
	{
		cfd=accept(lfd,(struct sockaddr*)&clt_addr,&clt_addr_len);
		if (cfd==-1) sys_err("accept error");
		
		pid=fork();
		if (pid<0) sys_err("fork error");
		else if (pid==0)
		{
			close(lfd);
			break;
		}
		else
		{
			close(cfd);
			continue;
		}
	}
	
	//write(STDOUT_FILENO,"hello world\n",12);
	if (pid==0)
	{
		for(;;)
		{
			ret=read(cfd,buf,sizeof(buf));
			if (ret==0) 
			{
				close(cfd);
				exit(1);
			}
			for (i=0;i<ret;i++) buf[i]=toupper(buf[i]);
			write(cfd,buf,ret);
			write(STDOUT_FILENO,buf,ret);
		}
	}
	
	return 0;
}
			
			
