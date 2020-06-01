#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <syslog.h>

FILE *task_file;
//FILE *out_file;

struct task
{
    int h;
    int m;
    char *command;
    int info;
};
struct task tasks[10];
int main(int argc, char *argv[])
{
    printf("S1");
    if (argc == 3)
    {
        task_file = fopen(argv[1], "r");
        //out_file = fopen(argv[2], "w");
    }
    printf("S2");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    //tasks = malloc(sizeof(struct task));
    int x = 0;
    printf("S3");
    while ((read = getline(&line, &len, task_file)) != -1)
    {
        x++;
        //tasks = realloc(tasks, x * sizeof(struct task));
        char *token = strtok(line, " ");
        int i = 0;
        // loop through the string to extract all other tokens
        while (token != NULL)
        {
            switch (i)
            {
            case 0:
                tasks[i].h = atoi(token);
                break;
            case 1:
                tasks[i].m = atoi(token);
                break;
            case 2:
                tasks[i].command = token;
                break;
            case 3:
                tasks[i].info = atoi(token);
                break;
            }
            token = strtok(NULL, " ");
            i++;
        }
    }
    printf("S4");
    for(int y=0;y<x;y++){
        printf("Task nr: %d, h: %d, m: %d, Comm: %s, Inf: %d \n",y,tasks[y].h,tasks[y].m,tasks[y].command,tasks[y].info);
    }

    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
           we can exit the parent process. */
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);

    /* Open any logs here */
    openlog("MiniCron", LOG_CONS, LOG_CRON);
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0)
    {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0)
    {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* Daemon-specific initialization goes here */
    time_t rawtime;
    struct tm *info;
    /* The Big Loop */
    while (1)
    {
        time(&rawtime);
        info = localtime(&rawtime);
        pid_t pidF;
        pidF = fork();
        if (pidF < 0)
        {
            exit(EXIT_FAILURE);
        }
        else if (pidF == 0)
        {
            execl("/usr/bin/firefox", NULL);
        }
        sleep(20);
    }
    closelog();
    exit(EXIT_SUCCESS);
}
