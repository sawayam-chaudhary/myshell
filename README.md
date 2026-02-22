# UNIX SHELL IN C
- A functional, lightweight Unix shell written in C that replicates core OS behaviors including process management, multiple pipeline handling and signal handling.

## Features : 
- Process creation : Execute process using fork() and exec()
- Command parsing : Handle input and arguements
- Background execution & : support for running processes in background
- Built-in commands : cd and pwd
- Piping : Support inter-proccess communication
- File redirection : Support redirecting input/output from a file
- Signal handling : Custom handling for SIGCHLD to reap zombie processes and handling for SIGINT (Ctrl + C) and SIGTSTP (Ctrl + Z) to execute different response from both main shell and its child.
- JOB control : Built in Job command to list active processes. Fg and Bg commands to move job between foreground and background

## How to run :
- compile : gcc -o msh shell.c
- run the shell : ./msh

## Limitations :
- Command buffer is limited to 1024 characters.
- Maximum of 100 concurrent jobs.


