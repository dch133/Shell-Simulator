# Tiny-Shell
Small Shell that uses Fork, VFork, Clone, and Piping to execute system commands

By Daniel Chernis

# REPORT
________________________________________________________________________________
Compiling and Running the tiny Shell
________________________________________________________________________________
To compile for execution options FORK, VFORK, CLONE, FIFO, SYSTEM:
Run respectively 'make fork'; 'make vfork'; 'make clone'; 'make pipe'; 'make all'

All except for FIFO do not take command line arguments.

Running tiny-shell (generally): 
> ./tshell

Running tiny-shell (FIFO): 
> ./tshell  <PIPE_NAME> <MODE>		
 
Where MODE WRITING = '1' and MODE READING = '0' 

*Note on FIFO (First in First Out - Named Pipe): Need 2 terminals (one for reading and one for writing) 

Commands from a file can be redirected into tshell as follows:
> ./tshell < input.txt

There is also sample 'hello world' C program that you can try to run on the tiny shell:

 > make hello 
 
 Compiles the sample C program 
 
 > make hello_seg
 
 Compiles the crashing version of the above 


________________________________________________________________________________


Average Execution Time Test Results on commands {'ls'; 'pwd'; 'echo hello'} in order:

fork():		5765011 nanosec; 1862117 nanosec; 543068 nanosec

vfork():	1396076 nanosec; 0387629 nanosec; 346691 nanosec

clone():	3177079 nanosec; 0536986 nanosec; 399556 nanosec

system():	2046151 nanosec; 0945451 nanosec; 897779 nanosec

---------------------------------------------------------------------------------

This is an average time. At different execution iterations, some of these outperform others for specific commands.

vfork() and system() seem to be the fastest in general. vfork() is more space efficient compared to fork().

With the vfork() semantics, the child is guaranteed to execute before the parent after the vfork().

At vfork() execution, the child gets its own file descriptors.

clone() is somewhere in the middle, but unlike others, is able to affect the directory of tshell with 'cd'.

See picture attached to this project that demonstates how 'cd' command successfully executes. 

_________________________________________________________________________________
Analysis:
_________________________________________________________________________________
_________________________________________________________________________________

1) What happens to your shell with working/failing commands?
---------------------------------------------------------------------------------
When commands work, they execute properly in the subshell displaying the result to stdout.
Then we go back to the shell to get more input from stdin.
Similarly, when commands fail, an error message is displayed by bin/sh stating that the command is invalid.
Then we go back to the shell to get more input from stdin.
_________________________________________________________________________________

2) Does your shell crash when a command crashes?
---------------------------------------------------------------------------------
The shell will not crash when a command crashes. But if you do ctrl-c while the child process is
executing the command, tshell will exit.
_________________________________________________________________________________

3) Does my_system() behave the same way as the implementation that used the system() from Linux? 
What are the diffences and why?
---------------------------------------------------------------------------------
system() calls out to sh to handle your command line, so you can get wildcard expansion, etc.
exec() replace the current process image with a new process image. 
That is why we use fork,vfork,clone to replicate this functionality. 
Nevertheless, besides clone(), the others mentioned only execute 'cd' within the child process until termination.
The parent process (tshell) doesn't remember the 'cd' command and remains in its directory.
_________________________________________________________________________________

4) How would you set the flags if you want to execute a command like cd (change directory) in the child
and parent be affected by it?
---------------------------------------------------------------------------------
Flags used: CLONE_FS

If CLONE_FS is set, the caller and the child process share the
              same filesystem information.  This includes the root of the
              filesystem, the current working directory, and the umask.  Any
              call to chroot(2), chdir(2), or umask(2) performed by the
              calling process or the child process also affects the other
              process.
_________________________________________________________________________________

5) If you want subsequent commands executed by the parent to run in a
specific directory that was changed into by a previous cd command, what should the cloning do?
That is, what kind of sharing should be performed while cloning?
---------------------------------------------------------------------------------
Since the version of exec() used for tshell is execl() which analyses the whole line at once,
separating commands with ';' allows for consecutive execution. e.g. cd..; ls
_________________________________________________________________________________

6) About Edge Cases
---------------------------------------------------------------------------------

Overall, tshell works on general cases and reports most errors like when mkfifo() fails
or when fork, vfork, and clone fail to create child processes.

The argument validity is tested only when 'make pipe' was run.

If the fifo pipe name given does not link to a file, the pipe will be created in the current directory of tshell.c

There are more potential edges cases not covered.

For 'tr' command e.g. tr 'ABC' '123', the only why to exit the execution is ctrl-c which also exits tshell.

There is wrong formatting of tshell when reading an EOF after finishing
to run and execute the file line by line.
_________________________________________________________________________________

7) About FIFO (named pipe)
----------------------------------------------------------------------------------

The behavior of FIFO is shown in the pictures attached to this project. 
The tested (and working) commands were {WRITE,READ}: {ls, wc} and {echo message, cat}

The FIFO was not exhaustively tested.

If you READ from FIFO 1st, the 'reader' tshell will wait for 'writer' tshell to write to FIFO
_________________________________________________________________________________

7) About Forking and VForking
----------------------------------------------------------------------------------

When fork() or vfork has an error (returns -1) we exit the child and go back to the parent.

The tshell waits for the child using "wait(NULL)". This way, it waits for any children (easier to conceptualize)  






