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
        if(strcmp(myargv[0],"exit")==0){
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

        int cmd_count=1;
        for(int j=0; myargv[j]; j++){
            if(strcmp(myargv[j],"|")==0){
                cmd_count++;
                
            }
        }
        if(cmd_count>1){
        char *command[cmd_count][20];
        int cmd_index =0;
        int arg_index=0;
        for(int i=0;myargv[i]!=NULL;i++){
            if(strcmp(myargv[i],"|")==0){
                command[cmd_index][arg_index] = NULL;
                cmd_index++;
                arg_index=0;
            }else{
                command[cmd_index][arg_index]= myargv[i];
                arg_index++;
            }}
        command[cmd_index][arg_index]=NULL;

        int pipes[cmd_count-1][2];
        for(int j=0;j<cmd_count-1;j++){
            pipe(pipes[j]);
        }

        pid_t pgid=0;
        for(int j=0;j<cmd_count;j++){
            
            pid_t pid=fork();
            if(pid==0){
                if(j==0){
                    setpgid(0,0);
                }else setpgid(0,pgid);
            signal(SIGINT,SIG_DFL);
            signal(SIGTSTP,SIG_DFL);

            if(j>0){
                dup2(pipes[j-1][0],STDIN_FILENO);
            }
            if(j<cmd_count-1){
                dup2(pipes[j][1],STDOUT_FILENO);
            }

            for(int k=0;k<cmd_count-1;k++){
            close(pipes[k][0]);
            close(pipes[k][1]);
                }

            execvp(command[j][0],command[j]);
            perror("exec");
            exit(1);
            }
            if(j==0){
                pgid=pid;
            }
            setpgid(pid,pgid);
        }

        for(int j=0;j<cmd_count-1;j++){
                close(pipes[j][0]);
                close(pipes[j][1]);
                }
       if(!background)
        tcsetpgrp(STDIN_FILENO,pgid);
        for(int j=0;j<cmd_count;j++){
                wait(NULL);
        }
        tcsetpgrp(STDIN_FILENO,shell_pid);}
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
            if(!background){
            tcsetpgrp(STDIN_FILENO,pid);
            waitpid(pid,NULL,0);
            tcsetpgrp(STDIN_FILENO, shell_pid);
            }
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




        











