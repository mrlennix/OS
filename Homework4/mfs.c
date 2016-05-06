/*
 * Name: Emmanuel Lennix
 * ID #: 1001104990
 * Programming Assignment 4
 * Description: A file system that allows users to store 5MB worth of files
 * 
 */
#include<unistd.h>
#include<sys/mman.h>
#include<time.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>

//max number of characters
#define max_char 100

//number of blocks in the file system
#define NumOfBlocks 1280

//max number of files that can be stored in the file system
#define MaxNumOfFiles 128

/*
 * Structure: FileInfo
 * Attributes:
 * name - name of the file 
 * size - size of the file
 * text - text within the file
 * blocktaken - number of blocks the file has taken root in
 * block - array of each block index the file resides in
 * spaceNblock - array consisting of the space taken from each block 
 */

typedef struct FileInfo
{
	char name[255];
	int size;
	char* text;
	char  date[20];
	int blocktaken;
	int* block;
	int* spaceNblock;
}FileInfo;

/*
 * Structure: Block
 * Attributes:
 * a_space - the available space within the block
 * NumOfFiles - number of files the reside in the block
 */
typedef struct Block
{
	int a_space;
	int NumOfFile;
		
	
}Block; 

//GLOBAL VARIABLES

//blocks used in the file system to keep track of space
Block* block;

//stores the array of files in the file system
FileInfo* files[MaxNumOfFiles];

//current number of files in the system
int NumOfFiles=0;

//methods defined
char ** parse_tokens( char* );

void supported_commands( char ** );

void put( char * );

void get(char *, char *);

void del(int );

void clear();

int df();

void list();

//int fit();

int getIndex(char * );

int main( void ) 
{
	int x;	
	block = (Block *)malloc(NumOfBlocks*sizeof(Block) );
	for(x = 0; x< NumOfBlocks; x++)
	{
		block[x].a_space = 4096;
		block[x].NumOfFile = 0;
	}

	char data[max_char];

	char ** command;

	printf("Starting the mavs file system!\n");

        while(1)
        {
                
                do
                {
                        printf("mfs> ");

			fgets(data,max_char,stdin);
		}
		while(!strcmp(data,"\n"));
		
		data[strlen(data)-1]='\0';

		command=parse_tokens(data);

		if(!strcmp(command[0],"quit"))break;

		supported_commands(command);
	}

	return 0 ;
}




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

	while(token != NULL)
        {
                parse[count]=token;

                token=strtok(NULL," ");

                count++;
        }

	return parse;
}//end of parse_token


/*
 * Function: supported_commands
 * Parameters: command - the parsed user input taken from the user
 * Return: N/A
 * Description: Takes the parsed user input and uses it to determine which task
 * should be done for the user.
 */
void supported_commands(char ** command)
{
	if( !strcmp(command[0],"df") )
	{
		printf("%d bytes free.\n",df() );

		return;
	}

	if( !strcmp(command[0],"clear") )
	{
		clear();

		return;
	}

	if( !strcmp(command[0],"list") )
        {
        	list();

        	return;
        }

	if( !strcmp(command[0],"put") )
        {
		put(command[1]);

		return;
	}

	if( !strcmp(command[0],"get") && command[2]!=NULL  )
	{
		get( command[1],command[2] );

		return;
	}

	else if( !strcmp(command[0],"get") )
	{
		get( command[1],NULL );

		return;
	}

	 if( !strcmp(command[0],"del") )
	{
		if( command[1] == NULL)return;

		int index;

		index = getIndex(command[1]);

		if(index == -99 )
		{
			printf("error: File not found.\n");
			return;
		}

		del(index);

		return;
	}	

	printf("error: Command not found.\n");
}

/*
 * Function: df
 * Parameters: N/A
 * Returns: Integer - number of free space in the system
 * Description: Adds the available space in each block and returns 
 * the free space.
 */
int df()
{
	int x,freespace = 0;
	for(x=0; x <NumOfBlocks;x++)
	{
		freespace+=block[x].a_space;
	}

	return freespace;
}

/*
 * Function: Clear
 * Parameters:N/A
 * Return:N/A
 * Description: Clears the screen!
 */
void clear()
{
	system ("clear");
}

/*
 * Function: list
 * Parameters:N/A
 * Return: N/A
 * Description: Lists all the files in the file system
 * so the user can see which files exist
 */
void list()
{	
	int y,count=0;
	
	for(y=0;y<MaxNumOfFiles;y++)
	{			
		if( (*files[y]).name!=NULL )
		{
			printf("%d %s %s\n",(*files[y]).size,(*files[y]).date,(*files[y]).name);

			count++;
		}
	}
		
	if(count == 0) printf("list: No files found.\n");
}


/*
 * Function: Put
 * Parameters: fname - name of the file the user wants to put in the file system.
 * Return: N/A
 * Description: This method puts a file into the file system by keeping it in memory
 * and storing it into the structure.
 */
void put( char * fname )
{
	if(NumOfFiles>MaxNumOfFiles)
	{
		printf("error: File limit reached.\n");
	
		return;
	}

	struct stat sbuf;
	
	int memfd;
	
	void * pmap;

	int file = open( fname,O_RDONLY);

	if(file == -1 )
	{
		printf("error: File not found\n");
		
		return;
	}

	if( stat(fname,&sbuf) == -1)
	{
		printf("stat");

		return;
	}
	
	if(sbuf.st_size >= 131072 )
	{
		printf("File to large.\n");

		munmap(pmap,sbuf.st_size);

		return;
	}	
	
	if(sbuf.st_size > df() )
	{
		printf("error: Not enough free space\n");

		munmap(pmap,sbuf.st_size);

		return;
	}

	memfd = open(fname,O_RDWR);

	if(memfd == -1)
	{
		return;
	}
		
	pmap = mmap(0,sbuf.st_size,PROT_READ | PROT_WRITE,MAP_SHARED,memfd,0);

	int index = fit();

	struct tm* tm;

	time_t t= time(NULL);

	tm = localtime ( &t);

	FileInfo* f = (FileInfo*)calloc(1,sizeof(FileInfo) );

        f->text = pmap;

        strcpy(f->name,fname);

	f->size = sbuf.st_size;

        strftime (f->date,sizeof( f->date ),"%b %e %k:%M",tm);

	NumOfFiles++;

	f->blocktaken=0;

	int tsize = sbuf.st_size;

	int x,count=0;

	for(x=0;x<NumOfBlocks;x++)
	{
		if(block[x].a_space <= 0)continue;

		if( tsize>block[x].a_space )
		{			
			tsize-=block[x].a_space;

			f->blocktaken+=1;

			block[x].NumOfFile+=1;

			f->block = realloc( f->block,sizeof(f->block)*f->blocktaken );

			f->spaceNblock = realloc( f->spaceNblock,sizeof(f->spaceNblock)*f->blocktaken );

			f->spaceNblock[count]=block[x].a_space;

			f->block[ f->blocktaken - 1 ]=x;

			block[x].a_space=0;
		}
		else
		{
			block[x].a_space-=tsize;

			f->blocktaken+=1;

                        block[x].NumOfFile+=1;

                        f->block = realloc( f->block,sizeof(f->block)*f->blocktaken);

			f->spaceNblock = realloc( f->spaceNblock,sizeof(f->spaceNblock)*f->blocktaken );

                        f->spaceNblock[count]=tsize;

			f->block[f->blocktaken-1]=x;

			tsize = 0;				
		}

		count++;

		if(tsize == 0 )break;

	}
	                 
	files[index] = f;
}

/*
 * Function: get
 * Parameters: fname - Name of the file the user wishes to get
 * newname - name the user wants the file changed to so it doesn't conflict with the original.
 * Return: N/A
 * Description: Gets the file selected by the user and puts a new name and replaces it with a new name,
 * if the name is given.
 */
void get(char * fname, char * newname)
{
	int index;

	char * name;

	if(newname != NULL)name = newname;

	else name = fname;

	index  = getIndex(fname);

	if(index == -99)
	{
		printf("error: File not found.\n");

		return;;
	}

	FILE *fp = fopen( name , "w" );

	if(fp == NULL)
	{
		printf("error: File not found.\n");

		return;
	}
	
	fprintf(fp,files[index]->text);
	
	fclose( fp );

}

/*
 * Function: fit
 * Parameters: N/A
 * Returns: Intger - position of an available spot for the file in the file array;
 * Description: Searches the files for a null spot (availiable spot) in the files array
 * which will later be used for a new file
 */
int fit()
{
	int x;

	for(x =0;x<NumOfFiles;x++)
	{
		if(files[x]==NULL)return x;
	}
}

/*
 * Function: del
 * Parameters - index - the index of the files that is inside of the files array
 * Return: N/A
 * Description: Puts memory back in the blocks that it was taken from and frees
 * up the existing file from memory so it coult be used for another file
 */
void del(int index )
{
	int x,b;

	for(x=0;x<files[index]->blocktaken;x++)
	{
		b = files[index]->block[x];

		block[b].a_space+=files[index]->spaceNblock[x];

		block[b].NumOfFile--;
	}
	free(files[index]->spaceNblock);

	free(files[index]->block);

	munmap(files[index]->text,files[index]->size);

	free(files[index]);

	files[index]=NULL;

	NumOfFiles--;
}

/*
 * Funtion: getIndex
 * Parameters: fname - file name provided by the user.
 * Return: 
 * Integer - which is the index that the file is stored at in the files array
 *  -99 - is return when a file doesn't exist in the files array
 * Description: Takes the file name and uses it to figure out what the position of the
 * file is in the files array
 */
int getIndex(char * fname)
{
	int x;

        for(x=0;x<MaxNumOfFiles;x++)
        {

                if(files[x]->name == NULL)continue;

                if( strcmp(files[x]->name,fname)==0 )
                {
                        return x;                        
                }
        }

	return -99;
}
