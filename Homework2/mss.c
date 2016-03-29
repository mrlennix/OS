/*
 * Name: Emmanuel Lennix
 * ID #: 1001104990
 * Programming Assignment 2
 * Description: Shakespeare Word Search
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<sys/stat.h>


#define max_char 30

char ** parse_tokens(char* );
void help();
void supported_commands(char **);
int search_(char*,int);
int job(int,char*,int);


int main(int argc, int* argv[] ) 
{ 

char data[max_char];
char ** command;

printf("Welcome to the Shakespeare word count service.\n");

while(1)
{
	printf("Enter: Search [word] [workers] to start your search.\n");
	do
	{	
	printf("> ");
	fgets(data,max_char,stdin);
	}
	while(!strcmp(data,"\n"));
	data[strlen(data)-1]='\0';
	command=parse_tokens(data);
	if(!strcmp(command[0],"quit"))break;
	supported_commands(command);
	
}//end of while



  return 0 ;
}//end of main

char ** parse_tokens(char * str)
{

// Stores the parsed input of user string
char ** parse = (char**)malloc(max_char*sizeof(char*));
//temp variable used for hold the value of each token
char * token;
// this resets the global count
int count =0;
//intialize users input using strtok to split at space
token = strtok(str," ");
//this loops gets each token and
//stores it in parse so it can be returned later
//while updating the global var. for each token counted
 while(token != NULL)
{
 	parse[count]=token;
 	token=strtok(NULL," ");
 	count++;
}

return parse;
}//end of parse_ token

void help()
{
printf("Shakespeare Word Search Service Command Help\n");
printf("_____________________________________________\n");
printf("help\t- displays this message\n");
printf("quit\t- exits\n");
printf("search [word] [workers] - searches the works of Shakespeare for [word] using\n");
printf("                          [workers].  [workers] can be from 1 to 100.\n");
}

int search_(char* word,int workers)
{

pid_t* child=(pid_t*)malloc(sizeof(pid_t)*workers);
int** fds = (int**)calloc(workers,sizeof(int*));
int status, index=0,buff,result=0;


//creating the pipes for all the children
for(index=0;index<workers;index++)fds[index]=(int*)calloc(2,sizeof(int));

index=0;

//loops spawns a proccess to do each search
while(index<workers)
{
	pipe(fds[index]);
	//child
	if((child[index]=fork())==0)
	{
		//printf("p%d\n",index);
		close(fds[index][0]);
		int num=job(index,word,workers);	
		write(fds[index][1],&num,sizeof(num));
		exit(0);
	}
	close(fds[index][1]);
	index++;	

}
//wait for all procces to end


for(index=0;index<workers;index++)
{
	waitpid(child[index],&status,0);
	read(fds[index][0],&buff,sizeof(buff));
        result+=buff;
	free(fds[index]);
}
free(child);
free(fds);
return result;

}//end of the search_ method

void supported_commands(char ** command)
{


int workers;
if(!strcmp(command[0],"help")){help();return;}

if(command[2]==NULL)return;
if(!(workers=atoi(command[2])))return;
if(workers<0 || workers>100)return;

if(!strcmp(command[0],"search"))
{
float time;
struct timeval start, end;

gettimeofday(&start,NULL);

int num=search_(command[1],workers);

gettimeofday(&end,NULL);

time =(end.tv_sec*1000000+end.tv_usec) - (start.tv_sec*1000000+start.tv_usec);

printf("Found %d instances of %s in %f microseconds\n",num,command[1],time);
}
}//end of supported_command




int job(int index,char* word,int workers)
{
void* pmap;
int memfd;
struct stat sbuf;
int result=0,i,start,end;
char* ptr;
int each;

memfd=open("shakespeare.txt",O_RDONLY);
if(memfd==-1)
{
	perror("open");
	exit(1);
}
if(stat("shakespeare.txt",&sbuf) == -1)
{
	perror("stat");
	exit(1);
}
pmap = mmap(0,sbuf.st_size,PROT_READ,MAP_SHARED,memfd,0);
if(pmap == (void*)-1)
{
	perror("mmap");
	exit(1);
}
each=sbuf.st_size/workers;
start=index*each;
end=each*(index+1);

if(index+1==workers)end=sbuf.st_size;
//printf("sbuf size = %d each =%d\nstart= %d  end = %d\n",sbuf.st_size,each,start, end);
ptr=pmap;

for(i=start;i<end;i++)
{
	if(!memcmp(ptr+i,word,strlen(word)))
	{
		result++;
	}
}



close(memfd);
munmap(pmap,sbuf.st_size);
return result;
}







