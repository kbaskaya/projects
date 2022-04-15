#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>

#define BUFSIZE 2000
#define PATHSIZE 300

int main(int argc, char *argv[]){
	int i;
	int mode;//0=a=line comparison, 1=b=byte comparison
	char f1dir[PATHSIZE];
	char f2dir[PATHSIZE];

	
	if(!(argc==3 || argc==4)){
		printf("Wrong argc:%d,need at least 2.\n",argc-1);
		return 1;
	}
	if(argc==3){
		mode=0;
		if(strcmp(argv[1]+strlen(argv[1])-3,"txt")!=0){
			printf("Arg is not a txt file:%s\n",argv[1]);
			return 1;
		}
		if(strcmp(argv[2]+strlen(argv[2])-3,"txt")!=0){
			printf("Arg is not a txt file:%s\n",argv[2]);
			return 1;
		}
		strcpy(f1dir,argv[1]);
		strcpy(f2dir,argv[2]);
	}
	else {
		if(!(strcmp(argv[1],"-a")==0 || strcmp(argv[1],"-b")==0)){
			printf("Wrong format:%s,can be '-a' or '-b'.\n",argv[1]);
			return 1;	
		}
		if(strcmp(argv[1],"-a")==0)mode=0;
		else mode=1;
		if(mode==0){
			if(strcmp(argv[2]+strlen(argv[2])-3,"txt")!=0){
				printf("Arg is not a txt file:%s\n",argv[2]);
				return 1;
			}	
			if(strcmp(argv[3]+strlen(argv[3])-3,"txt")!=0){
				printf("Arg is not a txt file:%s\n",argv[3]);
				return 1;
			}
		}
		strcpy(f1dir,argv[2]);
		strcpy(f2dir,argv[3]);
	}
	printf("If one file has less lines than other file, it will be indicated as \"--NO LINE--\"\n");
	printf("File#1:%s\nFile#2:%s\n\n",f1dir,f2dir);
	errno=0;
	FILE *fp1=NULL;
	fp1=fopen(f1dir,"r");
	if(errno==2){//if file1 doesnt exist
		printf("First file is invalid\n");
		return 1;	
	}
	errno=0;
	FILE *fp2=NULL;
	fp2=fopen(f2dir,"r");
	if(errno==2){//if file2 doesnt exist
		printf("Second file is invalid\n");
		return 1;	
	}
	if(mode==0){//-a line comparison
		char line1[BUFSIZE];
		char line2[BUFSIZE];
		int endof1=0;
		int endof2=0;
		int counter=0;
		int curline=1;
		while(1){
			if(fgets(line1,BUFSIZE,fp1)== NULL){
				endof1=1;
				strcpy(line1,"--NO LINE--\n");
			}
			if(fgets(line2,BUFSIZE,fp2)== NULL){
				endof2=1;
				strcpy(line2,"--NO LINE--\n");
			}
			if(endof1 && endof2)break;
			if(strcmp(line1,line2)!=0){
				counter++;
				printf("File#1:Line%d: %s",curline,line1);
				printf("File#2:Line%d: %s",curline,line2);
			}
			curline++;
		}
		if (counter==0)printf("Files are identical.\n");
		else printf("%d different lines found.\n",counter);
	}
	
	else if (mode==1){//-b byte comparison
		int f1size,f2size,dif,i,i2;
		char f1byte,f2byte;
		fseek(fp1,0,SEEK_END);
		f1size=ftell(fp1);
		rewind(fp1);
		fseek(fp2,0,SEEK_END);
		f2size=ftell(fp2);
		rewind(fp2);
		printf("size1:%d size2:%d\n",f1size,f2size);
		if(f1size>=f2size){
			dif=f1size-f2size;
			i2=f2size;
		}
		else {
			dif=f2size-f1size;
			i2=f1size;
		}
		for(i=0;i<i2;i++){
			fread(&f1byte,1,1,fp1);
			fread(&f2byte,1,1,fp2);
			if(f1byte!=f2byte)dif++;
		}
		if (dif==0)printf("Files are identical.\n");
		else printf("Files are different in %d bytes.\n",dif);
	}
	fclose(fp1);
	fclose(fp2);
	
	return 0;
}

