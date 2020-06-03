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
#include <time.h>
#include <signal.h>

FILE *task_file;
//FILE *out_file;

struct task
{
    int h;
    int m;
    char *command;
    int info;
};
struct task *tasks;
int swap(struct task *task1, struct task *task2)
{
    struct task taskTmp = *task1;
    *task1 = *task2;
    *task2 = taskTmp;
    return 0;
}
struct tm *info;
int count = 0;
time_t current_time;
void AlarmHandler(int sig)
{
    signal(SIGALRM, SIG_IGN);
    pid_t pidF;
    pidF = fork();
    if (pidF < 0)
    {
        exit(EXIT_FAILURE);
    }
    else if (pidF == 0)
    {
        execl(tasks[count].command, NULL);
    }
    count++;
    current_time = time(NULL);
    info = localtime(&current_time);
    info->tm_hour = tasks[count].h;
    info->tm_min = tasks[count].m;
    info->tm_sec = 0;
    alarm(difftime(timelocal(info), current_time));
    signal(SIGALRM, AlarmHandler);
}
int test(){
    pid_t pidF;
    pidF = fork();
    if (pidF < 0)
    {
        exit(EXIT_FAILURE);
    }
    else if (pidF == 0)
    {
        execl("/mnt/f/OS/Proj/Dem", NULL);
    }
    return 0;
}
int main(int argc, char *argv[])
{
    //printf("S1");
    if (argc > 1)
    {
        task_file = fopen(argv[1], "r");
        //out_file = fopen(argv[2], "w");
    }
    //printf("S2");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    tasks = malloc(sizeof(struct task));
    int x = 0;
    //printf("S3");
    while ((read = getline(&line, &len, task_file)) != -1)
    {
        x++;
        struct task *taskTmp;
        taskTmp = malloc(sizeof(tasks));
        taskTmp = tasks;
        tasks = realloc(tasks, x * sizeof(struct task));
        char *token = strtok(line, ":");
        int i = 0;
        // loop through the string to extract all other tokens
        while (token != NULL)
        {
            switch (i)
            {
            case 0:
                tasks[x - 1].h = atoi(token);
                break;
            case 1:
                tasks[x - 1].m = atoi(token);
                break;
            case 2:
                tasks[x - 1].command = token;
                break;
            case 3:
                tasks[x - 1].info = atoi(token);
                break;
            }
            token = strtok(NULL, ":");
            i++;
        }
    }
    //printf("S4");
    for (int h = x - 1; h >= 0; h--)
    {
        for (int y = 0; y < h; y++)
        {
            if ((tasks[y].h > tasks[y + 1].h) || (tasks[y].h == tasks[y + 1].h && tasks[y].m > tasks[y + 1].m))
            {
                swap(&tasks[y], &tasks[y + 1]);
            }
        }
    }
    for (int y = 0; y < x; y++)
    {
        printf("Task nr: %d, h: %d, m: %d, Comm: %s, Inf: %d \n", y, tasks[y].h, tasks[y].m, tasks[y].command, tasks[y].info);
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
    //test();
    /* Daemon-specific initialization goes here */
    /* The Big Loop */
    count = 0;
    current_time = time(NULL);
    info = localtime(&current_time);
    info->tm_hour = tasks[count].h;
    info->tm_min = tasks[count].m;
    info->tm_sec = 0;
    pid_t pidF;
    pidF = fork();
    if (pidF < 0)
    {
        exit(EXIT_FAILURE);
    }
    else if (pidF == 0)
    {
        execl("/mnt/f/OS/Proj/Dem", NULL);
    }
    //alarm(difftime(timelocal(info), current_time));
    //signal(SIGALRM, AlarmHandler);
    /*while (1)
    {
        current_time = time(NULL);
        info = localtime(&current_time);
        sleep(60);
    }*/
    closelog();
    exit(EXIT_SUCCESS);
}
