#include<stdio.h>
#include<string.h>
#include<syslog.h>
#include<errno.h>
#include<stdlib.h>



int main(int argc, char *argv[])
{

	const char *filename = argv[1];
	const char *data=argv[2];
	
	
	openlog("syslog",LOG_PID | LOG_CONS | LOG_NDELAY , LOG_USER);
	
	// check for 2 arguments
	
	if(argc != 3){
		fprintf (stderr,"insufficient arguments");
		syslog(LOG_ERR,"Parameter not provided\n");
		syslog(LOG_ERR,"ERROR");
		closelog();
		exit(1);
		}
		
	
	
	FILE *fp = fopen(filename,"w");
	
	
	if(fp == NULL){
		fprintf(stderr,"Error %s:%s\n",filename,strerror(errno));
		syslog(LOG_ERR,"file not created:%m");
		closelog();
		exit(1);
		}
	
	syslog(LOG_DEBUG,"writing %s to %s\n", data,filename);
	fprintf(stderr,"writing %s to %s\n",data,filename);
	size_t nr = fwrite(data,strlen(data),1,fp);
	if (nr!=sizeof(*data)) {
		syslog(LOG_ERR,"Write failed:%m");
		closelog();
		fprintf(stderr,"fwrite failed: %zu\n",nr);
		exit(1);
	}
	
	
	
	exit(0);
	
	
 
	return 0;
}
