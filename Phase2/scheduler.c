#include "headers.h"
int msgq_id;
Process *All;
Process *HeadOfAll;
Process *curpostion;
// int *startmemory;
// int *currentstartmemory;
FILE *out_file;
FILE *out_file2;
FILE *out_file3;
FILE *out_file4;
// CircQueue Ready;
Process Dummy;
Process *running = &Dummy;
int CurSize = 0;

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
void SetToFile2(int time, int bytes, int procsid, int startingbyte, int algot)
{
    int n = ceil(log(bytes) / log(2));
    int i = startingbyte + pow(2, n) - 1;

    bytes = pow(2, n);

    fprintf(out_file3, "At time %d allocated %d bytes for process %d from %d to %d\n", time, bytes, procsid, startingbyte, i); // write to file
}
void SetToFile3(int time, int bytes, int procsid, int startingbyte, int algot)
{
    int n = ceil(log(bytes) / log(2));
    int i = startingbyte + pow(2, n) - 1;

    bytes = pow(2, n);

    fprintf(out_file3, "At time %d freed %d bytes from process %d from %d to %d\n", time, bytes, procsid, startingbyte, i); // write to file
}
void CPU(float *CPU_UTILIZATION, float *AVGWAIT, float *AVGWTA, float *STDWTA)
{
    float totalruntimes = 0;
    float totalwait = 0;
    float totalwta = 0;

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

void travreseSRTN(PriQueue wPriQ, PriQueue PriQ)
{
    if (wPriQ->top == NULL)
        return;
    QnodePtr temp = wPriQ->top;
    bool cond = true;

    while (temp != NULL && cond)
    {
        if (allocate(temp->P->memsize, &temp->P->Start_Index))
        {
            // printf("The qwait = %d",temp->P->Priority);
            Penqueue(PriQ, temp->P, temp->P->RemainingTime);
            wPriQ->top = temp->next;
            if (wPriQ->top == NULL)
            {
                wPriQ->tail = NULL;
            }
            free(temp);
            temp = wPriQ->top; //
        }
        else
        {
            cond = false;
        }
    }

    if (temp != NULL)
    {
        QnodePtr tempn = temp->next;
        while (temp != NULL && tempn != NULL)
        {
            if (allocate(tempn->P->memsize, &tempn->P->Start_Index))
            {
                Penqueue(PriQ, tempn->P, tempn->P->RemainingTime);
                temp->next = tempn->next;
                if (tempn == wPriQ->tail)
                {
                    wPriQ->tail = temp;
                }
                free(tempn);
                tempn = temp->next;
            }
            else
            {
                temp = temp->next;
                tempn = temp->next;
            }
        }
    }
}
void travrese2(Queuetype *wPriQ, Queuetype *PriQ, int time, int algot)
{
    if (wPriQ->top == NULL)
        return;

    QnodePtr temp = wPriQ->top;
    bool cond = true;
    // insert the top
    while (temp != NULL && cond)
    {
        if (allocate(temp->P->memsize, &temp->P->Start_Index))
        {
            SetToFile2(time, temp->P->memsize, temp->P->ID, temp->P->Start_Index, algot);
            printf("I allocate Process with id = %d\n", temp->P->ID);
            Penqueue(PriQ, temp->P, temp->P->Priority);
            wPriQ->top = temp->next;
            if (wPriQ->top == NULL)
            {
                wPriQ->tail = NULL;
            }
            if (temp != NULL)
            {
                free(temp);
            }
            temp = wPriQ->top;
        }
        else
        {
            // printf("I Can't allocate Process with id = %d\n",temp->P->ID);
            cond = false;
        }
    }
    // not the top
    if (temp != NULL)
    {
        QnodePtr tempn = temp->next;
        while (temp != NULL && tempn != NULL)
        {
            if (allocate(tempn->P->memsize, &tempn->P->Start_Index))
            {
                SetToFile2(time, temp->P->memsize, temp->P->ID, temp->P->Start_Index, algot);
                printf("I allocate Process with id = %d \n", temp->P->ID);
                Penqueue(PriQ, tempn->P, tempn->P->Priority);
                temp->next = tempn->next;
                if (tempn == wPriQ->tail)
                {
                    wPriQ->tail = temp;
                }
                if (tempn != NULL)
                {
                    free(tempn);
                }
                tempn = temp->next;
            }
            else
            {
                temp = temp->next;
                tempn = temp->next;
            }
        }
    }
}

void trytoallocate(PriQueue Ready, PriQueue waiting, Process *p)
{
    if (Ready->top == NULL)
    {
        Penqueue(Ready, p, p->RemainingTime);
        allocate(p->memsize, &p->Start_Index);
        return;
    }
    if (Ready->top == Ready->tail)
    {
        Penqueue(Ready, p, p->RemainingTime);
        allocate(p->memsize, &p->Start_Index);

        return;
    }

    while (!allocate(p->memsize, &p->Start_Index))
    {
        QnodePtr temp = Ready->top;

        QnodePtr temp2 = temp->next;

        while (temp != NULL && temp2 != NULL)
        {
            if (temp2 == Ready->tail)
                break;
            temp = temp->next;
            temp2 = temp->next;
        }
        Ready->tail = temp;
        temp->next = NULL;
        Penqueue(waiting, temp2->P, temp2->P->RemainingTime);
        deallocate(temp2->P->Start_Index);
        free(temp2);
    }
    Penqueue(Ready, p, p->RemainingTime);
}

void try_to_allocate(queue *RQ, queue *WQ, int time)
{
    if (WQ->head == NULL)
        return;

    qnode *temp = WQ->head;
    qnode *temp2;
    while (temp != NULL)
    {
        if (allocate(temp->p1->memsize, &temp->p1->memsize))
        {
            SetToFile2(time, temp->p1->memsize, temp->p1->ID, temp->p1->Start_Index, 1);
            if (temp == WQ->head)
            {
                WQ->head = temp->next;
                enqueue(RQ, temp->p1);
                free(temp);
                temp = WQ->head;
            }
            else
            {
                temp2->next = temp->next;
                enqueue(RQ, temp->p1);
                free(temp);
                temp = temp2->next;
            }
        }
        else
        {
            temp2 = temp;
            temp = temp->next;
        }
    }
}

int main(int argc, char *argv[])
{
    signal(SIGUSR1, handler);
    out_file = fopen("scheduler_log.txt", "w");

    out_file4 = fopen("temp.txt", "w");

    fprintf(out_file, "#At time x process y state arr w total z remain y wait k\n");
    out_file3 = fopen("memory_log.txt", "w"); // write only
    if (out_file3 == NULL)
    {
        printf("Error! Could not open file\n");
        exit(-1); // must include stdlib.h
    }

    fprintf(out_file3, "#At time x allocated y bytes for process z from i to j\n"); // write to file

    // initiate memory
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    memory = malloc(10 * sizeof(linked_list));
    memoryhead = memory;
    map = ht_create();

    int n = 10;
    // int size = n + 1;
    for (int i = 0; i <= n; i++)
    {
        init_list(&memory[i]);
    }

    // Initially whole block of specified
    // size is available
    // free_list[n].push_back(make_pair(0, sz - 1));
    Pair P10;
    P10.first = 0;
    P10.second = 1023;

    insert(&memory[n], P10);
    /////////////////////////////////////////////////////////////////////////////////////////////////////

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
    Queuetype *wPriQ;
    // Queuetype *HeadwPriQ;

    if (algotype == 1 || algotype == 2)
    {
        fflush(stdout);
        printf("Ready queue iniiated\n");
        PriQ = initPriQueue();
        printf("Waiting queue iniiated\n");
        wPriQ = initPriQueue();

        // HeadwPriQ = initPriQueue();
    }
    // HeadwPriQ = wPriQ;

    queue RRRQ;
    queue WRRQ;
    if (algotype == 3)
    {
        init_queue(&RRRQ);
        init_queue(&WRRQ);
    }

    All = malloc(NoPro * sizeof(Process));
    HeadOfAll = All;
    curpostion = All;
    // startmemory = malloc(NoPro * sizeof(int));
    // currentstartmemory = startmemory;

    int count = 0;
    int clock = getClk();
    int clock2 = getClk();
    int Quantum = 0;
    while (num_of_finished < NoPro)
    {
        // if(num_of_finished == NoPro){
        //     num_of_finished++;
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
                Penqueue(wPriQ, &All[count], All[count].Priority);
                // printf("Hello World! 1\n");
                // Penqueue(PriQ, &All[count], All[count].Priority);
                travrese2(wPriQ, PriQ, clock, 1);
                fflush(stdout);
                printf("process inserted in the waiting q\n");
                count++;
                // printf("Done\n");
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
                        kill(running->PID, SIGTSTP);
                        printf("we stopped a process");
                        strcpy(running->state, "STopped");
                        Penqueue(PriQ, running, running->RemainingTime);
                        running = &Dummy;
                    }
                }
                if (running != &Dummy && running->RemainingTime > All[count].RemainingTime)
                {

                    if (allocate(All[count].memsize, &All[count].Start_Index))
                    {
                        SetToFile2(getClk(), All[count].memsize, All[count].ID, All[count].Start_Index, 1);
                        Penqueue(PriQ, &All[count], All[count].RemainingTime);

                        fflush(stdout);
                        printf("process inserted in the pq\n");
                    }
                    else
                    {
                        trytoallocate(PriQ, wPriQ, &All[count]);
                        SetToFile2(getClk(), All[count].memsize, All[count].ID, All[count].Start_Index, 1);
                    }
                }
                else
                {

                    if (allocate(All[count].memsize, &All[count].Start_Index))
                    {
                        SetToFile2(getClk(), All[count].memsize, All[count].ID, All[count].Start_Index, 1);
                        Penqueue(PriQ, &All[count], All[count].RemainingTime);

                        fflush(stdout);
                        printf("process inserted in the pq\n");
                    }
                    else
                    {

                        Penqueue(wPriQ, &All[count], All[count].RemainingTime);
                        fflush(stdout);
                        printf("process inserted in the waiting q\n");
                    }
                }
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
                if (allocate(All[count].memsize, &All[count].Start_Index))
                {
                    SetToFile2(getClk(), All[count].memsize, All[count].ID, All[count].Start_Index, 1);
                    enqueue(&RRRQ, &All[count]);
                }
                else
                {
                    enqueue(&WRRQ, &All[count]);
                }
                fflush(stdout);
                printf("process inserted in the pq\n");
                count++;
                printf("Done\n");
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }
        if (algotype == 2)
        {
            if (!isEmpty(wPriQ))
                travreseSRTN(wPriQ, PriQ);
        }

        // HPF
        ////////////////////////////////////////////////////////////////////////////////////////////
        if (algotype == 1)
        {
            if (!isEmpty(wPriQ))
            {
                travrese2(wPriQ, PriQ, clock, 1);
            }
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
                    printf("the remainig time is %d\n", running->RemainingTime);
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
                    deallocate(running->Start_Index);
                    SetToFile3(running->FinishTime, running->memsize, running->ID, running->Start_Index, 1);
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

            if (running == &Dummy && !IsEmpty(&RRRQ) /*&& clock2 < getClk()*/)
            {

                running = dequeue(&RRRQ);
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
                    running->WatingTime = getClk() - running->ArrivalTime - running->RunTime + running->RemainingTime;
                    strcpy(running->state, "resumed");
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
                try_to_allocate(&RRRQ, &WRRQ, getClk());
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
                    enqueue(&RRRQ, running);
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
                    deallocate(running->Start_Index);
                    SetToFile3(getClk(), running->memsize, running->ID, running->Start_Index, 3);
                    try_to_allocate(&RRRQ, &WRRQ, getClk());

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
    fclose(out_file3);
    // printf("scheduler finished\n");
    destroyClk(true);
    kill(getppid(), SIGINT);
    // if(algotype==3)
    // RoundRobin(Q);

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
    if (algotype == 1)
    {
        deallocate(check->Start_Index);
        printf("the process finished is %d\n ", check->Priority);
        SetToFile3(check->FinishTime, check->memsize, check->ID, check->Start_Index, algotype);
    }
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
