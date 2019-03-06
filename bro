#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define GREEN "\x1b[32m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"
char hostname[1024];
char *user;
char cwd_path[1024];
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
    printf(GREEN "%s@%s" RESET ":" BLUE "%s" RESET "$", user, hostname, cwd_path);
}
int Parse_input(char *input, char *delimiters, char **output)
{
    output[0] = (char *)malloc(sizeof(char) * 10000);
    output[0] = strtok(input, delimiters);

    int i = 0;

    while ((output[i]) != NULL)
    {
        i++;
        output[i] = (char *)malloc(sizeof(char) * 10000);
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
void exec_call(char **argument_list, char *command)
{

    if (!strcmp(command, ""))
    {
        return;
    }
    int pid = fork();
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
    }
    else if (pid > 0)
    {
        waitpid(pid,NULL,0);
      
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
        if (command[0] == '2' && command[1] == '>')
        {
            return 2;
        }
        if (!strcmp(command, "2>&1 "))
        {
            return 3;
        }
    }
    return 0;
}
void run_single_command(char *input)
{
    char **args = malloc(10000 * sizeof(char *));
    char **argument_list=malloc(10000 * sizeof(char *));
    int args_size = Parse_input(input, " ", args);
    char *input_file  = (char *)malloc(sizeof(char) * 10000);
    char *output_file=(char *) malloc(sizeof(char) * 10000);
   // printf("%s\n",(argument_list[1]));
    int flagin = -0;
    int flagout = 0;
    int flagapp = 0;
    int counter = 0;
    int in_num = dup(0);
    int out_num = dup(1);
    int check_flag;
    int err_flag=0;
    for (int i = 0; i < args_size; i++)
    {
        check_flag=command_check(args[i]);
        if(!check_flag){
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
            argument_list[counter]=(char *)malloc(sizeof(char) * 10000);
            argument_list[counter] = args[i];
            counter++;
        }
        }
        else{
            if(check_flag==1||check_flag==2){
                char *file_name = (char *)malloc(sizeof(char) * 1000);
                char **args_in = malloc(2 * sizeof(char *));
                for (int j = 0; j < 2; ++j)
                {
                    args_in[j] = (char *)malloc(10000 + 1);
                }
            int in_size=Parse_input(args[i],">",args_in);
                strcpy(file_name, args_in[1]);
                flagout = 1;
                output_file = file_name;
                if (access(output_file, F_OK) == -1)
                {
                    FILE *fp = fopen(output_file, "w");
                    fclose(fp);
                }
                if(check_flag==2){
                    err_flag=1;
                }
                free(file_name);
                free(args_in[0]);
                free(args_in[1]);
                free(args_in);
            }
            else{
                dup2(1,2);
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
        exec_call(argument_list, argument_list[0]);
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
    //free(input);
    //  for(int i=0;i<counter;i++){
    //      free(*(argument_list+i));
    //      free((argument_list + i));
    //  }
    // free(argument_list);
    //     for(int k=0;k<args_size;k++){
    //         free(args[k]);
    //     }
    //    free(args);
    free(input_file);
    free(output_file);
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
            dup2(fd_read,0);
            if (i != (size - 1))
            {
                dup2(fd[1], 1); // close and dup gives race conditions so use dup2
            }
            close(fd[0]);
            run_single_command(*(input + i));
            exit(1);
        }
        else if (pid > 0)
        {
            wait(NULL);
            close(fd[1]);
            fd_read=fd[0];
        }
        else
        {
            printf("Unable to fork \n");
        }
    }
    close(fd[0]);
    close(fd[1]);
}
void start_shell()
{
    int status = 1;
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
        char **args = malloc(10000 * sizeof(char *));
        int args_size = Parse_input(input_comand, "|", args);
        if (args_size > 1)
        {
            pipe_req = 1;
            pipe_input(args, args_size);
        }
        else
        {
            run_single_command(*args);
        }
        // for (int i = 1; i < args_size; i++)
        // {
        //     printf("%p\n", (void *)(args + i));
        //     free(*(args + i));
        // }
        // free(args_size);
        free(input_comand);
    } while (status);
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
    initialise_var();
    start_shell();
}