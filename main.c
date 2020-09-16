#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAX_SIZE 1024
#define MSG_STOP "exit"

void callProcess(int numProcessos, char *argv[])
{
    pid_t pid = fork();

    if (pid == 0)
    {
        char procNumber[255];
        sprintf(procNumber, "%d", numProcessos);
        argv[0] = procNumber;

        execve("./calculo", argv, NULL);

        exit(0);
    }
}

mqd_t createQueue(char name[])
{
    mqd_t mq;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    mq = mq_open(name, O_CREAT | O_RDONLY, 0644, &attr);

    return mq;
}

void closeQueue(char name[], mqd_t mq)
{
    mq_close(mq);
    mq_unlink(name);
}

void callSums(char *argv[], int procNumbers)
{
    for (int i = 0; i < procNumbers; i++)
    {
        callProcess(i, argv);
    }
}

double receiveSums(int procNumbers, mqd_t mq)
{
    int must_stop = 0;
    char buffer[MAX_SIZE + 1];

    int terminatedProcess = 0;
    double sum = 0.0;

    do
    {
        ssize_t bytes_read;

        bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);

        buffer[bytes_read] = '\0';

        if (!strncmp(buffer, MSG_STOP, strlen(MSG_STOP)))
        {
            must_stop = 1;
        }
        else
        {
            double receivedSum = atof(buffer);

            printf("Processo %d retornou %f\n", terminatedProcess, receivedSum);

            sum += receivedSum;
            terminatedProcess++;
        }
    } while (terminatedProcess < procNumbers);

    return sum;
}

double calcPi(double sum)
{
    double h = 1.0 / 1000000000;

    return sum * h;
}

int main(int argc, char *argv[], char *envp[])
{
    int procNumbers = atoi(argv[1]);

    char queueName[] = "/queue";

    mqd_t mq = createQueue(queueName);

    callSums(argv, procNumbers);

    double sum = receiveSums(procNumbers, mq);

    printf("\nO valor de pi e: %.16f\n", calcPi(sum));

    closeQueue(queueName, mq);
}