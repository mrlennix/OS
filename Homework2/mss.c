/*
 * Name: Emmanuel Lennix
 * ID #: 10011049990
 * Programming Assignment 2
 * Description: Shakespeare Word Search
 */


//libraries used
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

//the maximum number of character that the user can enter for the program
#define max_char 100


//These methods are defined below
char ** parse_tokens(char* );
void help();
void supported_commands(char **);
int search_(char*,int);
int job(int,char*,int);

/*
 * Function: Main
 * Parameters: argc - the count of the input passed by a outside source
 * argv - the string given from another program or outside source
 * Returns: An integer that signals the successfull termination  of the program.
 * Description: The starting point for the program that handles all the 
 * functionality.
 */
int main(int argc, char* argv[] ) 
{ 
//We are going to use this to keep track of the users input
char data[max_char];
//The command from to user gets parsed and stored here and that information gets
//processed in different methods later.
char ** command;

printf("Welcome to the Shakespeare word count service.\n");

while(1)
{
	printf("Enter: Search [word] [workers] to start your search.\n");
	
	do
	{	
	printf("> ");
	//stores users input
	fgets(data,max_char,stdin);
	}//if no input is obtained keep requesting it until it is given
	while(!strcmp(data,"\n"));
	data[strlen(data)-1]='\0';
	//onc command is parsed it gets stored in a 2d array in cammand
	command=parse_tokens(data);
	//if the first part of the string is quit then program ends
	if(!strcmp(command[0],"quit"))break;
	//figures out which task needs to be performed from the input
	supported_commands(command);
	
}//end of while



  return 1 ;
}//end of main







/*
* Function: do_task
* Paramter(s): str - A double char pointer which has
* all the different tokens given from the user.
* Returns: It doesn't return anything because this method
* only does a task and nothing needs to be returned.
* Decription: This method takes each of the tokens provided
* by the user, and uses the global count to determine which
* to type of task should be perform.
 */
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



/*
 * Funtion; help
 * Parameter(s): NONE
 * Return: NONE
 * Discription: This method prints the help message that will be displayed
 * to the user.
 */
void help()
{
printf("Shakespeare Word Search Service Command Help\n");
printf("_____________________________________________\n");
printf("help\t- displays this message\n");
printf("quit\t- exits\n");
printf("search [word] [workers] - searches the works of Shakespeare for [word] using\n");
printf("                          [workers].  [workers] can be from 1 to 100.\n");
}



/*
 * Function: search_
 * Parameter(s):word - A string that contains the word being searched.
 * workers - A integer that represents the user number of workers.
 * Returns: An integer that has the total number of times the word repeats
 * throughout the file.
 * Desciption: Spawns multiple processes based on the number of workers
 * and retrieves information from it's children.
 */
int search_(char* word,int workers)
{
//Used to keep track of all child processes id and helps the
//main program wait on its children
pid_t* child=(pid_t*)malloc(sizeof(pid_t)*workers);
//Used for creating a pipe for each of the children
//and allows  for communication between the parent and child
int** fds = (int**)calloc(workers,sizeof(int*));
//index - used to represent the postion of each child
//buff - stores the value returned from the child
//result -keeps track of the count of all the words
int status, index=0,buff,result=0;


//creating the pipes for all the children
for(index=0;index<workers;index++)fds[index]=(int*)calloc(2,sizeof(int));
//reset index so we fork the right number of children
index=0;

//The main program  forks tons of workers to do out memory mapping
//those processes return their count of how many times the word
//repeats in there section of the file and
while(index<workers)
{
	pipe(fds[index]);
	//child
	if((child[index]=fork())==0)
	{
		//close the read end of the pipe because it will not
		//be used in the child
		close(fds[index][0]);
		int num=job(index,word,workers);
		//writes the number of counted by the child back
		//to the parent	
		write(fds[index][1],&num,sizeof(num));
		exit(0);
	}
	//cloded the write end of the pipe in the parent because
	//the parent will not use it
	close(fds[index][1]);
	//increment the index to spawn a new child
	index++;	

}
//wait for all procces to end


for(index=0;index<workers;index++)
{	
	//wait for child so we before we collect data from out pipe
	waitpid(child[index],&status,0);
	//read our value returned from child and store it in buff
	read(fds[index][0],&buff,sizeof(buff));
    	//update the results with the value passed from the child
	 result+=buff;
	//deallocate each because we're done with it
	free(fds[index]);
}
//deallocate child
free(child);
//deallocate our pipes
free(fds);
return result;

}//end of the search_ method





/*
 * Function: supported_command
 * Parameter(s):command - 2D array of characters that has the users input
 *Returns: NONE
 *Description: Performs the command that the user wants.
 *
 */
void supported_commands(char ** command)
{


int workers;
if(!strcmp(command[0],"help")){help();return;}

if(command[2]==NULL)return;
if(!(workers=atoi(command[2])))return;
if(workers<0 || workers>100)return;

if(!strcmp(command[0],"search"))
{
//Used to store the time taken for the search to execute
float time;
/*
 * start - moment the program start it's search
 * end  - moment the program ended it's search
 */
struct timeval start, end;
//get start time
gettimeofday(&start,NULL);

int num=search_(command[1],workers);
//get the end time
gettimeofday(&end,NULL);
//cumpute the time taken for the search to execute
time =(end.tv_sec*1000000+end.tv_usec) - (start.tv_sec*1000000+start.tv_usec);
//print the result of the search along with the time taken
printf("Found %d instances of %s in %f microseconds\n",num,command[1],time);
}
}//end of supported_command



/*
 * Function: job
 * Parameter(s);
 * index - A integer representing which child process is performing the job (memory mapping)
 * word - A string representing the word the user wishes to search.
 * workers - A integer represeting how many workers there are and used to calulate how much work
 * each worker gets.
 * Returns: An integer that represents the number of time the users word is used in the file.
 * Description: This method provides a job to each of the workers by make sure each worker gets 
 * an equal work load;
 *
 */
int job(int index,char* word,int workers)
{


//The mapping address will get store here allowing us to keep track of it.
void* pmap;

//We are going to store the file into memfd so we can process and track it.
int memfd;
//Used to obatin the size of our file so we can map the work
struct stat sbuf;
/*
 * resule - Keep track of the amout of times the word is repeated
 * start - used determines where the child should start reading from file
 * end - used to determine where should I stop reading.
 */
int result=0,i,start,end;
char* ptr;
//each keeps track of what the equal workload should be and use
int each;
//open the file so we can read it only
memfd=open("shakespeare.txt",O_RDONLY);
//if error exit
if(memfd==-1)
{
	perror("open");
	exit(1);
}
//this si where we get our informationa bout the file and
//store it into sbuff
if(stat("shakespeare.txt",&sbuf) == -1)
{
	perror("stat");
	exit(1);
}
//map the file into memore so we can read from it later
pmap = mmap(0,sbuf.st_size,PROT_READ,MAP_SHARED,memfd,0);
if(pmap == (void*)-1)
{
	perror("mmap");
	exit(1);
}
//computes how much work each worker needs
each=sbuf.st_size/workers;
//computes where to start reading from
start=index*each;
//computes where we should stop reading from
end=each*(index+1);
//if this is the last child we just read until the end of the file
if(index+1==workers)end=sbuf.st_size;
//link our bites to characters so we can read chars from the file
ptr=pmap;

//mapping the nessary work load is done here
for(i=start;i<end;i++)
{
	//if the current string in memory is the same and the word we're
	//looking for then we increment our results
	if(!memcmp(ptr+i,word,strlen(word)))
	{
		result++;
	}
}


//close our file
close(memfd);
//unmap our memor
munmap(pmap,sbuf.st_size);
//return our results back
return result;
}







