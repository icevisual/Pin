#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <windows.h>
#include <io.h> 
#include<time.h>

#define DEBUG   0		//DEBUG or Not
#define WMAX   500		//Max Word Number
#define DMAX 10000		//Section length
#define AMAX   300		//The Number OF Char Kinds
#define OFFSET 150		//Negative \ Positive Offset
#define W_LENGTH	100	//The Max Length Of Key Word 	
#define C_INIT		50	//Context Capacity of initialization
#define C_INCREASE	25	//Context Capacity Of increasement
#define C_CXTLENGTh  5  //Context Half Length
#define V_LENGTH	50  //The Length Of Value For Replace 
#define MAX_CONF	30  //Max Conf File Number In A Dir



typedef struct ignorlNode{
    char isEnd;
    struct searchNode * next[AMAX];
}iNode,* iNP;	    	//Dictionary tree node, For Ignorl Dic


typedef struct searchNode{
    char isEnd;
	char * value;
    struct searchNode * next[AMAX];
}sNode,* sNP;	    	//Dictionary tree node, For Search

typedef struct treeNode{
    int windex;			//if this node is an end , it would be the index of word in dw
    struct treeNode * next[AMAX];
}Node,* NP;	    		//Dictionary tree node, To Add New Words

typedef struct leafNode{
    char ** context ;	//Contain The Contexts Each Line
	char ** enviroment; //Contain The Contexts Each Line
	char * start;
	char word[W_LENGTH];//Contain The Key Word
	int num;			//The Number Of Contexts Now 
	int capacity;		//Capacity Of Contexts 
}fNode,* fNP;			//Dictionary tree leaf node	,Store Word And Contexts 

iNP ign[AMAX]   = {0};	//ignorl Dic
sNP sea[AMAX]	= {0};	//Search Dic
NP  dic[AMAX]	= {0};	//Dictionary tree
fNP dw[WMAX]	= {0};	//Array With Words And Contexts,For Traversal Key Words
int dw_num = 0 ;
char section[DMAX];		//A Section Of The Page

void date(char * result){
	long t = time(0);
	struct	tm * tt = localtime(&t);
//		sprintf(result,"%04d-%02d-%02d %02d:%02d:%02d",tt->tm_year+1900,tt->tm_mon,tt->tm_mday,tt->tm_hour,tt->tm_min,tt->tm_sec);
	sprintf(result,"%02d%02d%02d",tt->tm_year+1900,tt->tm_mon,tt->tm_mday);
}
void printChar(char *str){
	int len = strlen(str);
	for(int i = 0 ; i< len ; i ++){
		printf("%4d => %4d\n",i,str[i]);
	}
}
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

//

void log(char * data){
	char name [20] = {0};
	char name1 [20] = {0};
	date(name1);
	sprintf(name,"log\\%s.txt",name1);
	writeInFile(name,data);
}



void printArray(int arr[],int n){
	for(int i = 0 ;i < n ; i ++){
		printf("%4d => %4d\n",i,arr[i]);
	}

}
//From The Index Of The Word And The EndIndex To Compare The String
//Get The StartIndex And EndIndex,Then Get The MaxPublicSubstring
int  getPublicSubstr(char * a,char * b,int indexa,int indexb,char *w,char *res){
	int lengthw = strlen(w),lengtha = strlen(a),lengthb = strlen(b);
	int start = 0,end = 0;
	int i = 0 , j = 0 ;
	int amark[C_CXTLENGTh * 2 ] = {0},bmark[C_CXTLENGTh * 2 ] = {0};
//	while()
	int na = 0,nb = 0;

	DEBUG && printf("STRA => |%s|\tLength => %d\nSTRB => |%s|\tLength => %d\n",a,indexa,b,indexb);

	while(i < indexa)	amark[na ++] = a[i] < 0 ? (i=i+2) - 1 :i ++;
	if(DEBUG) printArray(amark,na);
	i = 0 ;
	while(i < indexb)	bmark[nb ++] = b[i] < 0 ? (i=i+2) - 1 :i ++;
	if(DEBUG)  printArray(bmark,nb);
	
	
	if(na && nb){
		for(i = na - 1 , j = nb -1 ; i >=0 && j >= 0 ; i --,j --){
			int stA = i > 0 ? amark[ i - 1] : -1;
			int enA = amark[i];
			int stB = j > 0 ? bmark[ j - 1] : -1;
			int enB = bmark[j];
			stA ++,stB ++;
			int len = enA - stA;
			if(len == enB - stB){
				if(len == 1 ){
					if( !(a[stA] == b[stB] && a[enA] == b[enB]) ){
						break;
					}
				}else{
					if(a[stA] != b[stB] ){
						break;
					}
				}
			}
		}
		DEBUG && printf("Left\t=> i = %4d\t j = %4d\n",i,j);


		start = i == -1 ? -1:amark[i];
	}else{
		start = na > 0 ?indexa-1:-1;
	}


	i = indexa + 1, j = indexb + 1;
	while( i < lengtha && j < lengthb ){
		if(a[i] < 0){
			if(!(a[i]==b[j] && a[i + 1]==b[j + 1] )) break;
			i ++,j ++;
		}else{
			if(a[i] != b[j]) break;
		}
		i ++, j ++;
	}
	end = i;
	DEBUG && printf("Right\t=> i = %4d\t j = %4d\n",i,j);
	for(i = start+1 ,j = 0; i < end ;res [j ++ ] = a[i], i ++){
		DEBUG && printf("i = %4d => %4d \n",i,a[i]);
	}
	DEBUG && printf("\n");
	res[j] = '\0';
	DEBUG && printf("PUBLIC SUBSTR => |%s|\n",res);
	return start + 1;
}
// Add The Context Of The Word In The Section Into The Leaf Node
void addContext(fNP node,char * word,char * context,int start){
	if(node->capacity == 0){//When Initialization
		node->capacity	= C_INIT;
		node->start		= (char *)malloc(node->capacity * sizeof(char));
		node->context	= (char **)malloc(node->capacity * sizeof(char*));
		node->enviroment= (char **)malloc(node->capacity * sizeof(char*));
		memset(node->context,0,node->capacity * sizeof(char*));
		memset(node->enviroment,0,node->capacity * sizeof(char*));
	}else if(node->capacity < node->num + 5 ){//To Increace
		int oldCapacity = node->capacity;
		node->capacity += C_INCREASE;
		node->start		= (char *)realloc(node->start,node->capacity * sizeof(char));
		node->context	= (char **)realloc(node->context,node->capacity * sizeof(char*));
		node->enviroment= (char **)realloc(node->enviroment,node->capacity * sizeof(char*));
		//TODO:confirm
		memset(node->context+oldCapacity,0,C_INCREASE * sizeof(char*));
		memset(node->enviroment+oldCapacity,0,C_INCREASE * sizeof(char*));
	}
	
	int debug = DEBUG;
	int clen = strlen(context);
	char res[200];
	int newindex =  0;

	for(int i = 0 ; i < node->num ; i ++){
		debug && printf("CONTEXT[ %d ] => |%s|\n",i,node->context[ i ]);
		newindex = getPublicSubstr(	node->context[ i ],context,node->start[ i ],start,word,res);
		if( strcmp(res ,word) != 0){
			int cilen =  strlen(node->context[ i ]);
			if(node->enviroment[ i ] == 0 ){
				int elen = cilen > clen ? cilen : clen;
				node->enviroment[ i ] = (char *)malloc(( elen + 1 ) * sizeof(char));
				node->enviroment[ i ][0] = '\0';
			}

			int envLength = strlen(node->enviroment[ i ]);
			
			if( clen > envLength && clen > cilen ){
				strcpy(node->enviroment[ i ],context);
			}else if( cilen > envLength && cilen > clen  ){
				strcpy(node->enviroment[ i ],node->context[ i ]);
			}
			strcpy(node->context[ i ],res);
			node->start[ i ] -= newindex;
			debug && printf("AFTER REPLACE start => %4d\tcontext => |%s|\n",node->start[ i ],node->context[ i ]);
			return ;
		}

		debug && getchar();
	}

	node->start[ node->num ] = start;
	node->context[ node->num ] = (char *)malloc(( clen + 1 ) * sizeof(char));
	strcpy(node->context[ node->num ],context);
	node->num ++;
}


void printLeafNode(char * name){
	FILE *fp = fopen(name,"w+");
	if(fp == NULL){
		printf("Error\n");
		return ;
	}
	char logs [ C_CXTLENGTh * 100 ];
	for(int i = 1 ; i < dw_num ; i ++){
		sprintf(logs,"%4d => \"%s\"\n",i,dw[i]->word);
		//printf("%s",logs);
		log(logs);
		if(dw[i]->num >0){
			DEBUG && printf("\n");
			for(int j = 0 ; j < dw[i]->num  ;j ++){
				DEBUG &&	printf("\t%4d => \"%s\"\n",j,dw[i]->context[j] );
				sprintf(logs,"\t%4d =>\t%2d \"%s\"",j,dw[i]->start[j],dw[i]->context[j] );
				log(logs);
				sprintf(logs,"%s=%s",dw[i]->context[j] ,dw[i]->context[j] );
				if(dw[i]->enviroment[j] != 0 ){
					sprintf(logs,"%s\t%s",logs ,dw[i]->enviroment[j] );
				}
				writeInFp(fp,logs);
			}
			DEBUG && printf("\n");
		}
	}
	fclose(fp);
}

//Add The Word And The Context Into The Dictionary Tree
/**
	Build Dictionary Tree With Words.
	Add Words And Contexts Into Leaves
**/
void addWord(NP * dic,fNP * dw,char * w,char * c,int start){
    int length = strlen (w);
    NP * nNode ;
    nNode = dic;
    for(int i = 0 ; i < length ; i ++ ){
        int index  = w[i] + OFFSET  ;
        if(nNode[ index ] == 0){
            nNode[ index ] = (NP)malloc(sizeof(Node));
			nNode[ index ]->windex = 0 ;
            for(int j = 0 ; j < AMAX ;  nNode[ index ]->next[j] = 0,j ++);
        }
		
		if(i == length - 1){
			//The Last latter,leaf Node
			if(nNode[ index ]->windex == 0 ){
				//New Leaf Node
				dw_num ++;
				dw[dw_num] = (fNP) malloc(sizeof(fNode));
				dw[dw_num]->capacity = 0 ;
				dw[dw_num]->num = 0 ;

				strcpy(dw[dw_num]->word,w);
				nNode[ index ]->windex = dw_num;
			}//Add Context Then
			DEBUG && printf("|%s|\t|%s|\n",w,c);

		
			addContext(dw[nNode[ index ]->windex],dw[nNode[ index ]->windex]->word,c,start);

		}
		
        nNode = nNode[ index ]->next;
    }
}


/**
Get The Context Of Word,Here 
	'data'	is the source ,
	'res'	is the resultset,
	'index' is the index of word,
	'wlen'	is the length of word,
	'num'	is the length of context forword / backword
**/
int getPinContext(char * data,char * res ,int index,int wlen,int num){
	int i = 0;
	int start = index - 1, end = index + wlen ;
	int length = strlen(data);
	int wmark[DMAX] = {0};
	int n = 0;
	while(i <= start){
		if(data[i] < 0 ){
			wmark[n ++] = i + 1;
			i++;
		}else{
			wmark[n ++] = i;
		}
		i ++;
	
	}

//	n = n - num - 1;
	if(n <= num){
		start = 0;
	}else{
		start = wmark[n - num - 1] + 1;
	}

	i = 0 ;
	while(i < num && end < length ){
		if(data[end] < 0 ){
			end += 2;
		}else{
			end ++;
		}
		i ++;
	}

	int j = 0;
	for(i = start ; i < end ; i ++){
		res[j ++] = data[i];
	}
	DEBUG && printf("START3 => %d\tEND => %d\n",start,end);
	res[ j ] = '\0';
	j --;
	while(res[j] == 10) res[ j-- ] = '\0';
	return start;
}

int isWord(char ch){
	if(
	//	(ch >= 65 && ch <= 90) 
	//	|| 
		(ch >= 97 && ch <= 122) 
	//	|| ch == 46
		){
		return 1;
	}
	return 0;
}
int isPin(char ch1,char ch2){
	if( ch1 == -88 && (ch2 >= -95 && ch2 <= -65 )){
		return 1;
	}
	return 0;
}
char ignoreList[100][20] = {0};
int ignoreNum = 0;
//Build Ignore File
int ignore(char * data){
	if(strlen(data) == 1 && isWord(data[0])) return 1;
//	char ignoreList[7][20] = {"ps","Ps","vs","www","isuu","com","com)"};
	int i = 0 ;
	for(i = 0 ; i < ignoreNum ; i ++){
		if(strcmp(data,ignoreList[i]) == 0){
			return 1;
		}
	}
	return 0;
}

char cut(){
	char ch = getchar();
	return ch;
}
//Get The Whole Word In The Section,i means the index 
int getWholeWord(char * data,int i ,int length){
	int j = 0 ,k = 0 ;
	char res [200] = {0};
	char word[200] = {0};
	while( ( i + j < length 
		&& data[ i+j ] >0  && data[ i+j ] != 10 && data[ i+j ] != 46 //isWord(data[ i+j ]) 
		) 
			|| ( i + j + 1 < length && isPin(data[ i+j ],data[ i+j+1 ])  ) ){
		if( i + j + 1 < length && isPin(data[ i+j ],data[ i+j+1 ] ) ){
			j ++;
		}
		j ++;
	}
	DEBUG && printf("----------------------START\n");
	for( k = 0 ; k < j ; k ++ ){
		word [ k ] = data[ i+k ];
	}
	DEBUG && printf("WORD=>|%s|\n",word);
	word [ k ] = '\0';
	if(ignore(word)){
		return j;
	}


	DEBUG && printf("CHAR=>|%4d|\t|%4d|\t|%c|\ti = |%4d|\tj = |%4d|\n",i,data[i],data[i],i,j);
	
	
//	printChar(data);
	int start = getPinContext(data,res,i,j,C_CXTLENGTh);

	DEBUG && printf("I = %4d\t START INDEX =>%4d\n",i,start);
	DEBUG && printf("WORD CONTEXT=>\n\t===CONTEXT START===\n%s\n\t===CONTEXT   END===\n",res);
	DEBUG && printf("----------------------END\n");

	addWord(dic,dw,word,res,(i-start));

	if(DEBUG){
	
		char ch =  getchar();
	}
//	cut();

	return j;
}

//Check The Section If There Are Pin
int checkPin(char * data){

	int length  = strlen(data);
	char res [200] = {0};
	int flag = 0;
	int i = 0,j = 0 ;
	while(i < length){
		if( data[i] > 0 ){
			if( isWord(data[i]) ){
				j = getWholeWord(data, i ,length);
				i = i + j;
				continue;
			}
		}else{
			if( i + 1 < length && isPin(data[i],data[ i+1 ]) ){
				j = getWholeWord(data, i ,length);
				i = i + j;
				continue;
			}
			i ++;
		}
		i ++;
	}
	return flag;
}

//-125 126;
void addWordForSearch(sNP * dic,char * w,char *v){
    int length = strlen (w);
    sNP * nNode ;
    nNode = dic;
    for(int i = 0 ; i < length ; i ++ ){
        int index  = w[i] + OFFSET  ;
        if(nNode[ index ] == 0){
            nNode[ index ] = (sNP)malloc(sizeof(sNode));
			nNode[ index ]->isEnd = 0 ;
            for(int j = 0 ; j < AMAX ;  nNode[ index ]->next[j] = 0,j ++);
        }
		
		if(i == length - 1){
			//The Last latter
			nNode[ index ]->value = (char*)malloc(sizeof(char)*50);
			nNode[ index ]->isEnd = 1 ;
			strcpy(nNode[ index ]->value,v);
			//printf("Index => %4d\tname => |%s|\tValue => |%s|\n",index,w,nNode[ index ]->value);
		}
        nNode = nNode[ index ]->next;
    }
}
int ignorlLetter(char ch){
	char * arr = "\t\r\n ";
	int length =  strlen(arr);
	for(int tp = 0 ; tp < length ; tp ++){
		if(ch == arr[tp]) return 0;
	}
	return 1;
}
void separateConf(char *data,char *name,char *value){
	int i = 0,j = 0 ,eqIndex = 0 ;
	int len = strlen(data);

	i= 0;
	while(eqIndex < len && data[eqIndex++] != '=');
	while(i < eqIndex ) name[i] = data[i++];
	name[i-1] = '\0';
	while(i < len && ignorlLetter(data[i]) ) value[j++] = data[i++];
	value[j] = '\0';
}




void loadReplaceConf(char * staticFileName){
	FILE * fp = fopen(staticFileName,"r");
	if(fp == NULL){
		printf("FIle \"%s\" Not Found!\n",staticFileName);
		return ;
	}
	char ch = 0,name[50],value[50];
	int num = 0;
	while( (ch = fgetc(fp) ) != EOF){
		section[num++] = ch;
		if(ch == 10 ){
			section[num-1] = '\0';
			if(strlen(section) > 0 && section[0] != 10 ){
				separateConf(section,name,value);
				//printf("name => |%s|\tvalue => |%s|\n",name,value);
				addWordForSearch(sea,name,value);
			}
			num = 0;
		}
	}
	fclose(fp);

}



void searchDic(char *data,sNP *dic){
	int length = strlen(data);
	int i = 0 , j = 0,isEnd = 0;
	sNP * nNode ;
    nNode = dic;
	for(i = 0 ; i < length ; i++){
		int index  = data[i] + OFFSET  ;
		if(nNode[ index ] != 0 ){
			if(i == length -1){
				printf("isEnd => %d\n",nNode[ index ]->isEnd);
				if(nNode[ index ]->isEnd)
				printf("value => |%s|\n",nNode[ index ]->value);
			}
			nNode = nNode[ index ]->next;
		}else{
			printf("Not Find\n");
		}
	}
}
void searchDicReplace(char *data,sNP * dic){
	int length = strlen(data);
	int i = 0 , j = 0,isEnd = 0,n = 0,k = 0;
	char section[DMAX] = {0};
	char value[50];
	sNP * nNode ;
    nNode = dic;
	for(i = 0 ; i < length ; i++){
		int index  = data[i] + OFFSET  ;
		nNode = dic;
		if(nNode[ index ] != 0 ){
			j = i;
			while( nNode[ index ] != 0 ){
				isEnd = nNode[ index ]->isEnd;
				if(isEnd == 1){
					strcpy(value,nNode[ index ]->value);
				}
				nNode = nNode[ index ]->next;
				j ++;
				index = data[j] + OFFSET  ;
			}
			if(isEnd){
				//if next char is word or pin continue
				if(j < length){
					if(data[j] > 0 ){
						if(isWord(data[j])){
							section[ n ++ ] = data[i];
							continue;
						}
					}else if(j + 1 < length && data[j] < 0 ){
						if(isPin(data[j],data[ j+1 ])){
							section[ n ++ ] = data[i];
							continue;
						}
					}
				}
				//if the prev char is word or pin continue
				//todo
			
				for(k = i ; k < j ; k ++){
				DEBUG &&	printf("%c",data[k]);
				}
				DEBUG &&	printf("\t Len => %d,%d,|%s|\n",i,j - i,value );
				int vlen = strlen(value);
				k = 0;
				while(k < vlen){
					section[ n ++ ] = value[k ++];
				}
				i = j - 1;
			}else{
				section[ n ++ ] = data[i];
			}
		}else{
			section[ n ++ ] = data[i];
		}
	}
	section[ n ] = '\0';
	strcpy(data,section);
}

void processStaticConf(char *fileName,char * tmpFileName){
	FILE * fp = fopen(fileName,"r");
	if(fp == NULL){
		printf("File \"%s\" Not Found!\n",fileName);
		return;
	}
	remove(tmpFileName);
	FILE * fpIn = fopen(tmpFileName,"a");
	
	char ch = 0;
	int num = 0;
	while( (ch = fgetc(fp) ) != EOF){
		section[num++] = ch;
		if(ch == 10){
			section[num++] = '\0';
			if(strlen(section) > 0 && section[0] != 10 ){
				searchDicReplace(section,sea);
				DEBUG && printf("section => |%s|\n",section);
				searchDicReplace(section,sea);
				searchDicReplace(section,sea);
				writeInFp(fpIn,section);
				DEBUG && getchar();
			}
			num = 0;
		}
	}
	fclose(fp);
	fclose(fpIn);
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


void loadIgnoreWords(){
	FILE * fp = fopen("conf\\ignore.txt","r");
	if(fp == NULL){
		printf("FIle Not Found!\n");
		return ;
	}
	char ch = 0,temp[100];
	int num = 0;
	while( (ch = fgetc(fp) ) != EOF){
		temp[num++] = ch;
		if(ch == 10 ){
			temp[num-1] = 0;
			strcpy(ignoreList[ignoreNum ++],temp);
		//	printf("%s|%s\n",ignoreList[ignoreNum-1 ],temp);
			num = 0;
		}
	}
	printf("Load %d Ignore Words\n",ignoreNum);
	fclose(fp);
	//Build A Ignore Word Dic Tree To Store Info And To Search
}

int loadDirConf(char *dir,char *ptn){
	char * fres[MAX_CONF];
	int staticRes = findDirFiles(dir,ptn,fres);
	if(staticRes == -1){
		printf("No Conf File Found In \"%s\"!\n",dir);
		return -1;
	}else{
		for(int t = 0 ; t < staticRes ; t ++){
			printf("Loading \"%s\"\n",fres[t]);
			loadReplaceConf(fres[t]);
			printf("File \"%s\" Load Complete\n",fres[t]);
		}
	}
	return 0;
}


void loadConf(){
	//load Static Conf
	int staticRes = loadDirConf("conf\\static","\\*.static.conf");
	int otherRes  = loadDirConf("conf","\\*.conf");
	int outerRes  = loadDirConf(".","\\*.static.conf");
}
void init(){	
	access("conf",0)  && system("mkdir conf"); 
	access("tmp",0)   && system("mkdir tmp"); 
	access("rest",0)  && system("mkdir rest"); 
	access("txt",0)   && system("mkdir txt"); 
	access("log",0)   && system("mkdir log"); 
	loadIgnoreWords();
	loadConf();
}
void processOneFile(char * fileName){
	char * fName = strdup(fileName);
	char * name  = strrchr(fileName,'\\')+1;
	strtok(name,".");
	char tmpFileName[100] = {0};
	char confFileName[100] = {0};
	sprintf(tmpFileName,"tmp\\%s.txt",name);
	sprintf(confFileName,"rest\\%s.conf",name);
	processStaticConf(fName,tmpFileName);
	puts("ProcessStaticConf Over");


	FILE * fp = fopen(tmpFileName,"r");
	
	if(fp == NULL){
		printf("File \"%s\" Not Found!\n",tmpFileName);
		return;
	}
	char ch = 0;
	int num = 0;
	while( (ch = fgetc(fp) ) != EOF){
		section[num++] = ch;
		if(ch == 10){
			section[num++] = '\0';
			if(strlen(section) > 0 && section[0] != 10 ){
				checkPin(section);
			}
			num = 0; 
		}
	}
	fclose(fp);
	printLeafNode(confFileName);
}

int loadDirSourceFile(char *dir,char *ptn){
	char * fres[MAX_CONF];
	int staticRes = findDirFiles(dir,ptn,fres);
	if(staticRes == -1){
		printf("No Conf File Found In \"%s\"!\n",dir);
		return -1;
	}else{
		for(int t = 0 ; t < staticRes ; t ++){
			printf("Find File \"%s\"\n",fres[t]);
			processOneFile(fres[t]);
			printf("Process \"%s\" Over!\n",fres[t]);
		}
	}
	return 0;
}

int main(int argc,char *argv[]){
	init();
	//system("cmd /c dir c:\\windows\\*.* /a-d /b /s >c:\\allfiles.txt");
//	cut();
	loadDirSourceFile("txt","\\*.txt");
	puts("Process Over , Press Any Key To Exit!");
	cut();
	return 0;
}
