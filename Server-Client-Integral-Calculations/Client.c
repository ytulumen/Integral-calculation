#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#define MAXD 256
#define FIFO_FILE_NAME "communicate"
#define BACK_FIFO_FILE_NAME "respondComm"
typedef struct 
{
	pid_t pidid;
	double timeInterval;
	char operation;
	char *f1;
	char *f2;
}Client;
void catchSigInt(int signo) {
	fprintf(stderr,"myExit\n");
	FILE *logP;
	char logFile[MAXD];
	sprintf(logFile, "%ld", (long)getpid());
	strcat(logFile, ".txt");
	logP = fopen(logFile, "a");
	fprintf(logP, "CTRL+C handled.\nClient died.\nProgram has been terminated!\n");

	exit(EXIT_SUCCESS);
}
char* readFile(FILE* inf){
	char *input = malloc(sizeof(char)*MAXD);
	fread(input, sizeof(char), MAXD, inf);
	return input;
}
int main(int argc, char **argv){
	Client newC;
	if(argc != 5){
		fprintf(stderr, "Usage: %s MainFifoName\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if(argv[1][0]!='f' || strlen(argv[1])>2 || atoi(&argv[1][1]) > 6 || atoi(&argv[1][1]) < 1){
		fprintf(stderr, "Usage: %s MainFifoName\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if(argv[2][0]!='f' || strlen(argv[2])>2 || atoi(&argv[2][1]) > 6 || atoi(&argv[2][1]) < 1){
		fprintf(stderr, "Usage: %s MainFifoName\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	printf("%c\n", argv[4][0]);
	if(strlen(argv[4])>1 || !(argv[4][0] == '+' || argv[4][0] == '-' ||
		argv[4][0] == '/' || argv[4][0] == '*')){
		fprintf(stderr, "Usage: %s MainFifoName\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, catchSigInt);
	char fileName[MAXD];
	fileName[0] = '\0';
	sprintf(fileName, "%ld", (long)getpid());
	strcat(fileName, "mkfifo");
	if((mkfifo(fileName, 0666)==-1) && (errno != EEXIST) ){
		fprintf(stderr, "FIFO CAN NOT CREATE!\n" );
		exit(EXIT_FAILURE);
	}
	

	FILE *funci, *funcj, *logP;
	char func1[MAXD], func2[MAXD], logFile[MAXD];
	strcpy(func1, argv[1]);
	strcat(func1, ".txt");
	strcpy(func2, argv[2]);
	strcat(func2, ".txt");

	funci=fopen(func1, "r");
	funcj=fopen(func2, "r");
	if( funci == NULL || funcj == NULL){
		fprintf(stderr, "File could not open!\n" );
		exit(EXIT_FAILURE);
	}
	newC.pidid=(long)getpid();
	newC.timeInterval = atoi(argv[3]);
	newC.operation = argv[4][0];
	newC.f1 = readFile(funci);
	newC.f2 = readFile(funcj);
	



	double ans;
	int fd, fd2;
	fd=open(FIFO_FILE_NAME, O_WRONLY);
	write(fd, &(newC.pidid), sizeof(pid_t));
	write(fd, &(newC.timeInterval), sizeof(double));
	write(fd, &(newC.operation), sizeof(char));
	write(fd, newC.f1, sizeof(char)*MAXD);
	write(fd, newC.f2, sizeof(char)*MAXD);
	sprintf(logFile, "%ld", (long)getpid());
	strcat(logFile, ".txt");
	logP = fopen(logFile, "w");

	while(1){
		fd2=open(fileName, O_RDONLY);
		read(fd2, &ans, sizeof(double));
		fprintf(stderr, "answer is:%lf\n", ans);
		fprintf(logP, "time interval-> %lf\noperation-> %c\nf1-> %s\nf2-> %s pidId:%d\nResult:%lf\n", 
			newC.timeInterval, newC.operation, newC.f1, newC.f2, newC.pidid, ans );
		close(fd);
	}

	return 0;
}













