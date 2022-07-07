#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

void handler(int sig);
void cont_handler(int sig);

int shmid;
int *shmaddrpro;
int sem1;
union Semun semun;
int clock;

int main(int agrc, char *argv[])
{
    // signal(SIGTSTP, handler);
    signal(SIGCONT, cont_handler);

    initClk();
    remainingtime = atoi(argv[0]);
    key_t key_id;
    key_id = ftok("MyKey", getpid());
    printf("the process %d is created and remainig time %d \n", getpid(), remainingtime);
    shmid = shmget(key_id, 4096, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    shmaddrpro = shmat(shmid, (void *)0, 0);
    if (shmaddrpro == -1)
    {
        perror("Error in attach in writer");
        exit(-1);
    }

    int sem = semget(key_id, 1, 0666 | IPC_CREAT);
    if (sem == -1)
    {
        perror("error in create");
    }
    semun.val = 0; /* initial value of the semaphore, Binary semaphore */
    if (semctl(sem, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }

    clock = getClk();
    while (remainingtime > 0)
    {
        signal(SIGCONT, cont_handler);
        clock = getClk();
        while (getClk() < clock + 1)
        {
        }
        remainingtime -= 1;
        *shmaddrpro = remainingtime;
        up(sem);
        printf("iam process %d and the remaining time now is %d and current time is %d\n", getpid(), remainingtime, getClk());
    }
    *shmaddrpro = remainingtime;
    up(sem);
    printf("iam process %d and iam finished\n", getpid());
    int finishtime = getClk();
    destroyClk(false);
    shmdt(shmaddrpro);
    // shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
    kill(getppid(), SIGUSR1);

    exit(finishtime);
    // raise(SIGKILL);

    return 0;
}

void handler(int sig)
{
    printf("iam process %d and i will stop\n", getpid());
    signal(SIGTSTP, SIG_DFL);
    raise(SIGTSTP);
}

void cont_handler(int sig)
{
    clock = getClk();
    signal(SIGCONT, SIG_DFL);
    raise(SIGCONT);
}
