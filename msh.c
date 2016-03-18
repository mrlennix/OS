/*
 * Name:Emmanuel Lennix	 
 * ID #: 1001104990
 * Programming Assignment 1
 * Description: Build a successfull shell that compiles.
 */


//libaries
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>
#include<signal.h>
/*
 * global varianles:
 * numOftokens - this value is the max number of tokens allowed.
 * Change this if you need to be able to read more
 * than 10 different tokens.
 *
 * count - is used to keep tract of how many split there are in
 * the users string. It will always be defined in the parse_token method.
 * 
 * numOfchar - this is the number of character the user can input. Change
 * this if you wish to exceed 100 characters on input
*/
int numOftokens=10,count;
int numOfchar =100;


//function prototypes
//Requirement 18 provided above each actual method
void task(char str[]);
char ** parse_tokens(char *);
void do_task(char **);
void nothing();

/*
 *
 *THIS METHOD IS THE MAIN:
 *
 * IT TAKES USER INPUT AND PARSES EACH STRING
 *THEN IT SENDS OF THAT INFO TO ANOTHER FUNCTION FOR PROCESSING
 * THE USERS COMMAND
 */
int main( void ) 
{

//define var
//Stores the length of the user input for removing '\n'
int len;
//Stores the parsed command provided by the user
char ** parsecommand =(char **)malloc(numOftokens*sizeof(char*));
//Stores the users input
char * command  = (char*)malloc(numOfchar*sizeof(char)),SHELL[]="msh>";

//this calls nothing if control+c is called
signal(SIGINT,nothing);
//this calls nothing if control+z is called so the program isn't stopped
signal(SIGTSTP,nothing);

//This is the main loop for the program to run the shell
while ( strcmp(command,"exit") )
{
	//print shell format
	printf("%s",SHELL);
	
	//read users input
	fgets(command,numOfchar,stdin);
	
	//removes '\n' and replaces with '\0'
	if(command[len=(strlen(command)-1)]=='\n')command[len]='\0';
	//if equal to quit it will terminate
	if(!strcmp(command,"quit"))break;
	 //if equal to exit or quit don't do task
	if( strcmp(command,"exit")  )
	{
		//parse string command from use
		parsecommand=parse_tokens(command);
		if(!strcmp("",command))continue;
		//do users task
		do_task(parsecommand);
	}
}

  return 0 ;
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
void do_task(char **str)
{
int x;
//Stores the bin address for the commands
char bin[50]="/usr/bin/";

//this statement changes is for changing directories
if(!strcmp(str[0],"cd")){
chdir(str[1]);
return;
}
//concats the the users bin with the users command
strcat(bin,str[0]);
//Forks so the job can be executed by the child,
//so the main program won't end.
pid_t child =fork();
//Task only performed by the child
if(child==0)
{	//this switch is use to do certain jobs based on the user input
	switch(count)
	{
		case 1:execl(bin,str[0],NULL);break;
		case 2:execl(bin,str[0],str[1],NULL);break;
		case 3:execl(bin,str[0],str[1],str[2],NULL);break;
		case 4:execl(bin,str[0],str[1],str[2],str[3],NULL);break;		}
	printf("%d\t",x);
	//if execl fails prints the message in the requirements
	printf("%s: Command not found.\n",*str[0]);
	exit(0);
}
//parent will just wait for child to die and return
wait();
return;
}//end of do_task



/*
 * Function: parse_tokens
 * Parameter(s): str - It's a string entered by the user. 
 * It can be anything the user wishes.
 * Returns: It returns a double char pointer always.
 * Description: This method takes the users input, and splits
 * up the string by the spaces. It also updates the global 
 * count variable which is used to keep track of
 * how many different tokensi entered by user.
*/
char ** parse_tokens(char * str)
{

// Stores the parsed input of user string
char ** parse = (char**)malloc(numOftokens*sizeof(char*));
//temp variable used for hold the value of each token
char * token;
//this resets the global count
count =0;
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
 * Function: nothing
 * Parameter(s): N/A
 * Returns: N/A
 * Description: This method is only called when control+c or
 * control+z is called. This method doesn't do anything because 
 * I didn't want it to do anything when these signals are uses.
 */
void nothing()
{
}//end of nothing
