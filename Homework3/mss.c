/*
 * Name: Emmanuel Lennix
 * ID #: 10011049990
 * Programming Assignment 3
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
#include<pthread.h>
#include<assert.h>
//the maximum number of character that the user can enter for the program
#define max_char 100

//Descriptive header defined below
char ** parse_tokens(char* );

void help();

void supported_commands(char **);

int search_(char*,int);

static void* job(void*);

int replace(char*,char*,int);

static void* replace_job(void*);

void reset();

//mutex locak and shared variable for searching
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//the shared variable that is protect in the search
int sharedvar=0;

typedef struct stuff
{
	
	int workers;
	
	int index;
	
	char* word;
	
}s;

typedef struct rep
{
	
	int workers;
	
	int index;
	
	char* orig;
	
	char* replace;
	
}r;

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
		
		//replace \n with \0
		data[strlen(data)-1]='\0';
		
		//onc command is parsed it gets stored in a 2d array in cammand
		command=parse_tokens(data);
		
		//if the first part of the string is quit then program ends
		if(!strcmp(command[0],"quit"))break;
		
		//figures out which task needs to be performed from the input
		supported_commands(command);
		
		//reset shared variable for next iteration
		sharedvar=0;
		
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
	
}//end of parse_token

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
	
	printf("replace [word 1] [word 2] [workers] - search the works of Shakespeare for\n");
	
	printf("                          [word 1] using [workers] and replaces each\n");
	
	printf("                          instance with [word 2].  [workers] can be from\n");
	
	printf("                          1 to 100.\n");
	
	printf("reset\t- will reset the memory mapped file back to its original state.\n");
	
}//end of help

/*
 * Function: search_
 * Parameter(s):word - A string that contains the word being searched.
 * workers - A integer that represents the user number of workers.
 * Returns: An integer that has the total number of times the word repeats
 * throughout the file.
 * Desciption: Spawns multiple threads  based on the number of workers
 * and retrieves information from it's children.
 */
int search_(char* word,int workers)
{
	
	//keeps track of the thread id so we can use use the to wait
	pthread_t  threadID[100];
	
	//used a loop variable
	int index;
	
	for(index=0;index<workers;index++)
	{
		
		//Used only for the individual  thread to pass data to them
		struct stuff* data = malloc(sizeof(struct stuff));
		
		(*data).workers=workers;
		
		(*data).index=index;
		
		(*data).word=word;
		
		pthread_create(&threadID[index],0,&job,(void*)data);
		
	}
	
	for(index=0;index<workers;index++)pthread_join(threadID[index],NULL);
	
}//end of search_

/*
 * Function: supported_command
 * Parameter(s):command - 2D array of characters that has the users input
 *Returns: NONE
 *Description: Performs the command that the user wants.
 *
 */
void supported_commands(char ** command)
{
	
	//Used to keep track of the value of workers obtained from user
	int workers;
	
	if(!strcmp(command[0],"help")){help();return;}
	
	if(!strcmp(command[0],"search"))
	{
		
		if(command[2]==NULL)return;
		
		if(!(workers=atoi(command[2])))return;
		
		if(workers<0 || workers>100)return;
		
		//Used to store the time taken for the search to execute
		float time;
		
		/*
		 * start - moment the program start it's search
	 	* end  - moment the program ended it's search
	 	*/
		struct timeval start, end;
		
		//get start time
		gettimeofday(&start,NULL);
		
		//store the value count returned from the search_ method
		int num=search_(command[1],workers);
		
		//get the end time
		gettimeofday(&end,NULL);
		
		//cumpute the time taken for the search to execute
		time =(end.tv_sec*1000000+end.tv_usec) - (start.tv_sec*1000000+start.tv_usec);
		
		//print the result of the search along with the time taken
		printf("Found %d instances of %s in %f microseconds\n",sharedvar,command[1],time);
		
		fflush(NULL);
		
		return;
	}
	
	if(!strcmp(command[0],"replace"))
	{
		
		if(command[3]==NULL)return;
		
		//Used to keep track of word1 and word2
		char *str1 =command[1],*str2=command[2];
		
		workers=atoi(command[3]);
		
		if(workers==0)return;
		
		//x - used as loop variable
		//diff - used to computer difference in string length
		int x,diff;
		
		//stores length of word1
		int word1len=strlen(str1);
		
		//stores length of word2
		int word2len=strlen(str2);
		
		if(word1len>word2len)
		{
			
			diff=word1len-word2len;
			
			for(x=0;x<diff;x++)strcat(str2," ");
			
		}
		
		replace(str1,str2,workers);
		
		return;
		
	}	

	if(!strcmp(command[0],"reset"))
	{
		
		printf("Reset!\n");
		
		reset();	
		
		return;
		
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
static void* job(void* args)
{
	
	//stores the worker, index,word so we can access it here
	struct stuff* x = (struct stuff*) args;
	
	//makes it easier to use index
	int index=(*x).index;
	
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
	
	//Used to  compare our memory location 
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
	each=sbuf.st_size/(*x).workers;

	//computes where to start reading from
	start=index*each;

	//computes where we should stop reading from
	end=each*(index+1);

	//if this is the last child we just read until the end of the file
	if(index+1==(*x).workers)end=sbuf.st_size;

	//link our bites to characters so we can read chars from the file
	ptr=pmap;

	//mapping the nessary work load is done here
	for(i=start;i<end;i++)
	{
		
		//if the current string in memory is the same and the word we're
		//looking for then we increment our results
		if(!memcmp(ptr+i,(*x).word,strlen((*x).word)))
		{
			
			result++;
		
		}
		
	}

	//close our file
	close(memfd);

	//unmap our memor
	munmap(pmap,sbuf.st_size);
	
	//lock mutex to protect shared variable
	pthread_mutex_lock(&lock);
	
	//return our results back
	sharedvar+=result;
	
	//unlock mutex so another thread can use
	pthread_mutex_unlock(&lock);

	return NULL;

}//end of job

/*
 * Function: replace
 * Parameter(s):  originalword - An string that stores the word that will
 * be replaced.
 * replacedword - A string that stores the word that will eventuall replace the 
 * original.
 * workers - A integer that represents the number of workers that will be used
 * to do a job.
 * Returns: A integer represing the success of method
 * Description: Spawns threads based on the number of workers, and waits
 * for them to finish doing their job.
 */
int replace(char* originalword,char* replacedword ,int workers)
{

	//Stores the id of each thread so we can join them later
	pthread_t* threadID[100];

	//used as a loop variable to make sure we spawn the right number of workers
	int x;

	for(x=0;x<workers;x++)
	{

	//STORES A COPY OF EACH VARIABLE SO WE CAN USE IT LATER IN THREAD
	struct rep* data = malloc(sizeof(struct stuff));
	
	(*data).workers = workers;
	
	(*data).orig = originalword;
	
	(*data).replace=replacedword;
	
	(*data).index=x;
	
	pthread_create(&threadID[x],NULL,&replace_job,(void*)data);

	}

	//wait for each thread to end
	for(x=0;x<workers;x++)
	{
		
		pthread_join(threadID[x],NULL);
		
	}
	
	return 1;
	
}//end of replace

/*
 * Funtion replace_job
 * Parameter(s) : agrs - A address of the value being passed since we needed a
 * structure to prevent having to many global variables.
 * Returns: NULL
 * Description: Memory maps file and does it's required work load on the job
 * then returns the results in a shared variable because this method is spawned in 
 * a thread.
 */
static void* replace_job(void* args)
{
	
	//Stores worker, index, word1,word2 so we can use them
	struct rep * x = (struct rep*) args;
	
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
	memfd=open("shakespeare.txt",O_RDWR);

	if(memfd==-1)
	{
		
		perror("open");
		
		exit(1);
		
	}

	//store it into sbuff
	if(stat("shakespeare.txt",&sbuf) == -1)
	{
		
		 perror("stat");
		
		exit(1);
		
	}
	
	//mapping the file to read and write in memory
	pmap = mmap(0,sbuf.st_size,PROT_READ | PROT_WRITE,MAP_SHARED,memfd,0);

	if(pmap == (void*)-1)
	{
		
		perror("mmap");
		
		exit(1);                                   
		 
	}

	each=sbuf.st_size/(*x).workers;


	//start=index*each;
	//computes where we should stop reading from
	end=each*((*x).index+1);

	//if this is the last child we just read until the end of the file
	if((*x).index+1==(*x).workers)end=sbuf.st_size;
	
	//link our bites to characters so we can read chars from the file
	ptr=pmap;
	
	
	//mapping the nessary work load is done here
	for(i=start;i<end;i++)
	{
		
		//if the current string in memory is the same and the word we're
		//looking for then we increment our results
		if(!memcmp(ptr+i,(*x).orig,strlen((*x).replace)))
		{
			
			memcpy(ptr+i,(*x).replace,strlen((*x).replace));
			
			result++;
			
		}
		
	}
	
	//close our file
	close(memfd);
	
	//unmap our memor
	munmap(pmap,sbuf.st_size);
	
	//lock shard variable
	pthread_mutex_lock(&lock);
	
	//return our results back
	sharedvar+=result;
	
	//unlock shared variable
	pthread_mutex_unlock(&lock);
	
	return NULL;
	
}//end of replace_job

/*
 * Function: reset
 * Parameters(s): none
 * Returns: none
 * Description: Reset the file to its original state using the backup file
 */
void reset()
{

	system("cp shakespeare_backup.txt shakespeare.txt");

}//end of reset



