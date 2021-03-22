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
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<pthread.h>

#define SERV_PORT 8000
#define MAXLINE 8192
#define OPEN_MAX 5000

void sys_err(const char *str)
{
	perror(str);
	exit(1);
}

int main(int argc,char *argv[])
{
	int i,lfd,cfd,sockfd;
	int n,num=0;
	ssize_t nready,efd,res;
	char buf[MAXLINE],str[INET_ADDRSTRLEN];
	socklen_t clilen;
	
	struct sockaddr_in cliaddr,servaddr;
	struct epoll_event tep,ep[OPEN_MAX];
	
	lfd=socket(AF_INET,SOCK_STREAM,0);
	if (lfd==-1) sys_err("socket error");
	
	int opt=1;
	setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	bzero(&servaddr,sizeof(servaddr));
	
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(SERV_PORT);
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	
	bind(lfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
	listen(lfd,128);
	
	efd=epoll_create(OPEN_MAX);
	if (efd==-1) sys_err("epoll_create error");
	
	tep.events=EPOLLIN;
	tep.data.fd=lfd;
	
	res=epoll_ctl(efd,EPOLL_CTL_ADD,lfd,&tep);
	if (res==-1) sys_err("epoll_ctl error");
	
	for(;;)
	{
		nready=epoll_wait(efd,ep,OPEN_MAX,-1);
		if (nready==-1) sys_err("epoll_wait error");
		
		for (i=0;i<nready;i++)
		{
			if (!(ep[i].events & EPOLLIN)) continue;
			if (ep[i].data.fd==lfd)
			{
				clilen=sizeof(cliaddr);
				cfd=accept(lfd,(struct sockaddr*)&cliaddr,&clilen);
			
				printf("received from %s at port %d\n",
					inet_ntop(AF_INET,&cliaddr.sin_addr,str,sizeof(str)),
					ntohs(cliaddr.sin_port));
				printf("cfd %d---client %d\n",cfd,++num);
			
				tep.events=EPOLLIN;
				tep.data.fd=cfd;
				res=epoll_ctl(efd,EPOLL_CTL_ADD,cfd,&tep);
				if (res==-1) sys_err("eopll_ctl error");
			}
			else
			{
				sockfd=ep[i].data.fd;
				n=read(sockfd,buf,MAXLINE);
				if (n==0)
				{
					res=epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
					if (res==-1) sys_err("eopll_ctl error");
					close(sockfd);
					printf("client[%d] closed connection\n",sockfd);
				}
				else if (n<0)
				{
					perror("read n<0 error:");
					res=epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
					close(sockfd);
				}
				else
				{
					for (i=0;i<n;i++) buf[i]=toupper(buf[i]);
					write(sockfd,buf,n);
					write(STDOUT_FILENO,buf,n);
				}
			}
		}
	}
	close(lfd);
	close(efd);
	return 0;
}
					

