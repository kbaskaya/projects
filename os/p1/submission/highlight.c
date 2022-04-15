#include<stdio.h>
#include<string.h>

#define BUFSIZE 2000

int main(int argc, char *argv[]){
	int i;
	
	if(argc!=4){
		printf("Wrong argc:%d,need 3.\n",argc-1);
		return 1;
	}
	if(!(strcmp(argv[2],"r")==0 || strcmp(argv[2],"g")==0 || strcmp(argv[2],"b")==0)){
		printf("Wrong format:%s,can be 'r', 'g' or 'b'.\n",argv[2]);
		return 1;
	}
	FILE *fp=NULL;
	fp=fopen(argv[3],"r");
	if(fp==NULL){
		printf("Cannot find file at:%s\n",argv[3]);
		return 1;
	}
	
	int x;
	char *word=argv[1];
	char line[BUFSIZE];//line from file
	char newline[BUFSIZE+20];//line with
	char *colorcode;
	char *dcolorcode="\033[0m";//default color code
	int wordlength=strlen(word);
	char *p;
	if(strcmp(argv[2],"r")==0)colorcode="\033[0;31m";
	else if(strcmp(argv[2],"g")==0)colorcode="\033[0;32m";
	else colorcode="\033[0;34m";
	
	while(fgets(line,BUFSIZE,fp)!= NULL){
		p=strstr(line,word);
		if(p!=NULL){//if word is found
			memset(newline,'\0',BUFSIZE+20);
			x=p-line;//how many chars before word
			strncpy(newline,line,x);
			printf("%s",newline);
			printf("%s",colorcode);
			printf("%s",word);
			printf("%s",dcolorcode);
			printf("%s",line+x+wordlength);
		}
		//printf("--------------\n");
	}
	fclose(fp);
	return 0;
}


