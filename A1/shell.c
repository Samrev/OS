#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <signal.h>

#define MAXLIST 100

void print_prompt1(void);
char *read_cmd(void);
void print_curr_dir(void);
void cmd_history();
void ps_history();

int32_t pidlist[32768];
int32_t it =0;
char history[5][256];

void print_curr_dir(void){

    char buffer[FILENAME_MAX];
    getcwd( buffer, FILENAME_MAX );
    printf("%s",buffer);

}
void welcome(void)
{
        print_curr_dir();
    printf("~$ ");
}

char *read_cmd(void)
{
    char buf[1024];
    char *ptr = NULL;

    while(fgets(buf, 1024, stdin))
    {
        int buflen = strlen(buf);

        if(!ptr)
        {
            ptr = malloc(buflen+1);
        }

        if(!ptr)
        {
            fprintf(stderr, "error: failed to alloc buffer: %s\n", strerror(errno));
            return NULL;
        }


        if(buf[buflen-1] == '\n')
        {
            buf[buflen-1] = '\0';
            strcpy(ptr, buf);
            return ptr;
        }
        else{
                fprintf(stderr, "Input exceeds the limit: %s\n", strerror(errno));
                return NULL;
        }
    }

    return ptr;
}



void replacedollars(char **parsed){
    for(int i = 1 ; i<MAXLIST ; i++){
        char *sep[2];
        if(parsed[i]==NULL) break;
        if(strncmp("$",parsed[i],1)==0){
            for(int j = 0 ; j<2; j++){
                sep[j] = strsep(&parsed[i], "$");
            }
            parsed[i] = getenv(sep[1]);   
        }
    }
}

void Execute(char** parsed)
{
    // Forking a child

    
    uint8_t isBackgorund =0;
    int status;
    if(parsed[0][0] == '&'){
        isBackgorund =1;
        *parsed[0]++;
    }
    replacedollars(parsed);


    pid_t pid = fork(); 
    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } 
    else if (pid == 0) 
    {
        if(strcmp(parsed[0],"cmd_history")==0)
        {
            cmd_history();
        }

        else if(strcmp(parsed[0], "ps_history") == 0)
        {
            ps_history();
        }
        else if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } 
    else 
    {
        /*Backgorund process*/
        if(isBackgorund == 1)
        {
            waitpid(pid, &status , WNOHANG);
            pidlist[it++] = pid;
        }

        /* waiting for child to terminate */
        else
        {
            wait(NULL); 
            pidlist[it++] = pid;
        }
        return;
    }
}


void ExecutePiped(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int PipeDesc[2]; 
    pid_t p1, p2;
  
    if (pipe(PipeDesc) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }


    replacedollars(parsed);
    replacedollars(parsedpipe);


    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }
  
    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(PipeDesc[0]);
        dup2(PipeDesc[1], STDOUT_FILENO);
        close(PipeDesc[1]);
        if(strcmp(parsed[0],"cmd_history")==0)
        {
            cmd_history();
            exit(0);
        }

        else if(strcmp(parsed[0], "ps_history") == 0)
        {
            ps_history();
            exit(0);
        }

        else if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } else {
        // Parent executing
        pidlist[it++] = p1;
        p2 = fork();
  
        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }
  
        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            close(PipeDesc[1]);
            dup2(PipeDesc[0], STDIN_FILENO);
            close(PipeDesc[0]);
            if(strcmp(parsedpipe[0],"cmd_history")==0)
            {
                cmd_history();
                exit(0);
            }

            else if(strcmp(parsedpipe[0], "ps_history") == 0)
            {
                ps_history();
                exit(0);
            }
            else if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        } 
        else {
            // parent executing, waiting for two children
            pidlist[it++] = p2;
            close(PipeDesc[0]);
            close(PipeDesc[1]);
            wait(NULL);
            wait(NULL);
        }
    }
}

void removespace(char* str, char** parsed)
{
    int i;
  
    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");
  
        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}


int checkpipe(char* str, char** strpiped)
{
    int i;
    for (i = 0; i < 2; i++) {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }
  
    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
}

int checkequalsign(char* str, char** variable)
{
    int i;
    for (i = 0; i < 2; i++) {
        variable[i] = strsep(&str, "=");
        if (variable[i] == NULL)
            break;
    }
  
    if (variable[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
} 


  
int parsecmd(char* str, char** parsed, char** parsedpipe)
{
  
    char* strpiped[2];
    char* variable[2];

    int piped = 0, env=0;
  
    piped = checkpipe(str, strpiped);
    env = checkequalsign(str , variable);
    if (piped) 
    {
        removespace(strpiped[0], parsed);
        removespace(strpiped[1], parsedpipe);
        return 3;
  
    } 

    else if(env)
    {
        parsed[0]=variable[0];
        parsed[1]=variable[1];
        return 2;
    }

    else
    {
        removespace(str, parsed);
        return 1;
    }
}

void cmd_history(){
    for(int i =0 ; i<=4;i++)
    {
        if(strlen(history[i])==0)
        {
            break;
        }
        printf("%s\n",history[i]);
    }
}
void ps_history(){
    for(int i=0 ; i<it; i++)
    {
        int status;
        pid_t return_pid = kill(pidlist[i], 0);
        if (return_pid == 0) 
        {
            /* error */
            printf("%d RUNNING\n",pidlist[i]);
        } 
        else 
        {
            /* child is still running */
            printf("%d STOPPED\n",pidlist[i]);
        }
    }
}


struct sigaction old_action;

void sigint_handler(int sig_no)
{
    printf("CTRL-C pressed\n");
    sigaction(SIGINT, &old_action, NULL);
    kill(0, SIGINT);
}


void handle_sigchld(int sig) {
    pid_t pid;
    while ((pid = waitpid((pid_t)(-1), NULL, WNOHANG)) > 0) {}
    return;
}

int main(int argc, char **argv)
{

    /*handles CTRL+C*/
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &sigint_handler;
    sigaction(SIGINT, &action, &old_action);
    

    /*handling zombies*/
    struct sigaction sa = {0}; 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sa.sa_handler = handle_sigchld;
    sigaction(SIGCHLD, &sa, NULL);


    char *cmd , *parsedArgs[MAXLIST];

    char* parsedArgsPiped[MAXLIST];
    int type = 0,i=0;
    for(int i =0; i<5;i++){
        strcpy(history[i] , "");
    }

    do
    {
        int status;
        pid_t killpid;

        //kill zombie process
        while((killpid = waitpid(-1,&status,WNOHANG))>0){
            
        }


        welcome();

        cmd = read_cmd();
        char *copy_cmd = malloc(strlen(cmd)+1);
        strcpy(copy_cmd,cmd);
            
        if(!cmd)
        {
            exit(EXIT_SUCCESS);
        }

        if(cmd[0] == '\0' || strcmp(cmd, "\n") == 0)
        {
            // free(cmd);
            continue;
        }

        if(strcmp(cmd, "exit\0") == 0)
        {
            free(cmd);
            break;
        }

        type = parsecmd(cmd,
        parsedArgs, parsedArgsPiped);

        if (type == 1)
        {   
            
            
            Execute(parsedArgs);

            for(int i = 4 ; i>=1;i--)
            {
                strcpy(history[i],history[i-1]);
            }
            
            strcpy(history[0],copy_cmd);
        }
        if(type == 2){
            // printf("%s\n",copy_cmd);
            setenv(parsedArgs[0],parsedArgs[1],1);
            continue;
        }
        if (type == 3)
        {

            ExecutePiped(parsedArgs, parsedArgsPiped);
        }


        // free(cmd);

    } while(1);

    exit(EXIT_SUCCESS);
}