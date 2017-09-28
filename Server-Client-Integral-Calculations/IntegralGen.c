#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <matheval.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#define MAXD 256
#define FIFO_FILE_NAME "communicate"
#define BACK_FIFO_FILE_NAME "respondComm"
typedef struct 
{
	pid_t  pidid;
	double timeInterval;
	char operation;
	char f1[MAXD];
	char f2[MAXD];
}Client;
typedef struct 
{
	pid_t  pidid;
	double timeInterval;
	char operation;
	char f1[MAXD];
	char f2[MAXD];
	int fifo1;
	int fifo2;
	char *mem;

}ClientNew;
ClientNew gloClient;
pid_t clientsPid[MAXD];
int clientsPidCounter=0;

void catchSigInt(int signo) {
	fprintf(stderr,"myExit\n");
	FILE *logP;
	char logFile[MAXD];
	int i;
	sprintf(logFile, "%d", gloClient.pidid);
	strcat(logFile, ".txt");
	logP = fopen(logFile, "a");

	fprintf(logP, "CTRL+C handled.\nServer died.\nProgram has been terminated!\n");

	for (i = 0; i < clientsPidCounter; ++i){
		if(clientsPid[i]==gloClient.pidid){
			kill(clientsPid[i], SIGKILL);
			close(gloClient.fifo1);
			close(gloClient.fifo2);
			unlink(FIFO_FILE_NAME);
			free(gloClient.mem);
			exit(EXIT_SUCCESS);
		}
	}
	for (i = 0; i < clientsPidCounter; ++i){
		kill(clientsPid[i], SIGKILL);
	}
	close(gloClient.fifo1);
	close(gloClient.fifo2);
	unlink(FIFO_FILE_NAME);
	free(gloClient.mem);
	exit(EXIT_SUCCESS);
}
void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}
double calculateFunction(void *function, double t0, double tn, int h){

    int i, n;
    double y[100],se=0,ans,x[100];
    n=(tn-t0)/h;
    if(n%2==1)
    {
        n=n+1;
    }
    if(n==0){
    	fprintf(stderr, "dividing by zero ERROR\n" );
    	exit(EXIT_FAILURE);
    }

    h=(tn-t0)/n;
    for(i=0; i<n; ++i){
    	x[i]=t0+i*h;
    	y[i]=evaluator_evaluate_x(function, x[i]);
    }
    for(i=1; i<n-1; ++i){
    	se+=y[i];
    }
    ans = (h / 2.0)*(y[0] + y[n] + 2*se);
    return ans;
}
int main(int argc, char **argv){
	Client newC;
	char *fi;
	void *eval;
	int length;
    time_t epoch_time;
    struct tm *tm_p;
    double milisecond;
	double answer=0;
	double lastTI;
	int i=0;
	int fd, fd2;
	char fileName[MAXD];
	pid_t childpid;

    epoch_time = time( NULL );
    tm_p = localtime( &epoch_time );

	if(argc!=3){
		fprintf(stderr, "Usage: %s MainFifoName\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int resolution = atoi(argv[1]);
	int maxClients = atoi(argv[2]);

	signal(SIGINT, catchSigInt);


	for(i=0 ; i<maxClients ; ++i){
		childpid=fork();
		if(childpid==0)
			break;
	}
	signal(SIGINT, catchSigInt);
	wait(NULL);
	if(childpid==0){
		if((mkfifo(FIFO_FILE_NAME, 0666)==-1) && (errno != EEXIST) ){
			fprintf(stderr, "FIFO CAN NOT CREATE!\n" );
			exit(EXIT_FAILURE);
		}
		clientsPid[clientsPidCounter]=(long)getpid();
		++clientsPidCounter;
		fd=open(FIFO_FILE_NAME, O_RDONLY);
		read(fd, &(newC.pidid), sizeof(pid_t));
		read(fd, &(newC.timeInterval), sizeof(double));
		read(fd, &(newC.operation), sizeof(char));
		read(fd, newC.f1, sizeof(char)*MAXD);
		read(fd, newC.f2, sizeof(char)*MAXD);
		fileName[0] = '\0';
		sprintf(fileName, "%d", newC.pidid);
		strcat(fileName, "mkfifo");
		fd2=open(fileName, O_WRONLY);
	
	
		milisecond = 100.0*((tm_p->tm_min)*60.0 + tm_p->tm_sec);
	
		if((fi=malloc(strlen(newC.f1)+strlen(newC.f2)+6))!=NULL){
			fi[0]='\0';
			strcat(fi, "(");
			strcat(fi, newC.f1);
			strcat(fi, ")");
			length=strlen(fi);
			fi[length]=newC.operation;
			fi[length+1]='\0';
			strcat(fi, "(");
			strcat(fi, newC.f2);
			strcat(fi, ")");
		}
		else{
			fprintf(stderr, "DYNAMIC MEMORY CAN NOT ALLOCATE!\n" );
			exit(EXIT_FAILURE);
		}
		gloClient.timeInterval = newC.timeInterval;
		gloClient.pidid = newC.pidid;
		gloClient.operation = newC.operation;
		strcpy(gloClient.f1, fi);
		gloClient.mem=fi;
		gloClient.fifo1=fd;
		gloClient.fifo2=fd2;

		newC.timeInterval *= 100.0;
		lastTI = milisecond + newC.timeInterval;
		while(1){
			signal(SIGINT, catchSigInt);

			eval = evaluator_create(fi);
			answer=calculateFunction(eval, milisecond, lastTI, resolution);
			fprintf(stderr, "answer%lf\n",answer);
			milisecond += newC.timeInterval;
			lastTI += newC.timeInterval;
			write(fd2, &answer, sizeof(double));
			delay(5000000);
		}

	}
	return 0;
}





