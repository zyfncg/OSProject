#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int BPB_BytsPerSec=512; //每扇区字节数
int BPB_SecPerClus=1;  //每簇扇区数
int BPB_RootEntCnt=224;  //根目录最大文件数
int RootDirSectors;
int FirstDataSectors;


struct DirEntry{
	char DIR_Name[11];
	char DIR_Attr;
	char useless[10];
	unsigned short DIR_WrtTime;
	unsigned short DIR_WrtDate;
	unsigned short DIR_FstClus;  //此条目开始簇号
	unsigned int   DIR_FileSize; //文件大小
	
};

//输出函数
void myprint(char str[]){
        int length;
        length=strlen(str);
        
	my_print(str,length);
	printf("length: %d   %s",length,str);
}

//调用汇编函数
int my_print(char str[],int length);

//文件读取
int fileRead(FILE* file,void* ptr,long begin,int size);

//根目录获取
void getBootDirEntry(FILE* file,struct DirEntry entryListPtr[]);

//输出根目录
void printBootDir(FILE* file,struct DirEntry entryListPtr[]);

//判断是否是有效文件或目录 如果是返回1,并将文件名放在name中，否则返回0 
int getVaildName(char* name,char* ptr,char type);

//输出文件夹及文件
void printAllDir(FILE* file,struct DirEntry entryListPtr[],int dirSize,char path[]);

//读取文件中数据
int getFileData(FILE* file,short firstClus,char text[]);

//判断是否是子字符串,如果path等于str返回2，如果 path是str子字符串返回1，如果 str是path子字符串返回-1，都不是返回0 
int isSubString(char path[],char str[]);

//目录或文件具体输出
int printDetailData(FILE* file,struct DirEntry entryListPtr[],int dirSize,char path[],char target[]);

//目录文件计数
void countDirFile(FILE* file,struct DirEntry entryListPtr[],int dirSize,char path[],char target[],int* dirNum,int* fileNum);

int printCount(FILE* file,struct DirEntry entryListPtr[],int dirSize,char path[],char target[],char space[]);

//解析命令如果文件输出命令返回1，count命令返回2，否则返回0 
int commandCheck(char* input);

//正整数转字符串
void intToStr(int n,char* str);

int main(){ 
  
    FILE* img=NULL;
    char target[100];
    char path[100]="\0";
    char* ptr;
    int command;

    if((img = fopen("abc.img","rb"))==NULL) {  
    	myprint("File cannot be opened\n");  
   	    exit(0);  
    } 
     
	struct DirEntry entryListPtr[224];
	
	getBootDirEntry(img,entryListPtr);

//	printBootDir(img,entryListPtr);

	
	
	printAllDir(img,entryListPtr,BPB_RootEntCnt,path);
	
	
	while(1){
		//scanf("%s",target);
		gets(target);
		ptr=target;
		command=commandCheck(ptr);
		
		if(command==1){
			printDetailData(img,entryListPtr,BPB_RootEntCnt,path,ptr);	
		}else if(command==2){
			printCount(img,entryListPtr,BPB_RootEntCnt,path,ptr,""); 
		}else if(command!=-1){
			break;
		}
		
	}
	
//	printDetailData(img,entryListPtr,BPB_RootEntCnt,path,target);
	 
	
//	puts(buffer);

    if(fclose(img)!=0) {  
    	myprint("File cannot be closed\n");   
    	exit(1);   
    }else{
		myprint("File is now closed\n"); 
    }
  
    getchar();
	getchar();
    return 0;
} 


//目录文件计数
void countDirFile(FILE* file,struct DirEntry entryListPtr[],int dirSize,char path[],char target[],int* dirNum,int* fileNum){
	int i,j;

	
	char nameptr[12];
	short firstClus;
	
	for(i=0;i<dirSize;i++){
		
		if(entryListPtr[i].DIR_Attr==0x10){
			
			if(getVaildName(nameptr,entryListPtr[i].DIR_Name,entryListPtr[i].DIR_Attr)==1){

				
				firstClus=entryListPtr[i].DIR_FstClus;
				char name[120];
				strcpy(name,path);
				strcat(name,nameptr);
				
				if(isSubString(name,target)!=0){

					if(isSubString(name,target)==-1){
						(*dirNum)++;
					}
					strcat(name,"/");
				
					int entrySize=BPB_BytsPerSec>>5;
					struct DirEntry entryListPtr1[entrySize];
					long clusBegin=(FirstDataSectors+firstClus-2)*BPB_BytsPerSec;
				
					if(fileRead(file,entryListPtr1,clusBegin,BPB_BytsPerSec)==0){
						myprint("目录读取失败");
					}
					countDirFile(file,entryListPtr1,entrySize,name,target,dirNum,fileNum);
			
				}
				
				
				
			}
			
		}
	}
	
	for(i=0;i<dirSize;i++){
		
		if(entryListPtr[i].DIR_Attr==0x20){
			if(getVaildName(nameptr,entryListPtr[i].DIR_Name,entryListPtr[i].DIR_Attr)==1){
				
				char fileAllName[120];
				strcpy(fileAllName,path);
				strcat(fileAllName,nameptr);
				
				if(isSubString(fileAllName,target)==-1){
					(*fileNum)++;
				}
			}
			
		}
	}
 
} 

int printCount(FILE* file,struct DirEntry entryListPtr[],int dirSize,char path[],char target[],char space[]){
	int i,j;
	int dirNum=0,fileNum=0;
	int* dirNumPtr=&dirNum;
	int* fileNumPtr=&fileNum;
	int isHave=0;
	int entrySize=BPB_BytsPerSec>>5;
	int isEmpty=1;
	long clusBegin;
	char nameptr[12];
	char temp[120];
	short firstClus;
	char output[180];
	char num[30];
	
	if(strchr(target, '.')){
		myprint(target);
		myprint("\e[1;31m is not a directory!\n\e[0m");
		return 0;
	}
	printf("%s\n",path);
	for(i=0;i<dirSize;i++){
		printf("===%d===\n",i);
		if(entryListPtr[i].DIR_Attr==0x10){
			
			if(getVaildName(nameptr,entryListPtr[i].DIR_Name,entryListPtr[i].DIR_Attr)==1){
				isEmpty=0;
				
				firstClus=entryListPtr[i].DIR_FstClus;
				char name[120];
				strcpy(name,path);
				strcat(name,nameptr);
				
				if(isSubString(name,target)!=0){
					printf("+++++++++++++++++++++++++++++++++\n");
					isHave=1;
					strcpy(temp,space);
					//strcat(temp," ");
					strcat(temp,nameptr);
					strcat(name,"/");
				
					
					struct DirEntry entryListPtr1[entrySize];
					clusBegin=(FirstDataSectors+firstClus-2)*BPB_BytsPerSec;
				
					if(fileRead(file,entryListPtr1,clusBegin,BPB_BytsPerSec)==0){
						myprint("目录读取失败");
					}
					if(isSubString(name,target)==-1||isSubString(name,target)==2){
						dirNum=0;
						fileNum=0;
						countDirFile(file,entryListPtr1,entrySize,name,target,dirNumPtr,fileNumPtr);
						strcpy(output,temp);
						strcat(output,": ");
						intToStr(fileNum,num);
						strcat(output,num);
						strcat(output," files, ");
						intToStr(dirNum,num);
						strcat(output,num);
						strcat(output," directories\n");
						myprint(output);
						//printf("%s: %d files, %d directories\n",temp,fileNum,dirNum);
						//printf("%s",output);
						strcpy(temp,space);
					    strcat(temp,"   ");
					}else{
						strcpy(temp,space);
					}
					printf("----------------%d---------------------\n",i);
					printCount(file,entryListPtr1,entrySize,name,target,temp);
					printf("----------------%d---------------------\n",i);
				}
				
				
				
			}
			
		}
		
	}
	
	if(isHave==0&&(isSubString(path,target)==1||isSubString(path,target)==0)){
		myprint("\e[1;31mUnknown path\n\e[0m");
	}
	return 1;
}


//解析命令如果文件输出命令返回1，count命令返回2，否则返回0 
int commandCheck(char* input){
	int i=0,j=0;
	int isCommand=1;
	char command[7]="count ";
	for(i=0;i<6;i++){
		if(command[i]!=input[i]){
			isCommand=0;
		}
	}
	if(isCommand==1){
		i=5;
		while(input[i]==' '){
			i++;
		}
		j=0;
		while(input[j+i]!='\0'){
			input[j]=input[j+i];
			j++;
		}
		input[j]='\0';
		return 2;
	}else if(input[0]=='#'){
		return 0;
	}else if(input[0]=='\0'){
		return -1;
	}else{
		return 1;
	}
} 

//判断是否是子字符串,如果path等于str返回2，如果 path是str子字符串返回1，如果 str是path子字符串返回-1，都不是返回0 
int isSubString(char path[],char str[]){
	int i=0;
	while(path[i]!='\0'&&str[i]!='\0'){
		if(path[i]!=str[i]){
			return 0;
		}
		i++;
	}
	
	if(path[i]=='\0'){
		if(str[i]=='\0'){
			return 2;
		}else{
			return 1;
		}
	}else{
		return -1;
	}
	
	return 1;
} 

//目录或文件具体输出
int printDetailData(FILE* file,struct DirEntry entryListPtr[],int dirSize,char path[],char target[]){
	int i,j;
	char outBlue[8]="\e[1;34m";
	char outGreen[8]="\e[1;32m";
	char colorEnd[5]="\e[0m";
	int isHave=0;
	int isEmpty=1;
	char nameptr[12];
	short firstClus;

	char fileAllName[120];
	char output[120];
	char name[120];	

	for(i=0;i<dirSize;i++){
		
		if(entryListPtr[i].DIR_Attr==0x10){
			
			if(getVaildName(nameptr,entryListPtr[i].DIR_Name,entryListPtr[i].DIR_Attr)==1){
				isEmpty=0;
				
				firstClus=entryListPtr[i].DIR_FstClus;
				
				strcpy(name,path);
				strcat(name,nameptr);
				
				if(isSubString(name,target)!=0){
					isHave=1;
					
					strcat(name,"/");
				
					int entrySize=BPB_BytsPerSec>>5;
					struct DirEntry entryListPtr1[entrySize];
					long clusBegin=(FirstDataSectors+firstClus-2)*BPB_BytsPerSec;
				
					if(fileRead(file,entryListPtr1,clusBegin,BPB_BytsPerSec)==0){
						myprint("目录读取失败");
					}
					printDetailData(file,entryListPtr1,entrySize,name,target);
			
				}
				
				
				
			}
			
		}
	}
	
	for(i=0;i<dirSize;i++){
		
		if(entryListPtr[i].DIR_Attr==0x20){
			if(getVaildName(nameptr,entryListPtr[i].DIR_Name,entryListPtr[i].DIR_Attr)==1){
				
				isEmpty=0;
				
				
				strcpy(output,outBlue);
				strcat(output,path);

				strcpy(fileAllName,path);
				strcat(fileAllName,nameptr);
				
				
				//如果文件全名与输入字符串相等就输出文件内容 
				if(isSubString(fileAllName, target)==2){
					char text[65536];
					isHave=1;
					firstClus=entryListPtr[i].DIR_FstClus;
					getFileData(file,firstClus,text);
					myprint(text);
					myprint("\n");	
				}else if(isSubString(fileAllName,target)==-1){
					isHave=1;
					
					strcat(output,colorEnd);
					strcat(output,outGreen);
					strcat(output,nameptr);
					strcat(output,colorEnd);
					
					myprint(output);
					myprint("\n");
				}
			}
			
		}
	}
	if(isHave==0&&(isSubString(path,target)==1||isSubString(path,target)==0)){
		
		if(strchr(target, '.')){
			myprint("\e[1;31mUnknown file\n\e[0m");
		}else{
			myprint("\e[1;31mUnknown path\n\e[0m");
		}
	}else if(isEmpty==1&&(isSubString(path,target)==-1||isSubString(path,target)==2)){

			strcpy(output,outBlue);
			strcat(output,path);
			strcat(output,colorEnd);
			myprint(output);
			myprint("\n");		
	}
	
	
	return isHave; 
} 

//读取文件中数据
int getFileData(FILE* file,short firstClus,char text[]){
	int size=0;
	unsigned short fatID=firstClus;   //簇号 
	unsigned short offset=0;    //偏移字节数 
	long fatAddr=BPB_BytsPerSec;
	unsigned short* fatIDPtr=&fatID;
	long dataclus;
	char* textPtr=text;
	
	while(1){
		dataclus=(FirstDataSectors+fatID-2)*BPB_BytsPerSec;
		if(fileRead(file,textPtr+BPB_BytsPerSec*size,dataclus,BPB_BytsPerSec)==0){
			myprint("数据读取失败");
			return 0; 
		}
		size++;
		if(fatID%2==0){
			offset=fatID/2*3;
			fatAddr=BPB_BytsPerSec+offset;
			if(fileRead(file,fatIDPtr,fatAddr,2)==0){
				myprint("文件FAT读取失败");
				return 0; 
			}
			fatID=fatID&0x0FFF; 
		}else{
			offset=fatID/2*3+1;
			fatAddr=BPB_BytsPerSec+offset;
			if(fileRead(file,fatIDPtr,fatAddr,2)==0){
				myprint("文件FAT读取失败");
				return 0; 
			}
			fatID=fatID>>4;
		}
		
		if(fatID>=0x0FF7){
			
			if(fatID>0x0FF7){
				break;
			}else{
				myprint("坏簇"); 
				return 0;
			}
		}
		
		
	}
	
	return 1;
	
} 

void printBootDir(FILE* file,struct DirEntry entryListPtr[]){
	int i,j;
	char name[12];
	char* nameptr=name;
	
	for(i=0;i<BPB_RootEntCnt;i++){
		
		if(entryListPtr[i].DIR_Attr==0x10||entryListPtr[i].DIR_Attr==0x20){
			nameptr=entryListPtr[i].DIR_Name;
			if(getVaildName(name,nameptr,entryListPtr[i].DIR_Attr)==1){
				myprint(name);
				myprint("\n");
			}
			
		}
	}
	char path[120]="";
	printAllDir(file,entryListPtr,BPB_RootEntCnt,path);
}

//输出文件夹及文件
void printAllDir(FILE* file,struct DirEntry entryListPtr[],int dirSize,char path[]){
	int i,j;

	int isEmpty=1;
	char nameptr[12];
	short firstClus;
	char outBlue[8]="\e[1;34m";
	char outGreen[8]="\e[1;32m";
	char colorEnd[5]="\e[0m";
	char output[120];
	char fileAllName[120];	

	for(i=0;i<dirSize;i++){
		
		if(entryListPtr[i].DIR_Attr==0x10){
			
			if(getVaildName(nameptr,entryListPtr[i].DIR_Name,entryListPtr[i].DIR_Attr)==1){
				isEmpty=0;
				
				firstClus=entryListPtr[i].DIR_FstClus;
				char name[120];
				strcpy(name,path);
				strcat(name,nameptr);
				strcat(name,"/");
				int entrySize=BPB_BytsPerSec>>5;
				struct DirEntry entryListPtr1[entrySize];
				long clusBegin=(FirstDataSectors+firstClus-2)*BPB_BytsPerSec;
				
				if(fileRead(file,entryListPtr1,clusBegin,BPB_BytsPerSec)==0){
					myprint("目录读取失败");
				}
				printAllDir(file,entryListPtr1,entrySize,name);
			}
			
		}
	}
	
	for(i=0;i<dirSize;i++){
		
		if(entryListPtr[i].DIR_Attr==0x20){
			if(getVaildName(nameptr,entryListPtr[i].DIR_Name,entryListPtr[i].DIR_Attr)==1){
				isEmpty=0;
				strcpy(output,outBlue);
				strcat(output,path);
				strcat(output,colorEnd);
				strcat(output,outGreen);
				strcat(output,nameptr);
				strcat(output,colorEnd);
				
					
				myprint(output);

				myprint("\n");
			}
			
		}
	}
	
	if(isEmpty==1){
		strcpy(output,outBlue);
		strcat(output,path);
		strcat(output,colorEnd);
		myprint(output);myprint("\n");
	}
} 

int getVaildName(char* name,char* ptr,char type){
	int i,k=0;
	int isVaild=1;
	int isFirst=1;
	for(i=0;i<11;i++){
		if(('a'<=ptr[i]&&ptr[i]<='z')||('A'<=ptr[i]&&ptr[i]<='Z')||('0'<=ptr[i]&&ptr[i]<='9')){
			name[k]=ptr[i];
			k++;
		}else if(ptr[i]==0x20){
			if(type==0x20){
				if(isFirst==1){
				    name[k]='.';
				    k++;
				    isFirst=0;
			    }
			}
			
		}else{
			isVaild=0;
			break;
		}
	}
	name[k]='\0';
	return isVaild;

} 
void getBootDirEntry(FILE* file,struct DirEntry entryListPtr[]){
	long rootBegin=19*BPB_BytsPerSec;

	if(fileRead(file,entryListPtr,rootBegin,BPB_RootEntCnt*32)==0){
		myprint("根目录读取失败");
	}
	RootDirSectors=(BPB_RootEntCnt*32+BPB_BytsPerSec-1)/BPB_BytsPerSec;  //设置根目录扇区数量 
	FirstDataSectors=19+RootDirSectors;   //设置数据区开始扇区号 
}

int fileRead(FILE* file,void* ptr,long begin,int size){

	//文件指针从第begin个字节处开始
	if(0!=fseek(file,begin,SEEK_SET)){
		myprint("fseek filed!\n");
		return 0;
	}
	//读取长度为size字节
	if(size!=fread(ptr,1,size,file)){
		myprint("fread failed!\n");
		return 0;
	}
	
	return 1;
}

//正整数转字符串
void intToStr(int n,char* str){
    int i=0;
    int count=0;
    char c;   
    
    do{
	i=n%10;
	n=n/10;
	str[count]=i+'0';
	count++;
    }while(n!=0);

    for(i=0;i<count/2;i++){
	c=str[i];
	str[i]=str[count-1-i];
	str[count-1-i]=c;	
	
    }
    str[count]='\0';

}
