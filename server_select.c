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

#define SERV_PORT 6666

void sys_err(const char *str)
{
	perror(str);
	exit(1);
}

int main(int argc,char *argv[])
{
	int i,j,n,nready;
	int maxfd=0;
	int listenfd,connfd;
	char buf[BUFSIZ];
	
	struct sockaddr_in clie_addr,serv_addr;
	socklen_t clie_addr_len;
	
	listenfd=socket(AF_INET,SOCK_STREAM,0);
	if (listenfd==-1) sys_err("socket error");
	
	int opt=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	bzero(&serv_addr,sizeof(serv_addr));
	
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	
	bind(listenfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
	listen(listenfd,128);
	
	fd_set rset,allset;
	maxfd=listenfd;
	
	FD_ZERO(&allset);
	FD_SET(listenfd,&allset);
	
	while (1)
	{
		rset=allset;
		nready=select(maxfd+1,&rset,NULL,NULL,NULL);
		if (nready<0) sys_err("select errot");
		
		if (FD_ISSET(listenfd,&rset))
		{
			clie_addr_len=sizeof(clie_addr);
			connfd=accept(listenfd,(struct sockaddr*)&clie_addr,&clie_addr_len);
			if (connfd<0) sys_err("connect error");
			
			FD_SET(connfd,&allset);
			if (maxfd<connfd) maxfd=connfd;
			if (0==--nready) continue;
		}
		
		for (i=listenfd+1;i<=maxfd+1;i++)
		{
			if (FD_ISSET(i,&rset))
			{
				n=read(i,buf,sizeof(buf));
				if (n==0)
				{
					close(i);
					FD_CLR(i,&allset);
				}
				else if (n>0)
				{
					for (j=0;j<n;j++) buf[j]=toupper(buf[j]);
					write(i,buf,n);
					write(STDOUT_FILENO,buf,n);
				}
			}
		}
	}
	close(listenfd);
	return 0;
}
					
	
	
