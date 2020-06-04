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
FILE *out_file;

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
char *file;
void AlarmHandler(int sig)
{
    signal(SIGALRM, SIG_IGN);
    syslog(LOG_INFO, "Running task nr: %d, h: %d, m: %d, Comm: %s, Inf: %d \n", count, tasks[count].h, tasks[count].m, tasks[count].command, tasks[count].info);
    pid_t pidF;
    pidF = fork();
    if (pidF < 0)
    {
        exit(EXIT_FAILURE);
    }
    else if (pidF == 0)
    {
        int fd = open(file, O_WRONLY | O_APPEND);
        switch (tasks[count].info)
        {
        case 0:
            dup2(fd, 1);
            break;
        case 1:
            dup2(fd, 2);
            break;
        case 2:
            dup2(fd, 2);
            dup2(fd, 1);
            break;
        default:
            break;
        }
        close(fd);
        if(execl(tasks[count].command, NULL) == -1){
            syslog(LOG_ERR, "Error occurred when trying to run task nr: %d, h: %d, m: %d, Comm: %s, Inf: %d !!!\n", count, tasks[count].h, tasks[count].m, tasks[count].command, tasks[count].info);
        }
    }
    else
    {
        int status;
        int wpid;
        do
        {
            wpid = waitpid(pidF, &status, WUNTRACED);
        } while (!WIFEXITED(status));
        syslog(LOG_INFO, "Exit status of task nr: %d, exit status: %d", count, WIFEXITED(status));
    }

    count++;
    current_time = time(NULL);
    info = localtime(&current_time);
    info->tm_hour = tasks[count].h;
    info->tm_min = tasks[count].m;
    info->tm_sec = 0;
    if (difftime(timelocal(info), current_time) <= 0)
    {
        AlarmHandler(0);
    }
    else
    {
        alarm(difftime(timelocal(info), current_time));
        signal(SIGALRM, AlarmHandler);
    }
}
int main(int argc, char *argv[])
{
    if (argc == 3)
    {
        task_file = fopen(argv[1], "r");
        out_file = fopen(argv[2], "wb");
        fclose(out_file);
        file = argv[2];
    }
    else
    {
        printf("Not enough arguments.");
        exit(EXIT_FAILURE);
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    tasks = malloc(sizeof(struct task));
    int x = 0;
    while ((read = getline(&line, &len, task_file)) != -1)
    {
        x++;
        struct task *taskTmp;
        taskTmp = malloc(sizeof(tasks));
        taskTmp = tasks;
        tasks = realloc(tasks, x * sizeof(struct task));
        char *token = strtok(line, ":");
        int i = 0;
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
                tasks[x - 1].command = malloc(strlen(token) + 1);
                memcpy(tasks[x - 1].command, token, strlen(token) + 1);
                break;
            case 3:
                tasks[x - 1].info = atoi(token);
                break;
            }
            token = strtok(NULL, ":");
            i++;
        }
        if (i != 4)
        {
            printf("Error in parsing tasks file.");
            exit(EXIT_FAILURE);
        }
    }
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
    /*for (int y = 0; y < x; y++)
    {
        printf("Task nr: %d, h: %d, m: %d, Comm: %s, Inf: %d \n", y, tasks[y].h, tasks[y].m, tasks[y].command, tasks[y].info);
    }*/

    pid_t pid, sid;

    pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    openlog("MiniCron", LOG_CONS, LOG_CRON);

    sid = setsid();

    if (sid < 0)
    {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0)
    {
        exit(EXIT_FAILURE);
    }

    count = 0;
    current_time = time(NULL);
    info = localtime(&current_time);
    do
    {
        info->tm_hour = tasks[count].h;
        info->tm_min = tasks[count].m;
        info->tm_sec = 0;
        count++;
    } while (difftime(timelocal(info), current_time) < 0);
    count--;
    alarm(difftime(timelocal(info), current_time));

    signal(SIGALRM, AlarmHandler);
    while (count < x)
    {
        current_time = time(NULL);
        info = localtime(&current_time);
        sleep(60);
    }
    closelog();
    exit(EXIT_SUCCESS);
}
