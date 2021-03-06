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
int x = 0;
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
int pipeout, pipeerr;
pid_t *pidFArray;
int *arrayStarted;
void ChildChange(int sig)
{
    signal(SIGCHLD, SIG_IGN);
    int status;
    int wpid;
    for (int i = 0; i < sizeof(pidFArray) / sizeof(pid_t); i++)
    {
        if (pidFArray[i] > 0)
        {
            wpid = waitpid(pidFArray[i], &status, WUNTRACED);
            if (WIFEXITED(status))
            {
                syslog(LOG_INFO, "Exit status of task nr: %d, exit status: %d", arrayStarted[i], WEXITSTATUS(status));
                pidFArray[i] = -1;
            }
        }
    }
}

void AlarmHandler(int sig)
{
    signal(SIGALRM, SIG_IGN);
    syslog(LOG_INFO, "Running task nr: %d, h: %d, m: %d, Comm: %s, Inf: %d \n", count, tasks[count].h, tasks[count].m, tasks[count].command, tasks[count].info);
    pid_t pidF;
    pidF = fork();
    int fd = open(file, O_WRONLY | O_APPEND);
    if (pidF < 0)
    {
        exit(EXIT_FAILURE);
    }
    else if (pidF == 0)
    {
        switch (tasks[count].info)
        {
        case 0:
            dup2(fd, STDOUT_FILENO);
            break;
        case 1:
            dup2(fd, STDERR_FILENO);
            break;
        case 2:
            dup2(fd, STDERR_FILENO);
            dup2(fd, STDOUT_FILENO);
            break;
        default:
            break;
        }
        close(fd);
        if (execlp(tasks[count].command, NULL) == -1)
        {
            syslog(LOG_ERR, "Error occurred when trying to run task nr: %d, h: %d, m: %d, Comm: %s, Inf: %d !!!\n", count, tasks[count].h, tasks[count].m, tasks[count].command, tasks[count].info);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    else
    {
        int test = 0;
        for (int i = 0; i < sizeof(pidFArray) / sizeof(pid_t); i++)
        {
            if (pidFArray[i] <= 0)
            {
                pidFArray[i] = pidF;
                arrayStarted[i] = count;
                test = 1;
            }
        }
        if (test == 0)
        {
            pidFArray = realloc(pidFArray, ((sizeof(pidFArray) / sizeof(pid_t)) + 1) * sizeof(pid_t));
            arrayStarted = realloc(arrayStarted, ((sizeof(arrayStarted) / sizeof(int)) + 1) * sizeof(int));
            pidFArray[(sizeof(pidFArray) / sizeof(pid_t)) - 1] = pidF;
            arrayStarted[(sizeof(arrayStarted) / sizeof(int)) - 1] = count;
        }
    }

    count++;
    if (count < x)
    {
        current_time = time(NULL);
        info = localtime(&current_time);
        info->tm_hour = tasks[count].h % 25;
        info->tm_min = tasks[count].m % 61;
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
}

void WritePendingTasksList(int sig)
{
    signal(SIGUSR2, SIG_IGN);
    for (int y = count; y < x; y++)
    {
        syslog(LOG_INFO, "Task nr: %d, h: %d, m: %d, Comm: %s, Inf: %d \n", y, tasks[y].h, tasks[y].m, tasks[y].command, tasks[y].info);
    }
}

void exitSigHandler(int sig)
{
    exit(EXIT_SUCCESS);
}
char *path;
void ReadTasksFunc()
{
    free(tasks);
    task_file = fopen(path, "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    tasks = malloc(sizeof(struct task));
    x = 0;
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
            syslog(LOG_ERR, "Error in parsing tasks file.");
            exit(EXIT_FAILURE);
        }
        free(token);
    }
    free(line);
    fclose(task_file);
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
    count = 0;
    current_time = time(NULL);
    info = localtime(&current_time);
    do
    {
        info->tm_hour = tasks[count].h % 25;
        info->tm_min = tasks[count].m % 61;
        info->tm_sec = 0;
        count++;
    } while (difftime(timelocal(info), current_time) < 0);
    count--;
    info->tm_hour = tasks[count].h % 25;
    info->tm_min = tasks[count].m % 61;
    info->tm_sec = 0;
    if (difftime(timelocal(info), current_time) < 0)
    {
        count++;
    }
    else
    {
        alarm(difftime(timelocal(info), current_time));
    }
    /*printf("%d\n", x);
    for (int y = 0; y < x; y++)
    {
        printf("Task nr: %d, h: %d, m: %d, Comm: %s, Inf: %d \n", y, tasks[y].h, tasks[y].m, tasks[y].command, tasks[y].info);
    }*/
}

void reReadTasks(int sig)
{
    signal(SIGUSR1, SIG_IGN);
    ReadTasksFunc();
}

int main(int argc, char *argv[])
{
    signal(SIGCHLD, ChildChange);
    signal(SIGINT, exitSigHandler);
    signal(SIGALRM, AlarmHandler);
    signal(SIGUSR1, reReadTasks);
    signal(SIGUSR2, WritePendingTasksList);

    pidFArray = malloc(sizeof(pid_t));
    arrayStarted = malloc(sizeof(int));

    if (argc == 3)
    {
        path = argv[1];
        out_file = fopen(argv[2], "wb");
        fclose(out_file);
        file = argv[2];
    }
    else
    {
        printf("Not enough arguments.");
        exit(EXIT_FAILURE);
    }
    char cwd[100];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }
    size_t len = strlen("/");
    strncat(cwd, "/", len);
    len = strlen(path);
    strncat(cwd, path, len);
    path = cwd;
    /*printf("%d\n", x);
    for (int y = 0; y < x; y++)
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

    int fd;

    fd = open("/dev/null", O_RDWR, 0);
    dup2(fd, STDIN_FILENO);

    ReadTasksFunc();
    while (1)
    {
        int test = 0;
        for (int i = 0; i < sizeof(pidFArray) / sizeof(pid_t); i++)
        {
            if (pidFArray[i] > 0)
            {
                test = 1;
            }
        }
        if (count >= x)
        {
            if (test == 0)
            {
                break;
            }
            else
            {
                ChildChange(0);
            }
        }
        current_time = time(NULL);
        info = localtime(&current_time);
        sleep(60);
    }
    closelog();
    exit(EXIT_SUCCESS);
}