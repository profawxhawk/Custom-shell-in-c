#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define PINK "\x1b[35m"
#define RESET "\x1b[0m"
char hostname[1024];
char *user;
char cwd_path[1024];
int pid_to_kill=-1;
static int *status; 
int child_pid[200];
int child_pid_counter=0;
void signal_handler(int signal)
{
    if (pid_to_kill != -1)
    {
        kill(pid_to_kill, SIGINT);
    }
}
char *take_input()
{
    char *in_string = malloc(255);
    if (in_string == NULL)
    {
        printf("Unable to read command as no space available in heap \n");
        exit(0);
    }
    else
    {
        int i = 0;
        char chary;
        int count = 1;
        long size = 255;
        chary = getchar();
        while (chary != '\n')
        { // replace with actual enter key detection
            if ((int)chary == -1)
            {
                break;
            }
            in_string[i] = chary;
            i++;
            if (i == 255)
            {
                count++;
                size = count * size;
                in_string = realloc(in_string, size);
            }
            chary = getchar();
        }
        in_string[i] = '\0';
    }
    return in_string;
}
void display_path()
{
    printf(RED "USER:%s@" RESET GREEN "HOST:%s@" RESET PINK "PATH:%s$" RESET, user, hostname, cwd_path);
}
int Parse_input(char *input, char *delimiters, char **output)
{

    output[0] = strtok(input, delimiters);

    int i = 0;

    while ((output[i]) != NULL)
    {
        i++;
        (output[i]) = strtok(NULL, delimiters);
    }
    char *ret_1 = getcwd(cwd_path, 1024);
    if (ret_1 == NULL)
    {
        printf("Couldn't get current path \n");
        exit(1);
    }
    return i;
}
void exec_call(char *argument_list[10000], char *command)
{

    int pid = fork();
    pid_to_kill=pid;
    if (pid == -1)
    {
        printf("Fork error. Unable to create child process \n");
    }
    else if (pid == 0)
    {

        int err = execvp(command, argument_list);
        if (err == -1)
        {
            printf("Unknown command \n");
            exit(0);
        }
        exit(0);
    }
    else if (pid > 0)
    {
        wait(NULL);
        pid_to_kill=-1;
    }
}
int command_check(char *command)
{
    if (strlen(command) > 2)
    {
        if (command[0] == '1' && command[1] == '>')
        {
            return 1;
        }
        if (!strcmp(command, "2>&1"))
        {
            return 3;
        }
        if (command[0] == '2' && command[1] == '>')
        {
            return 2;
        }
       
    }
    if(!strcmp(command,"exit")){
        *status=0;
        return 4;
    }
    return 0;
}
void run_single_command(char *input)
{
    char *args[10000];
    char *argument_list[10000];
    int args_size = Parse_input(input, " ", args);
    char *input_file;
    char *output_file;
    int flagin = -0;
    int flagout = 0;
    int flagapp = 0;
    int counter = 0;
    int in_num = dup(0);
    int out_num = dup(1);
    int check_flag;
    int err_flag = 0;
    for (int i = 0; i < args_size; i++)
    {
        check_flag = command_check(args[i]);
        if(check_flag==4){
            return;
        }
        if (!check_flag)
        {
            if (strcmp(args[i], ">") == 0)
            {
                if (i == args_size - 1)
                {
                    printf("Syntax error for output redirection. Please specify a file to output to \n");
                }
                else
                {
                    flagout = 1;
                    output_file = args[i + 1];
                    i++;
                    if (access(output_file, F_OK) == -1)
                    {
                        FILE *fp = fopen(output_file, "w");
                        fclose(fp);
                    }
                }
            }
            else if (strcmp(args[i], "<") == 0)
            {
                if (i == args_size - 1)
                {
                    printf("Syntax error for input redirection. Please specify a file to input from \n");
                }
                else
                {
                    flagin = 1;
                    input_file = args[i + 1];
                    i++;
                }
            }
            else if (strcmp(args[i], ">>") == 0)
            {
                if (i == args_size - 1)
                {
                    printf("Syntax error for output append. Please specify a file to append to \n");
                }
                else
                {
                    flagapp = 1;
                    output_file = args[i + 1];
                    i++;
                    if (access(output_file, F_OK) == -1)
                    {
                        FILE *fp = fopen(output_file, "w");
                        fclose(fp);
                    }
                }
            }
            else
            {

                argument_list[counter] = args[i];
                counter++;
            }
        }
        else
        {
            if (check_flag == 1 || check_flag == 2)
            {
                char *file_name;
                char **args_in = malloc(2 * sizeof(char *));
                for (int j = 0; j < 2; ++j)
                {
                    args_in[j] = (char *)malloc(100);
                }
                int in_size = Parse_input(args[i], ">", args_in);
                file_name=args_in[1];
                flagout = 1;
                output_file = file_name;
                if (access(output_file, F_OK) == -1)
                {
                    FILE *fp = fopen(output_file, "w");
                    fclose(fp);
                }
                if (check_flag == 2)
                {
                    err_flag = 1;
                }
                //free(file_name);
                // free(args_in[0]);
                // free(args_in[1]);
                // free(args_in);
            }
            else
            {
                dup2(1, 2);
            }
        }
    }
    FILE *fin = NULL, *fout = NULL, *fapp = NULL;
    if (flagin == 1)
    {
        fin = fopen(input_file, "r");
        if (fin == NULL)
        {
            printf("Error in opening file for input. Please enter valid input file \n");
            return;
        }
        else
        {
            dup2(fileno(fin), 0);
        }
    }
    if (flagout == 1)
    {
        fout = fopen(output_file, "w");
        if (fout == NULL)
        {
            printf("Error in opening file for output. Please enter valid output file \n");
            return;
        }
        else
        {
             if(err_flag==1){
                dup2(fileno(fout), 2);
            }
            else{
            dup2(fileno(fout), 1);
            }
        }
    }
    if (flagapp == 1)
    {
        fapp = fopen(output_file, "a");
        if (fapp == NULL)
        {
            printf("Error in opening file for appending. Please enter valid file \n");
            return;
        }
        else
        {
            dup2(fileno(fapp), 1);
        }
    }
    if (strcmp(args[0], "cd"))
    {
        exec_call(argument_list, args[0]);
        if (fin)
        {
            dup2(in_num, 0);
            fclose(fin);
        }
        if (fout)
        {
            dup2(out_num, 1);
            fclose(fout);
        }
        if (fapp)
        {
            dup2(out_num, 1);
            fclose(fapp);
        }
    }
}
void pipe_input(char **input, int size)
{
    int pid;
    int fd[2];
    int ret;
    int fd_read;
    for (int i = 0; i < size; i++)
    {
        
        ret = pipe(fd);
        if (ret == -1)
        {
            printf("Unable to create pipe \n");
            return;
        }
        pid = fork();
        if (pid == 0)
        {
            close(0);
            dup(fd_read);
            if (*status == 0)
            {
               exit(1);
            }
            if (i != (size - 1))
            {
                close(1);
                dup(fd[1]);
            }
            close(fd[0]);
            run_single_command(*(input + i));
            exit(1);
        }
        else if (pid > 0)
        {
            (child_pid[child_pid_counter])=pid;
            child_pid_counter++;
            if (*status == 0)
            {
                return;
            }
            close(fd[1]);
            fd_read = fd[0];
        }
        else
        {

            printf("Unable to fork \n");
        }
    }
}
void start_shell()
{
    *status = 1;
    signal(SIGINT, signal_handler);
    do
    {

        display_path();
        int pipe_req = 0;
        char *input_comand = malloc(255);
        input_comand = take_input();
        if (input_comand[0] == 0)
        {
            continue;
        }
        char *args[10000];
        int args_size = Parse_input(input_comand, "|", args);
        child_pid_counter=0;
        if (args_size > 1)
        {
            pipe_req = 1;
            pipe_input(args, args_size);
            for(int j=0;j<child_pid_counter;j++){
                    waitpid(child_pid[j], NULL, 0);
            }
        }
        else
        {
            run_single_command(*args);
        }
        free(input_comand);
    } while (*status);
}
int initialise_var()
{
    user = getenv("USER");
    if (user == NULL)
    {
        printf("Couldn't load user \n");
        exit(1);
    }
    int ret = gethostname(hostname, 1024);
    if (ret == -1)
    {
        printf("Couldn't load host \n");
        exit(1);
    }
    char *ret_1 = getcwd(cwd_path, 1024);
    if (ret_1 == NULL)
    {
        printf("Couldn't get current path \n");
        exit(1);
    }
}
int main()
{
    status= mmap(NULL, sizeof *status, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    initialise_var();
    start_shell();
}