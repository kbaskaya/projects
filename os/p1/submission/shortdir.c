#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>

#define BUFSIZE 1000

int main(int argc, char *argv[]){
	int i;
	if(argc<2){
		printf("Invalid argument.\n");
		return 1;
	}
	char fileline[BUFSIZE];//line from file
	char line[BUFSIZE]="";//line to write
	char pwd[BUFSIZE];//pwd
	char listdir33[BUFSIZE]="";//path of DIRLIST.txt
	char listdir33new[BUFSIZE]="";//temp path of DIRLIST2.txt
	FILE *fp;
	FILE *fp2;
	memset(listdir33,'\0',100);
	readlink("/proc/self/exe", listdir33, 100);
	listdir33[strlen(listdir33)-8]='\0';
	strcpy(listdir33new,listdir33);
	strcat(listdir33,"DIRLIST.txt");
	strcat(listdir33new,"DIRLIST2.txt");
	getcwd(pwd,BUFSIZE);
	errno=0;
	fp=fopen(listdir33,"r+");
	if(errno==2){
		fp=fopen(listdir33,"a+");
		fclose(fp);
	}
	
	if(strcmp(argv[1],"set")==0){//---SET---------------
		if(argc!=3){
			printf("No name argument.\n");
			return 1;
		}
		strcpy(line,argv[2]);
		strcat(line," ");
		strcat(line,pwd);
		strcat(line,"\n");
		fp=fopen(listdir33,"r+");
		int found=0;
		int linenumber=1;
		while(fgets(fileline,BUFSIZE,fp)!=NULL){
			if(strstr(fileline,argv[2])==fileline){
				found=1;
				break;
			}
			linenumber++;
		}
		if(!found){//not found
			fprintf(fp,"%s",line);
			fclose(fp);
		}
		else{//found    //if the name was already set, we duplicate the file line by line, and change the path of the name to the new path.
			fclose(fp);
			fp=fopen(listdir33,"r");
			fp2=fopen(listdir33new,"w");
			int templinenumber=1;
			while(fgets(fileline,BUFSIZE,fp)!=NULL){
				if(templinenumber!=linenumber)fprintf(fp2,"%s",fileline);
				else{
					fprintf(fp2,"%s",line);
				}
				templinenumber++;
			}
			remove(listdir33);
			rename(listdir33new,listdir33);
			fclose(fp);
			fclose(fp2);
		}
		printf("%s is set as an alias for %s\n",argv[2],pwd);
	}
	else if(strcmp(argv[1],"jump")==0){//---JUMP---------------//jumps both here (just in case) and in seashell
		if(argc!=3){
			printf("No name argument.\n");
			return 1;
		}
		int r;
		int f=0;
		char *ddir;
		char dddir[200]="";
		fp=fopen(listdir33,"r");
		while(fgets(fileline,BUFSIZE,fp)!=NULL){
			if(strstr(fileline,argv[2])==fileline){
				f=1;
				ddir=strchr(fileline,' ');
				ddir++;
				strcpy(dddir,ddir);
				dddir[strlen(dddir)-1]='\0';
				//printf("%s\n",dddir);
				break;
			}
		}
		fclose(fp);
		if(f){
			r=chdir(dddir);
			if(r!=0){
				printf("Error while changing directory.\n");
				return 1;
			}
		}
		else{
			printf("Name not found.\n");
			return 1;
		}
		
	}
	else if(strcmp(argv[1],"del")==0){//---DEL---------------
		if(argc!=3){
			printf("No name argument.\n");
			return 1;
		}
		fp=fopen(listdir33,"r+");
		int found=0;
		int linenumber=1;
		while(fgets(fileline,BUFSIZE,fp)!=NULL){
			if(strstr(fileline,argv[2])==fileline){
				printf("word found!\n");
				found=1;
				break;
			}
			linenumber++;
		}
		if(!found){//not found
			printf("Name not found.\n");
			fclose(fp);
		}
		else{//found  //works similar to set
			fclose(fp);
			fp=fopen(listdir33,"r");
			fp2=fopen(listdir33new,"w");
			int templinenumber=1;
			while(fgets(fileline,BUFSIZE,fp)!=NULL){
				if(templinenumber!=linenumber)fprintf(fp2,"%s",fileline);
				templinenumber++;
			}
			remove(listdir33);
			rename(listdir33new,listdir33);
			fclose(fp);
			fclose(fp2);
		}
		printf("%s is deleted.\n",argv[2]);
	}
	else if(strcmp(argv[1],"clear")==0){//---CLEAR---------------
		if(argc!=2){
			printf("Invalid argument.\n");
			return 1;
		}
		remove(listdir33);
		fp=fopen(listdir33,"a+");
		fclose(fp);
		printf("All associations are cleared.");
	}
	else if(strcmp(argv[1],"list")==0){//---LIST---------------
		if(argc!=2){
			printf("Invalid argument.\n");
			return 1;
		}
		fp=fopen(listdir33,"r");
		while(fgets(fileline,BUFSIZE,fp)!=NULL){
			printf("%s",fileline);
		}
	}
	else {
		printf("Invalid argument: %s\n",argv[1]);
		return 1;
	}	
	
	return 0;
}


