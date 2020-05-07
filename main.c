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
FILE *out_file;

int main(int argc, char *argv[])
{
    if (argc == 3)
    {
        task_file = fopen(argv[1], "r");
        out_file = fopen(argv[2], "w");
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
