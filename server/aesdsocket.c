#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/queue.h>
#include <stdint.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>
#include <sys/select.h>

 
 #define DATA_PATH	"/var/tmp/aesdsocketdata"
 #define DATA_SERVICE	"9000"
 

char *program; 
int datafd;
int sockfd;

//linked list structures
struct entry {
           pthread_t thread;
           SLIST_ENTRY(entry) entries;            
       };


struct thread_head thread_h;
SLIST_HEAD(thread_head,entry);

volatile sig_atomic_t sig_recieved;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

struct sockaddr_storage addr;
socklen_t addrlen = sizeof(addr);
struct sockaddr_in *addrin = (void *)&addr;

void signal_handler(int signal_number){
	
	syslog(LOG_DEBUG," Caught signal, exiting");
	closelog();
	remove("/var/tmp/aesdsocketdata");
	close(sockfd); 
	close(datafd);
	exit(0);
	}
	

void append(int fd, void *buf, ssize_t n){
	
 	pthread_mutex_lock(&mutex_lock);
		if (lseek(datafd, 0, SEEK_END) < 0){
			perror("lseek error\n");
			return;
		}
		if(write(datafd,buf,n)!=(ssize_t)strlen(buf)){
			perror("write error\n"); 
			return;
		}	
		pthread_mutex_unlock(&mutex_lock);
 }


void *timestamp_thread(void *arg){
	time_t ltime;
 	struct tm *tmp;
 	char stime[1024];
 	ssize_t n;
 	
 	//int file_fd = open("/var/tmp/aesdsocketdata", O_WRONLY | O_APPEND | O_CREAT, 0644);
	//if(file_fd == -1){
		//perror("open\n");
		//pthread_exit(NULL);
	//}
	
 
 	while (!sig_recieved) {
 		sleep(10);
 		if (sig_recieved)
 			break;
 		
 		ltime = time(NULL);
 		tmp = localtime(&ltime);
 		n = strftime(stime, sizeof(stime), "timestamp:%a, %d %b %Y %T %z\n", tmp);
 		append(datafd,stime,n);
 		 	}
 	//close(file_fd);
 	//pthread_exit(NULL);
 	return NULL;
}

void *client(void *arg){
	int acceptfd = (uintptr_t)arg;
 	char buf[1024];
 	ssize_t n;
 
 	syslog(LOG_INFO, "Accepted connection from %s", inet_ntoa(addrin->sin_addr));
 
 	// Accumulate packets until a newline is received:
 	while (!sig_recieved) {
 		n = recv(acceptfd, buf, sizeof(buf), 0);
 		if (n < 0)
 			perror("recv\n");
 		append(datafd,buf,n);
 		if (memchr(buf, '\n', sizeof(buf)) != NULL)
 			break;
 	}
 
 	// Send accumulated packets to client:
 	pthread_mutex_lock(&mutex_lock);
 	if (lseek(datafd, 0, SEEK_SET) < 0)
		perror("lseek_client\n");
		
 	while (!sig_recieved) {
 		n = read(datafd, buf, sizeof(buf));
 		if (n < 0)
 			perror("read \n");
 		if (n == 0)
 			break; 
 
 		if (send(acceptfd, buf, n, 0) < 0)
 			perror("send\n");
 	}
 	pthread_mutex_unlock(&mutex_lock);
 
 	close(acceptfd);
 	syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(addrin->sin_addr));
 	return NULL;
 }
 
 
 int connect_service(const char *service)
 {
 	int rv, sockfd;
 	struct addrinfo hints, *res;
 
 	memset(&hints, 0, sizeof(hints));
 	hints.ai_family = AF_UNSPEC;
 	hints.ai_socktype = SOCK_STREAM;
 	hints.ai_flags = AI_PASSIVE;
 
 	rv = getaddrinfo(NULL, service, &hints, &res);
 	if (rv != 0){
 		syslog(LOG_ERR, "getaddrinfo error: %s\n", gai_strerror(rv));
 		exit(-1);
	}
 
 	for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
 		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
 		if (sockfd < 0){
			syslog(LOG_ERR, "socket: %s\n", strerror(errno));
			exit(-1);
		}
 			
 
 		const int *optval = &(int){1};
 		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, optval, sizeof(*optval))){
 			perror("setsockopt");
 			exit(-1);
		}
 
 		if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0)
 			syslog(LOG_ERR, "bind: %s\n", strerror(errno));
 
 		freeaddrinfo(res);
 		return sockfd;
 	}
 	return -1;
 }
 
 
 int main(int argc, char *argv[])
 {
	int rl, opt;
 	bool daemonize = false;
	struct entry *n1, *n2;
 	
 
 	program = argv[0];
 	while ((opt = getopt(argc, argv, "d")) != -1) {
 		switch (opt) {
 		case 'd':
 			daemonize = true;
 			break;
 		}
 	}
	SLIST_INIT(&thread_h);
 	openlog(NULL, 0, LOG_USER);
 
 	datafd = open(DATA_PATH, O_CREAT | O_RDWR, 0644);
 	if (datafd < 0)
		perror("open failed\n");
	
 	sockfd = connect_service(DATA_SERVICE);
 	if (sockfd < 0)
 		perror("Unable to bind\n");
 
 	if (daemonize)
 	if (daemon(0, 0) < 0)
 		perror("daemon");
 
	n1 = malloc(sizeof( struct entry));
	if(!n1){
		perror("cannot malloc for entry\n");
		}
		
	int t = pthread_create(&n1->thread,NULL,timestamp_thread,NULL);
	if (t != 0){
		perror("pthread_create failed for timestamp thread");
		syslog(LOG_ERR,"thread create failed for timestamp thread : %s\n",strerror(errno));
		close(sockfd);
		exit(-1);
		}
	SLIST_INSERT_HEAD(&thread_h,n1, entries);
	
 
 	signal(SIGINT,signal_handler);
 	signal(SIGTERM,signal_handler);
 	
 	rl = listen(sockfd,8);
 	if(rl == -1){
		perror("listen failed\n");
		syslog(LOG_ERR,"error listening on socket: %s\n",strerror(errno));
		close(sockfd);
		exit(-1);
	}
		
 	for(;;) {
 		int acceptfd = accept(sockfd,(void *)&addr, &addrlen);
 		if (acceptfd < 0){
			syslog(LOG_ERR,"Error accepting connection: %s",strerror(errno));
			continue;
		}
		n2 = malloc(sizeof(struct entry));
		if(!n2){
			perror("malloc\n");
		}
		
 		int ct = pthread_create(&n2->thread,NULL,client,(void *)(uintptr_t)acceptfd);
 		if (ct!=0){
			syslog(LOG_ERR,"unable to create client thread:%s\n",strerror(errno));
			SLIST_INSERT_HEAD(&thread_h,n2,entries);
		}
 	}
 	//sig_recieved = true;
	SLIST_FOREACH(n2, &thread_h, entries){
		pthread_join(n2->thread, NULL);
		SLIST_REMOVE(&thread_h,n2,entry,entries);
		free(n2);
		}
	exit(0);
 }
