#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <mqueue.h>

#define QUEUE_NAME "/queue"
#define MAX_SIZE 1024
#define MSG_STOP "exit"

mqd_t openQueue()
{
    mqd_t mq;

    mq = mq_open(QUEUE_NAME, O_WRONLY);

    return mq;
}

void closeQueue(mqd_t mq)
{
    mq_close(mq);
}

void writePi(double pi, mqd_t mq, int procNumber)
{
    char buffer[MAX_SIZE];

    fflush(stdout);
    memset(buffer, 0, MAX_SIZE);
    sprintf(buffer, "%f", pi);

    mq_send(mq, buffer, MAX_SIZE, 0);
}

void saveSum(double pi, int procNumber)
{
    mqd_t mq = openQueue();

    writePi(pi, mq, procNumber);
}

double generateRandomNumber()
{
    return (double)rand() / (double)RAND_MAX;
}

float calcSum(int procNumber, int totalProc)
{
    int passos = 1000000000 / totalProc;
    int initial = passos * procNumber;
    double sum = 0.0;
    double x;
    double h = 1.0 / 1000000000;

    for (int i = 1; i <= passos; i += 1)
    {
        x = h * ((double)(initial + i) - 0.5);

        sum += 4.0 / (1.0 + x * x);
    }

    return sum;
}

int main(int argc, char *argv[], char *envp[])
{
    int processNumber = atoi(argv[0]);
    int totalProcess = atoi(argv[1]);

    double pi = calcSum(processNumber, totalProcess);

    saveSum(pi, processNumber);
}
