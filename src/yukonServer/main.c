
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
#include <pthread.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

static char *date(void)
{
	static char sbuf[32];
	time_t tim = time(NULL);
	struct tm *now = localtime(&tim);
	snprintf(sbuf, 32, "%d-%02d-%02d--%02d:%02d:%02d", now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
	return sbuf;
}

static int output(char *name)
{
	char path[4096];
	snprintf(path, 4096, "%s/yukonOutput/%s.yukon", getenv("HOME"), name);
	return open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH);
}

static pthread_mutex_t mutex;
static struct thread {
	pthread_t thread;
	int socket;
} threads[16];

static void *loop(void *arg)
{
	struct thread *data = arg;
	
	pthread_mutex_lock(&mutex);
	char *now = date();
	printf("[%s]: >>> %d\n", now, data - threads);
	int file = output(now);
	pthread_mutex_unlock(&mutex);
	
	if (file < 0) {
		perror("open");
		goto out;
	}
	
	size_t bsize = 4096;
	char buffer[4096];
	
	for (;;) {
		int rb = read(data->socket, buffer, bsize);
		if (rb == 0) {
			break;
		} else if (rb < 0) {
			if (errno == EINTR)
				continue;
			perror("read");
			goto out;
		}

		char *p = buffer;
		do {
			int wb = write(file, p, rb);
			if (wb == 0) {
				printf("filesystem full!\n");
				exit(0);
			} else if (wb < 0) {
				if (errno == EINTR)
					continue;
				perror("write");
				goto out;
			}
			p += wb;
			rb -= wb;
		} while (rb);
	}
	
out:
	close(data->socket);
	close(file);
	
	pthread_mutex_lock(&mutex);
	data->socket = -1;
	now = date();
	printf("[%s]: <<< %d\n", now, data - threads);
	pthread_mutex_unlock(&mutex);
	
	return NULL;
}


static void spawn(int socket)
{
	pthread_mutex_lock(&mutex);
	
	for (int i = 0; i < 16; ++i) {
		if (threads[i].socket < 0) {
			threads[i].socket = socket;
			pthread_create(&threads[i].thread, NULL, loop, (void *) &threads[i]);
			goto out;
		}
	}

out:
	pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[])
{	
	if (argc == 1) {
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		return 0;
	}
	
	pthread_mutex_init(&mutex, NULL);
	
	for (int i = 0; i < 16; ++i) {
		threads[i].socket = -1;
	}
	
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
	
	for (;;) {
		socklen_t len = sizeof(addr);
		
		int client = accept(fdSocket, &addr, &len);
		if (client < 0) {
			perror("accept()");
			continue;
		}
		
		spawn(client);
	}
	
	close(fdSocket);
	
	return 0;
}
