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
        printf("Unable to read command as no space available in heap");
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
        printf("Couldn't get current path");
        exit(1);
    }
    return i;
}
void run_single_command(char *input)
{
    char *args[10000];
    int args_size = Parse_input(input, " ", args);
   
}
void pipe_input(char **input, int size)
{
    int pid;
    int fd[2];
    int ret;
    for (int i = 0; i < size; i++)
    {
        ret = pipe(fd);
        if (ret == -1)
        {
            printf("Unable to create pipe\n");
            return;
        }
        pid = fork();
        if (pid == 0)
        {
            dup2(fd[0], 0);
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
        }
        else
        {
            printf("Unable to fork\n");
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
        char *input_comand = take_input();
        char *args[10000];
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

    } while (status);
}
int initialise_var()
{
    user = getenv("USER");
    if (user == NULL)
    {
        printf("Couldn't load user");
        exit(1);
    }
    int ret = gethostname(hostname, 1024);
    if (ret == -1)
    {
        printf("Couldn't load host");
        exit(1);
    }
    char *ret_1 = getcwd(cwd_path, 1024);
    if (ret_1 == NULL)
    {
        printf("Couldn't get current path");
        exit(1);
    }
}
int main()
{
    initialise_var();
    start_shell();
}