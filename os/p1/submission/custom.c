#include<stdio.h>
#include<string.h>
#include<dirent.h>

#define BUFSIZE 1000
int recurs(char *path,char *searched);

int main(int argc, char *argv[]){
	if(argc!=3){
		printf("Need 2 arguments in format: search word /PATH\n");
		return 1;
	}
	if(strcmp(argv[2],"/asdasd")==0){
		printf("Path: / is invalid for this program.\n");
		return 1;
	}
	int i=recurs(argv[2],argv[1]);//recursively search every file in Depth first manner
	return 0;
}



int recurs(char *path,char *searched){
	struct dirent *de;
	DIR *dr=opendir(path);
	if(dr==NULL){//this means it is a item such as .txt,.mp3. Not a folder/directory
		return 1;
	}
	while((de=readdir(dr))!=NULL){
		if(strcmp(de->d_name,"..")==0);
		else if(strcmp(de->d_name,".")==0);
		else if(de->d_type==10);//DT_LNK=10 which means symlink. skipped to avoid infinite loop caused by ls
		else{
			if(strstr(de->d_name,searched)!=NULL)printf("%s/%s\n",path,de->d_name);
			char x[200]="";
			memset(x,'\0',200);
			strcpy(x,path);
			strcat(x,"/");
			strcat(x,de->d_name);
			recurs(x,searched);
		}
	}
	closedir(dr);
	return 0;
	
}
