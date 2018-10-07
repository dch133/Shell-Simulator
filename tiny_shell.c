#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <syscall.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <time.h>
//definitions for clone()
#define _GNU_SOURCE
#include <linux/sched.h>
#include <sched.h>
#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */

// Some Colors to customise the shell
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
 

int my_fork(char* line)
{
  pid_t pid;

  if ((pid = fork()) == -1)
  {
    perror(RED"fork error"RESET);
    return -1;
  }
  else if (pid == 0) //child runs
  {
    if (execl("/bin/sh", "/bin/sh", "-c", line, (char *)NULL) ==-1) //execute the line
    {
      perror(RED"execl() failure!\n"RESET);
      EXIT_FAILURE;
    }
    exit(0);
  }
  else if (pid > 0) //parent runs
  {
    wait(NULL); //NULL for waiting for any child to finish
  }

  return 0;
}


int my_vfork(char* line)
{
  pid_t pid;
  if ((pid = vfork()) == -1)
  {
    perror(RED"vfork error"RESET);
    return -1;
  }
  else if (pid == 0) //child runs
  {
    if (execl("/bin/sh", "/bin/sh", "-c", line, (char *)NULL) ==-1) //execute the line
    {
      perror(RED"execl() failure!\n"RESET);
      exit(EXIT_FAILURE);
    }
    exit(0);
  }
  else if (pid > 0) //parent runs
  {
    wait(NULL); //NULL for waiting for any child to finish
  }
  return 0;
}

/*start up function for cloned child*/
static int childFunc(char *line) 
{
  if ( execl("/bin/sh", "/bin/sh", "-c", line, (char *)NULL) == -1) //execute the line
  {
    perror(RED"execl() failure!\n"RESET);
    exit(EXIT_FAILURE);
  }
  return 0;
}

int my_clone(char* line)
{
  char *stack;                    /* Start of stack buffer */
  char *stackTop;                 /* End of stack buffer */
  pid_t pid;

  /* Allocate stack for child */
  stack = malloc(STACK_SIZE);
  if (stack == NULL)
    perror("stack malloc error");
  stackTop = stack + STACK_SIZE;  /* Assume stack grows downward */

  /* Create child; child commences execution in childFunc() */
  if (clone(childFunc, stackTop, CLONE_FS|SIGCHLD, line) == -1)
    perror(RED"clone() error"RESET);

  wait(NULL);    /* Wait for child */
  free(stack);
  return 0; 
}

static char* fifo_name;
static char* pipe_end;

int my_pipe(char* line)
{
  int fd0,fd1;

  // make fifo file if it doesn't already exist 
  if (access(fifo_name, F_OK) == -1)
  {
    if (mkfifo(fifo_name, 0666) == -1)
    { // mkfifo(<pathname>, <permission>)
      perror(RED"mkfifo error"RESET);
      _exit(EXIT_FAILURE);
    }   
  }
  //backup of original file descriptors
  int stdin_copy = dup(0);
  int stdout_copy = dup(1);

  if (atoi(pipe_end) == 1)  //read end of PIPE
  { 
    close(1); //close stdout
    if (fd1 = open(fifo_name, O_WRONLY) == -1) //open FIFO and write-end
    {
      perror("Open WRITE fd failed");
    }
    my_fork(line); //execute command
  }
  if (atoi(pipe_end) == 0)  //read end of PIPE
  { 
    close(0);
    if (fd0 = open(fifo_name, O_RDONLY) == -1)
    {
      perror("Open READ fd failed");
    } 
    my_fork(line); //execute command
  } 

  //close FIFO and reopen stdin and stdout
  dup2(stdin_copy, 0);
  dup2(stdout_copy, 1);
  close(stdin_copy);
  close(stdout_copy);
  return 0;
}

int my_system(char* line)
{
  int status;
  #ifdef FORK
    status = my_fork(line);
  #elif VFORK
    status = my_vfork(line);
  #elif CLONE
    status = my_clone(line);
  #elif PIPE
    status = my_pipe(line);
  #else 
    status = system(line);
  #endif
  return status;
}

int main(int argc, char *argv[])
{
  char* line;
  int input_size = 2000;
  char* exit = "exit\n";
  
  //SIG_IGN - Ignore kill and ctrl-c if called from parent
  //signal(SIGTERM, SIG_IGN); //kill
  //signal(SIGINT, SIG_IGN); //ctrl -c
 
  #ifdef PIPE //check for correct format of input argc when using FIFO
  if(argc > 1)
  {
    if (argc == 3 && (atoi(argv[2]) == 0 || atoi(argv[2]) == 1))
    {
      fifo_name = (char*) malloc(512*sizeof(char));
      pipe_end = (char*) malloc(512*sizeof(char));
      strcpy(fifo_name, argv[1]);
      strcpy(pipe_end, argv[2]);
    }
    else
    {
      printf(RED"Wrong format of input for FIFO\n"RESET);
      _exit(1);
    }
  }
  #endif

  while (1) //run the shell
  {
    printf(BOLDYELLOW "tshell" RESET ": "BOLDWHITE); 
    fflush(stdout);
    line = (char*) malloc(input_size*sizeof(char));
    line = fgets(line, input_size*sizeof(char), stdin); //get a line from input
    
    if (sizeof(line) > 1)
    {     
      if(feof(stdin) || strcmp(line,exit) == 0) _exit(0);
      else 
      { /*Commented out the code to see duration of execution*/
        // struct timespec tstart={0,0}, tend={0,0};
        // clock_gettime(CLOCK_MONOTONIC, &tstart);
        if (my_system(line) != 0) perror("my_system() execute error");
        // clock_gettime(CLOCK_MONOTONIC, &tend);
        // printf(BOLDMAGENTA"DURATION: %ld nanoseconds\n",
        // (tend.tv_nsec - tstart.tv_nsec) );
      }
    }
    free(line);
    fflush(stdout);
  }
}
