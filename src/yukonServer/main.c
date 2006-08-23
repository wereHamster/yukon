
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE

#define _GNU_SOURCE
#define _BSD_SOURCE

#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


static char *home;
static char date[32];

static void buildDate(void)
{
	time_t tim = time(NULL);
	struct tm *now = localtime(&tim);
	snprintf(date, 32, "%d-%02d-%02d--%02d:%02d:%02d", now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
}

static int openOutput()
{
	char path[4096];
	snprintf(path, 4096, "%s/yukonOutput/%s.yukon", home, date);
	return open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH);
}

int main(int argc, char *argv[])
{	
	if (argc == 1) {
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		return 0;
	}
	
	home = getenv("HOME");
	int port = atoi(argv[1]);
		
	int fdSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(fdSocket, &addr, sizeof(addr)) != 0) {
		perror("bind()");
		return 0;
	}
	
	if (listen(fdSocket, 1) != 0) {
		perror("bind()");
		return 0;
	}
	
	while(1) {
		socklen_t len = sizeof(addr);
		
		int fdInput = accept(fdSocket, &addr, &len);
		if (fdInput < 0) {
			perror("accept()");
			break;
		}
		
		buildDate();
		int fdOutput = openOutput();
		if (fdOutput < 0) {
			perror("openOutput()");
			break;
		}
		
		printf("[%s] => %s:%d\n", date, inet_ntoa(addr.sin_addr), addr.sin_port);
		
		size_t size = 4096;
		void *buffer[4096];
		
		ssize_t ret = 0;
		while (ret = read(fdInput, buffer, size)) {
			if (ret > 0) {
				write(fdOutput, buffer, ret);
			}
		}
		
		buildDate();
		printf("[%s] => end\n", date);
		
		close(fdOutput);
		close(fdInput);
	}
	
	close(fdSocket);
	
	return 0;
}
