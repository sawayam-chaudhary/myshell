#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    pid_t shell_pid = getpid();
    setpgid(shell_pid,shell_pid);
    tcsetpgrp(STDIN_FILENO,shell_pid);

    while(1){
        int status;
        while(waitpid(-1,&status,WNOHANG)>0){};
        printf("msh>");
        fflush(stdout);

        char str[100];
        if(fgets(str,sizeof(str),stdin)==NULL){
            break;
        }
        str[strcspn(str,"\n")]=0;
        char *myargv[20];
        int i=0;
        char *token = strtok(str," ");
        while(token != NULL){
            myargv[i] = token;
            token=strtok(NULL," ");
            i++;
        }
        myargv[i]=NULL;
        if(myargv[0]==NULL){
            continue;
        }
        if(strcmp(str,"exit")==0){
            break;
        }
        
        if(strcmp(myargv[0],"cd")==0){
            char *path;
            if(myargv[1]==NULL){
                path=getenv("HOME");
                if(path==NULL){
                    printf("home not set");
                    continue;
                }
            }
                else{
                    path=myargv[1];
                }
                if(chdir(path)!=0){
                    perror("cd");
                }
                continue;
            
        }
        if(strcmp(myargv[0],"pwd")==0){
            char cwd[1024];
            if(getcwd(cwd,sizeof(cwd))){
                printf("%s\n",cwd);
            }
            else 
                perror("pwd");
            continue;
        }

        int background =0;
        if(i>0 && strcmp(myargv[i-1],"&")==0){
            myargv[i-1]=NULL;
            background=1;
        }

        int pipe_idx = -1;
        for(int j=0; myargv[j]; j++){
            if(strcmp(myargv[j],"|")==0){
                pipe_idx =j;
                break;
            }
        }
        if(pipe_idx != -1){
            myargv[pipe_idx]=NULL;
            char **left = myargv;
            char **right = &myargv[pipe_idx + 1];
            int fd[2];
            pipe(fd);
            pid_t pid1 = fork();
            if(pid1==0){
                setpgid(0,0);
                
                dup2(fd[1],STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                signal(SIGINT,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                execvp(left[0],left);
                perror("exec");
                exit(1);
            }

            pid_t pid2=fork();
            if(pid2==0){
                setpgid(0,pid1);

                dup2(fd[0],STDIN_FILENO);
                close(fd[0]);
                close(fd[1]);
                signal(SIGINT,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                execvp(right[0],right);
                perror("exec");
                exit(1);
            }
            close(fd[0]);
            close(fd[1]);
            setpgid(pid1,pid1);
            setpgid(pid2,pid1);
            tcsetpgrp(STDIN_FILENO,pid1);
            waitpid(pid1,NULL,0);
            waitpid(pid2,NULL,0);
            tcsetpgrp(STDIN_FILENO,shell_pid);
            continue;
        }

        int redir_idx = -1;
       int  redir_type = 0;
        for(int i=0;myargv[i];i++){
            if(strcmp(myargv[i],">")==0){
                redir_idx=i;
                redir_type=1;
                break;
            }
            if(strcmp(myargv[i],"<")==0){
                redir_idx=i;
                redir_type=2;
                break;
            }
            if(strcmp(myargv[i],">>")==0){
                redir_idx=i;
                redir_type=3;
                break;
            }
        }
        if(redir_idx!= -1){
            myargv[redir_idx]=NULL;
            char *filename = myargv[redir_idx+1];
            int fd;
            if(redir_type==1){
                fd=open(filename,O_WRONLY|O_CREAT|O_TRUNC, 0644);
            }
            else if(redir_type==2){
                fd=open(filename,O_RDONLY);
            } else {
                fd=open(filename,O_WRONLY | O_CREAT | O_APPEND, 0644);
            }
            if(fd<0){
                perror("fd");
                continue;
            }
            pid_t pid=fork();
            if(pid==0){
                setpgid(0,0);
                if(redir_type==1 || redir_type==3){
                    dup2(fd,STDOUT_FILENO);
                }
                if(redir_type==2){
                    dup2(fd,STDIN_FILENO);
                }
                close(fd);
                signal(SIGINT,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                execvp(myargv[0],myargv);
                exit(1);
            }
            close(fd);
            setpgid(pid,pid);
            tcsetpgrp(STDIN_FILENO,pid);
            waitpid(pid,NULL,0);
            tcsetpgrp(STDIN_FILENO, shell_pid);
            continue;
        }

        pid_t pid = fork();
        if(pid==0){
            setpgid(0,0);
            signal(SIGINT,SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            execvp(myargv[0],myargv);
            perror("exec");
            exit(1);
        }
        setpgid(pid,pid);
        if(!background){
        tcsetpgrp(STDIN_FILENO,pid);
        waitpid(pid,NULL,0);
        tcsetpgrp(STDIN_FILENO,shell_pid);
        }
    }
}




        











