#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <windows.h>
#include <io.h> 
#define DEBUG   0		//DEBUG or Not
#define DMAX 10000		//Section length
#define MAX_CONF	30  //Max Conf File Number In A Dir
#define E_RANGE		5	//


char section[DMAX];		//A Secti
void writeInFp(FILE * fp,char * data){
	fputs(data,fp);
	if(data[strlen(data)] != 10) fputc(10,fp);
}
void writeInFile(char *name,char *data){
	FILE * fp = fopen(name,"a");
	fputs(data,fp);
	if(data[strlen(data)] != 10) fputc(10,fp);
	fclose(fp);
}
bool isSpace(char ch){
	char * str = "\t\r\n ";
	int len = strlen(str);
	for(int i = 0 ; i < len ; i ++){
		if(ch == str[i]) return true;
	}
	return false;
}
char cut(){
	return getchar();
}
//-95-94，-43-62
//isSpace and check Section Number
int  checkTitle(char * data,int sn){

	int len = strlen(data),i = 0,number = 0 ;
	//Trim Section
	while(isSpace(data[i++]));
	char * str = data + i - 1;
	i = 0;
	//Get Section Number
	while(str[i] >= 48 && str[i] <= 57){
		number = number * 10 + (str[i] - 48);
		i ++;
	}
	
	if(number == 0) return sn; 

//	printf("number => %d\n",number);	
//	if(number <= sn - E_RANGE || number >= sn + E_RANGE ) return sn;
	sn = number;

	str = str + i;
	//filter "、章"
	char filter[] = {-95,-94,-43,-62};
	char format[150] = {0};
	int f_len = strlen (filter);
loop:
	for(i = 0 ;i < f_len ; i = i +2){
		if(filter[i] == str[0] && filter[i+1] == str[1]){
			str +=2;
			goto loop;
		} 
	}
	sprintf(format,"第%d章 %s",number,str);
	//strcpy(data,format);
	//data = (char *) malloc(sizeof(char) * (strlen(format) + 1));
//	printf("|%s|\t|%s|",data,format);

	strcpy(data,format);
//	printf("SN => %d\t|%s|",sn,data);	
//	if(sn >260)	cut();
	return sn;
}

int findDirFiles(char * lpPath,char * ptn,char **res){
	char szFind[128];
	int fnum = 0 ;
	strcpy(szFind,lpPath);
    strcat(szFind,ptn);
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(szFind,&FindFileData);
	if(INVALID_HANDLE_VALUE == hFind)   
        return -1;
	do{
		if(fnum >= MAX_CONF) return fnum;
        if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
            
        }else{
			res[fnum ] = (char *)malloc(sizeof(char)*40);
			sprintf(res[fnum ++ ],"%s\\%s",lpPath,FindFileData.cFileName);
		//	printf("%s\\%s\n",lpPath,FindFileData.cFileName);
        }
    }while(FindNextFile(hFind,&FindFileData));
    FindClose(hFind);
	return fnum;
}
int processOneFile(char * fileName){
	char * fName = strdup(fileName);
	char * name  = strrchr(fileName,'\\')+1;
	strtok(name,".");
	char tmpFileName[100] = {0};
	char sourceFileName[100] = {0};
	sprintf(tmpFileName,"title\\%s.txt",name);
	sprintf(sourceFileName,"tmp\\%s.txt",name);
	remove(tmpFileName);
	FILE * infp = fopen(tmpFileName,"a");
	FILE * fp = fopen(sourceFileName,"r");
	if(fp == NULL){
		printf("FIle Not Found!\n");
		return 0;
	}
	char ch = 0;
	int num = 0,sn = 0,p = 0;
	while( (ch = fgetc(fp) ) != EOF){
		section[num++] = ch;
		if(ch == 10){
			section[num++] = '\0';
			if(strlen(section) > 0 && section[0] != 10 ){
				p = sn ;
				sn = checkTitle(section,sn);
			
				writeInFp(infp,section);
			}
			num = 0;
		}
	}
	fclose(fp);
	fclose(infp);
	return 0;
}

int loadDirSourceFile(char *dir,char *ptn){
	char * fres[MAX_CONF];
	int staticRes = findDirFiles(dir,ptn,fres);
	if(staticRes == -1){
		printf("No File Found In \"%s\"!\n",dir);
		return -1;
	}else{
		for(int t = 0 ; t < staticRes ; t ++){
			printf("Find File \"%s\"\n",fres[t]);
			processOneFile(fres[t]);
			printf("Process \"%s.txt\" Over!\n",fres[t]);
		}
	}
	return 0;
}

void init(){	
	access("title",0)  && system("mkdir title"); 
	access("tmp",0)  && system("mkdir tmp"); 
}

void tst(){
	char *str = "   \t\r\n 0261 乐器兵魂，曲谱兵法和古殿大门【看到这一章，你就会发现，这是一个很特别的构思】";
	char ddt[10000] = {0};

	strcpy(str,"sad");
	puts("asdad");
	getchar();
	strcpy(ddt,str);
	checkTitle(ddt,0);
}
//-125 126;
int main(){
//	tst();
	init();
	loadDirSourceFile("tmp","\\*.txt");
	getchar();
	return 0;
}