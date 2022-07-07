#include "headers.h"
int msgq_id;
Process *All;
Process *HeadOfAll;
Process *curpostion;
// CircQueue Ready;
Process Dummy;
Process *running = &Dummy;
int CurSize = 0;
FILE *out_file;
FILE *out_file2;
int shmid;
int *shmaddrpro;
int sem1;
union Semun semun;
int CurrentTime = 0;
int NoPro;
int num_of_finished = 0;
int algotype;
int sem;
int lastprocessfinishtime;
void handler(int signum);
struct msgbuff
{
    long mtype;
    Process mprocess
};

void CPU(float *CPU_UTILIZATION, float *AVGWAIT, float *AVGWTA, float *STDWTA)
{
    float totalruntimes = 0;
    float totalwait = 0;
    float totalwta = 0;
    All = HeadOfAll;

    for (size_t i = 0; i < NoPro; i++)
    {
        totalruntimes += (float)All[i].RunTime;
        totalwta += (float)All[i].WTA;
        totalwait += (float)All[i].WatingTime;
    }
    All = HeadOfAll;
    for (size_t i = 0; i < NoPro; i++)
    {

        *STDWTA += (float)pow(All[i].WTA, 2);
    }
    *CPU_UTILIZATION = (totalruntimes / (lastprocessfinishtime - 1)) * 100;
    *AVGWAIT = (totalwait / NoPro);
    *AVGWTA = (totalwta / NoPro);
    *STDWTA = (float)sqrt((double)(*STDWTA) / (double)NoPro);
}

void HPF()
{
    fflush(stdout);
    printf("HPF started");
}

int main(int argc, char *argv[])
{
    signal(SIGUSR1, handler);
    out_file = fopen("scheduler_log.txt", "w");

    fprintf(out_file, "#At time x process y state arr w total z remain y wait k\n");

    strcpy(Dummy.state, "");
    Dummy.ArrivalTime = -1;
    Dummy.FinishTime = -1;
    Dummy.ID = -1;
    Dummy.PID = -1;
    Dummy.Priority = -1;
    Dummy.RunTime = -1;
    Dummy.StartedTime = -1;
    Dummy.RemainingTime = -1;

    printf("scheduler started\n");
    NoPro = atoi(argv[0]);
    printf("No Processes is %d\n", NoPro);
    algotype = atoi(argv[1]);
    printf("Algorithm is %d\n", algotype);

    int Q = atoi(argv[2]);
    printf("Quantum is %d\n", Q);
    initClk();
    key_t key_id;

    key_id = ftok("MyKey", 45);
    int msgq_id = msgget(key_id, 0666 | IPC_CREAT);
    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    Queuetype *PriQ;
    if (algotype == 1 || algotype == 2)
    {
        fflush(stdout);
        printf("priorty queue iniiated\n");
        PriQ = initPriQueue();
    }

    queue RRQ;
    if (algotype == 3)
    {
        init_queue(&RRQ);
    }

    All = malloc(NoPro * sizeof(Process));
    HeadOfAll = All;
    curpostion = All;

    int count = 0;
    int clock = getClk();
    int clock2 = getClk();
    int Quantum = 0;
    while (num_of_finished < NoPro)
    {
        // if (clock < getClk())
        // {
        //     printf("the clock is %d\n", getClk());
        //     clock = getClk();
        // }

        struct msgbuff buffer;
        int rec_val = msgrcv(msgq_id, &buffer, sizeof(buffer.mprocess), 3, IPC_NOWAIT);
        if (rec_val != -1)
        {

            if (algotype == 1)
            {

                CurSize++;
                All[count] = buffer.mprocess;
                curpostion = All;
                printf("Id is %d\n", All[count].ID);
                printf("Arrival Time is %d\n", All[count].ArrivalTime);
                printf("my priority is %d \n", All[count].Priority);
                Penqueue(PriQ, &All[count], All[count].Priority);
                fflush(stdout);
                printf("process inserted in the pq\n");
                count++;
                printf("Done\n");
            }

            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (algotype == 2)
            {
                printf("i check sth \n");
                CurSize++;
                All[count] = buffer.mprocess;
                curpostion = All;
                printf("Id is %d\n", All[count].ID);
                printf("Arrival Time is %d\n", All[count].ArrivalTime);
                printf("my remaining time is %d \n", All[count].RemainingTime);
                if (running != &Dummy)
                {
                    printf("iam not dummy");
                    if (running->RemainingTime > All[count].RemainingTime)
                    {
                        fprintf(out_file, "At time %d process %d stopped arr %d total %d remain %d wait %d\n",
                                getClk(), running->ID, running->ArrivalTime, running->RunTime, running->RemainingTime, running->WatingTime);
                        // if (*shmaddrpro != NULL && shmaddrpro != -1)
                        // running->RemainingTime = *shmaddrpro;
                        kill(running->PID, SIGTSTP);
                        printf("we stopped a process");
                        strcpy(running->state, "STopped");
                        Penqueue(PriQ, running, running->RemainingTime);
                        running = &Dummy;
                    }
                }
                Penqueue(PriQ, &All[count], All[count].RemainingTime);
                fflush(stdout);
                printf("process inserted in the pq\n");
                count++;
                printf("Done\n");
            }
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (algotype == 3)
            {
                CurSize++;
                All[count] = buffer.mprocess;
                curpostion = All;
                printf("Id is %d\n", All[count].ID);
                printf("Arrival Time is %d\n", All[count].ArrivalTime);
                printf("my priority is %d \n", All[count].Priority);
                // Penqueue(PriQ, &All[count], All[count].Priority);
                enqueue(&RRQ, &All[count]);
                fflush(stdout);
                printf("process inserted in the pq\n");
                count++;
                printf("Done\n");
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }

        // HPF
        ////////////////////////////////////////////////////////////////////////////////////////////
        if (algotype == 1)
        {
            if (running == &Dummy && !isEmpty(PriQ))
            {
                running = pdequeue(PriQ);
                fflush(stdout);
                // printf("there is a process start now with Pid %d \n", running->PID);

                char buffer[5];
                sprintf(buffer, "%d", running->RemainingTime);
                int pid = fork();

                if (pid == 0)
                {
                    printf("process created\n");
                    execl("process", buffer, NULL);
                }
                running->PID = pid;
                strcpy(running->state, "started");

                running->StartedTime = getClk();
                running->WatingTime = -running->ArrivalTime + running->StartedTime;
                key_t key_id, key;
                key_id = ftok("MyKey", running->PID);
                shmid = shmget(key_id, 4096, IPC_CREAT | 0666);
                shmaddrpro = shmat(shmid, (void *)0, 0);
                fprintf(out_file, "At time %d process %d started arr %d total %d remain %d wait %d\n",
                        running->StartedTime, running->ID, running->ArrivalTime, running->RunTime, running->RemainingTime, running->WatingTime);
            }

            if (clock < getClk())
            {
                if (running != &Dummy)
                {
                    if (*shmaddrpro != NULL && shmaddrpro != -1)
                        running->RemainingTime = *shmaddrpro;
                    fflush(stdout);
                    printf(".\n");
                    // printf("////////////////////////////////////////////////////////////////////////////////////\n");
                    // printf("////////////////////////////////////////////////////////////////////////////////////\n");
                    // printf("////////////////////////////////////////////////////////////////////////////////////\n");
                    // printf("the ID is %d\n", running->ID);
                    // printf("the PID is %d\n", running->PID);
                    // printf("the start time is %d\n", running->StartedTime);
                    printf("the remainig time is %d\n", running->RemainingTime);
                    // printf("the priority is %d\n", running->Priority);
                    // printf("the state is %s\n", running->state);
                    // printf("////////////////////////////////////////////////////////////////////////////////////\n");
                    // printf("////////////////////////////////////////////////////////////////////////////////////\n");
                    // printf("////////////////////////////////////////////////////////////////////////////////////\n");
                }
                clock = getClk();
            }
        }
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // SRTN
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (algotype == 2)
        {
            if (running == &Dummy && !isEmpty(PriQ))
            {
                running = pdequeue(PriQ);
                fflush(stdout);
                // printf("there is a process start now with Pid %d \n", running->PID);
                if (running->PID == -1)
                {
                    printf("the process will run\n");
                    char buffer[5];
                    sprintf(buffer, "%d", running->RemainingTime);
                    int pid = fork();

                    if (pid == 0)
                    {
                        printf("process created\n");
                        execl("process", buffer, NULL);
                    }
                    running->PID = pid;
                    strcpy(running->state, "started");
                    running->StartedTime = getClk();
                    running->WatingTime = -running->ArrivalTime + running->StartedTime;
                    key_t key_id, key;
                    key_id = ftok("MyKey", running->PID);
                    shmid = shmget(key_id, 4096, IPC_CREAT | 0666);
                    shmaddrpro = shmat(shmid, (void *)0, 0);

                    sem = semget(key_id, 1, 0666 | IPC_CREAT);
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
                    fprintf(out_file, "At time %d process %d started arr %d total %d remain %d wait %d\n",
                            running->StartedTime, running->ID, running->ArrivalTime, running->RunTime, running->RemainingTime, running->WatingTime);
                }
                else
                {

                    kill(running->PID, SIGCONT);
                    strcpy(running->state, "resumed");
                    running->WatingTime = getClk() - running->ArrivalTime - running->RunTime + running->RemainingTime;
                    key_t key_id, key;
                    key_id = ftok("MyKey", running->PID);
                    shmid = shmget(key_id, 4096, IPC_CREAT | 0666);
                    shmaddrpro = shmat(shmid, (void *)0, 0);

                    sem = semget(key_id, 1, 0666 | IPC_CREAT);
                    if (sem == -1)
                    {
                        perror("error in create");
                    }
                    printf("we will continue a process \n");

                    fprintf(out_file, "At time %d process %d resumed arr %d total %d remain %d wait %d\n",
                            getClk(), running->ID, running->ArrivalTime, running->RunTime, running->RemainingTime, running->WatingTime);
                }
            }

            if (running != &Dummy && clock < getClk())
            {
                down(sem);
                running->RemainingTime = *shmaddrpro;

                printf("process %d Remaining time %d\n", running->ID, running->RemainingTime);
                if (running->RemainingTime == 0 && running != &Dummy)
                {

                    running->FinishTime = getClk();
                    lastprocessfinishtime = running->FinishTime;
                    running->RemainingTime = 0;
                    running->WatingTime = (running->FinishTime) - (running->ArrivalTime) - (running->RunTime);
                    running->TA = (running->FinishTime) - (running->ArrivalTime);
                    running->WTA = (float)(running->TA) / (running->RunTime);
                    printf("process %d finished at time %d\n", running->ID, running->FinishTime);
                    strcpy(running->state, "Finished");
                    key_t key_id;
                    key_id = ftok("MyKey", running->PID);
                    shmid = shmget(key_id, 4096, IPC_CREAT | 0666);
                    if (shmid == -1)
                    {
                        perror("Error in create");
                        exit(-1);
                    }
                    fprintf(out_file, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
                            running->FinishTime, running->ID, running->ArrivalTime, running->RunTime, running->RemainingTime, running->WatingTime, running->TA, running->WTA);

                    shmdt(shmaddrpro);
                    semctl(sem, 0, IPC_RMID, (union Semun *)0);
                    shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);

                    running = &Dummy;
                    num_of_finished++;
                }
                clock = getClk();
            }
            // if (clock < getClk())
            // {

            //     if (running != &Dummy)
            //     {

            //         if (*shmaddrpro != NULL && shmaddrpro != -1)
            //             running->RemainingTime = *shmaddrpro;
            //         fflush(stdout);
            //         printf(".\n");
            //         printf("////////////////////////////////////////////////////////////////////////////////////\n");
            //         printf("////////////////////////////////////////////////////////////////////////////////////\n");
            //         printf("////////////////////////////////////////////////////////////////////////////////////\n");
            //         printf("the ID is %d\n", running->ID);
            //         printf("the PID is %d\n", running->PID);
            //         printf("the start time is %d\n", running->StartedTime);
            //         printf("the remainig time is %d\n", running->RemainingTime);
            //         printf("the priority is %d\n", running->Priority);
            //         printf("the state is %s\n", running->state);
            //         printf("////////////////////////////////////////////////////////////////////////////////////\n");
            //         printf("////////////////////////////////////////////////////////////////////////////////////\n");
            //         printf("////////////////////////////////////////////////////////////////////////////////////\n");
            //     }
            //     clock = getClk();
            // }
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////

        // RR
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (algotype == 3)
        {
            if (running == &Dummy && !IsEmpty(&RRQ) /*&& clock2 < getClk()*/)
            {
                running = dequeue(&RRQ);
                if (running->PID == -1)
                {
                    printf("the process will run\n");
                    char buffer[5];
                    sprintf(buffer, "%d", running->RemainingTime);
                    int pid = fork();

                    if (pid == 0)
                    {
                        printf("process created\n");
                        execl("process", buffer, NULL);
                    }
                    running->PID = pid;
                    strcpy(running->state, "started");
                    running->StartedTime = getClk();
                    running->WatingTime = -running->ArrivalTime + running->StartedTime;
                    key_t key_id, key;
                    key_id = ftok("MyKey", running->PID);
                    shmid = shmget(key_id, 4096, IPC_CREAT | 0666);
                    shmaddrpro = shmat(shmid, (void *)0, 0);

                    sem = semget(key_id, 1, 0666 | IPC_CREAT);
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
                    fprintf(out_file, "At time %d process %d started arr %d total %d remain %d wait %d\n",
                            running->StartedTime, running->ID, running->ArrivalTime, running->RunTime, running->RemainingTime, running->WatingTime);
                }
                else
                {
                    printf("we will continue a process \n");
                    kill(running->PID, SIGCONT);
                    strcpy(running->state, "resumed");
                    running->WatingTime = getClk() - running->ArrivalTime - running->RunTime + running->RemainingTime;
                    key_t key_id, key;
                    key_id = ftok("MyKey", running->PID);
                    shmid = shmget(key_id, 4096, IPC_CREAT | 0666);
                    shmaddrpro = shmat(shmid, (void *)0, 0);

                    sem = semget(key_id, 1, 0666 | IPC_CREAT);
                    if (sem == -1)
                    {
                        perror("error in create");
                    }
                    fprintf(out_file, "At time %d process %d resumed arr %d total %d remain %d wait %d\n",
                            getClk(), running->ID, running->ArrivalTime, running->RunTime, running->RemainingTime, running->WatingTime);
                }
                clock2 = getClk();
            }

            if (running != &Dummy && clock < getClk())
            {
                int rem = running->RemainingTime;
                down(sem);
                running->RemainingTime = *shmaddrpro;
                int dif = rem - running->RemainingTime;
                Quantum = Quantum + dif;
                // Quantum++;
                printf("process %d Remaining time %d at time %d \n ", running->ID, running->RemainingTime, getClk());
                if (Quantum >= Q && running->RemainingTime != 0)
                {
                    fprintf(out_file, "At time %d process %d stopped arr %d total %d remain %d wait %d\n",
                            getClk(), running->ID, running->ArrivalTime, running->RunTime, running->RemainingTime, running->WatingTime);
                    printf("we will stop a process with pid %d\n", running->PID);
                    kill(running->PID, SIGTSTP);
                    strcpy(running->state, "Stopped");
                    enqueue(&RRQ, running);
                    running = &Dummy;
                    Quantum = 0;
                }

                if (running->RemainingTime == 0 && running != &Dummy)
                {

                    running->FinishTime = getClk();
                    lastprocessfinishtime = running->FinishTime;
                    running->RemainingTime = 0;
                    running->WatingTime = (running->FinishTime) - (running->ArrivalTime) - (running->RunTime);
                    running->TA = (running->FinishTime) - (running->ArrivalTime);
                    running->WTA = (float)(running->TA) / (running->RunTime);
                    printf("process %d finished at time %d\n", running->ID, running->FinishTime);
                    strcpy(running->state, "Finished");
                    key_t key_id;
                    key_id = ftok("MyKey", running->PID);
                    shmid = shmget(key_id, 4096, IPC_CREAT | 0666);
                    if (shmid == -1)
                    {
                        perror("Error in create");
                        exit(-1);
                    }
                    fprintf(out_file, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
                            running->FinishTime, running->ID, running->ArrivalTime, running->RunTime, running->RemainingTime, running->WatingTime, running->TA, running->WTA);

                    shmdt(shmaddrpro);
                    semctl(sem, 0, IPC_RMID, (union Semun *)0);
                    shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
                    running = &Dummy;
                    num_of_finished++;
                    Quantum = 0;
                }

                // printf("process %d Remaining time %d\n", running->ID, running->RemainingTime);

                clock = getClk();
            }
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }

    All = HeadOfAll;
    fclose(out_file);
    out_file2 = fopen("scheduler_pref.txt", "w");
    if (out_file2 == NULL)
        printf("Error opening File 2");
    float CPU_UTILIZATION = 0;
    float AVGWAIT = 0;
    float AVGWTA = 0;
    float STDWTA = 0;
    CPU(&CPU_UTILIZATION, &AVGWAIT, &AVGWTA, &STDWTA);
    fprintf(out_file2, "CPU utilization = %.2f%c\n", CPU_UTILIZATION, '%');
    fprintf(out_file2, "Avg WTA = %.2f\n", AVGWTA);
    fprintf(out_file2, "Avg Waiting = %.2f\n", AVGWAIT);
    fprintf(out_file2, "Std WTA = %.2f\n", STDWTA);
    printf("scheduler finished\n");

    fclose(out_file2);
    destroyClk(true);
    kill(getppid(), SIGINT);
    exit(0);

    // msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
}

void handler(int signum)
{

    printf(" schedular take the signal\n");
    // running = &Dummy;
    int pid, stat_loc;
    pid = wait(&stat_loc);
    printf("the pid of finished process is %d", pid);
    printf("stat loc is %d\n", stat_loc);
    int finishT = (stat_loc >> 8);
    printf("my finish time is %d \n", finishT);
    if (algotype != 1)
        return;
    All = HeadOfAll;
    Process *check;
    for (int i = 0; i < CurSize; i++)
    {
        if (All[i].PID == pid)
        {
            check = &All[i];
            All = curpostion;
            break;
        }
    }

    check->FinishTime = finishT;
    lastprocessfinishtime = check->FinishTime;
    check->RemainingTime = 0;
    check->WatingTime = (check->FinishTime) - (check->ArrivalTime) - (check->RunTime);
    check->TA = (check->FinishTime) - (check->ArrivalTime);
    check->WTA = (float)(check->TA) / (check->RunTime);
    strcpy(check->state, "Finished");
    printf("the process priority that finished is %d\n ", check->Priority);
    fprintf(out_file, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
            check->FinishTime, check->ID, check->ArrivalTime, check->RunTime, check->RemainingTime, check->WatingTime, check->TA, check->WTA);
    key_t key_id;
    key_id = ftok("MyKey", check->PID);
    shmid = shmget(key_id, 4096, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    shmdt(shmaddrpro);
    shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
    // running = &Dummy;
    sem = semget(key_id, 1, 0666 | IPC_CREAT);
    if (sem == -1)
    {
        perror("error in create");
    }

    semctl(sem, 0, IPC_RMID, (union Semun *)0);
    if (running == check)
        running = &Dummy;

    num_of_finished++;
}
