#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include <sys/wait.h>

#define BUFSIZE 1000

int main(int argc, char *argv[]){
	int i;
	if(argc!=3){
		printf("Wrong argc:%d,need 2.\n",argc-1);
		return 1;
	}
	if(strlen(argv[1])!=5){
		printf("Format: HH.MM /PATH\n");
		return 1;
	}
	if(strchr(argv[1],'.')==NULL){
		printf("Format: HH.MM /PATH\n");
		return 1;
	}
	int x=100;//temp for string->int
	char hh[10];//hour string
	strcpy(hh,argv[1]);
	strchr(hh,'.')[0]='\0';
	x=atoi(hh);
	if(x<0 || x>23){
		printf("Hour range [0-23]\n");
		return 1;
	}
	char *mm=strchr(argv[1],'.')+1;//minute string
	x=100;
	x=atoi(mm);
	if(x<0 || x>59){
		printf("Minute range [0-59]\n");
		return 1;
	}
	char listdir33[BUFSIZE]="";//this is the path of the crontab.txt
	memset(listdir33,'\0',100);
	readlink("/proc/self/exe", listdir33, 100);
	listdir33[strlen(listdir33)-11]='\0';
	strcat(listdir33,"crontab.txt");
	FILE *fp;
	errno=0;
	fp=fopen(listdir33,"a");
	fprintf(fp, "%s %s * * * XDG_RUNTIME_DIR=/run/user/$(id -u) DISPLAY=:0.0 /usr/bin/rhythmbox-client --play-uri %s\n",mm,hh,argv[2]);
	fclose(fp);
	
	char* arr[] = {"crontab", listdir33, NULL};
	pid_t pid=fork();
	if (pid==0) // child for executing crontab
	{
		execv("/usr/bin/crontab", arr); 
		exit(0);
	}
	else{
		wait(0);
		printf("DONE!\n");
		return 0;
	}
}









