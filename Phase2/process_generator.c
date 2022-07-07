#include "headers.h"
int No_of_Processes;
int msgq_id ;
Process *head;
struct msgbuff
{
    long mtype;
    Process mprocess
};

void ReadFile(Process *PD2)
{

    FILE *fp;
    char c;
    int count = 0; // Line counter (result)
    fp = fopen("processes.txt", "r");
    if (fp == NULL)
    {
        printf("Could not open file ");
        return 0;
    }
    for (c = getc(fp); c != EOF; c = getc(fp))
        if (c == '\n') // Increment count if this character is newline
            count = count + 1;
    fclose(fp);
    printf("The file has %d lines\n ", count);
    PD2 = malloc(count * sizeof(Process));
    head = PD2;
    // struct line* array = malloc(number_of_elements * sizeof(struct line));
    printf("The file has Done\n ");
    fp = fopen("processes.txt", "r");
    int k = 0;
    int lala = -1;
    char line[256];
    while (fgets(line, 260, fp))
    {
        k++;
        if (k < count)
        {
            int hm;
            lala++;
            fscanf(fp, "%d", &hm);
            (PD2[lala]).ID = hm;
            printf("THE ID %d ", (PD2[lala]).ID);
            fscanf(fp, "%d", &hm);
            (PD2[lala]).ArrivalTime = hm;
            printf("THE Arrival %d ", (PD2[lala]).ArrivalTime);
            fscanf(fp, "%d", &hm);
            (PD2[lala]).RunTime = hm;
            printf("THE Time %d ", (PD2[lala]).RunTime);

            fscanf(fp, "%d", &hm);
            (PD2[lala]).Priority = hm;
            printf("THE Priority %d ", (PD2[lala]).Priority);

            fscanf(fp, "%d", &hm);
            (PD2[lala]).memsize = hm;
            printf("THE memsize %d ", (PD2[lala]).memsize);

            PD2[lala].RemainingTime = PD2[lala].RunTime;
            strcpy(PD2[lala].state, "Ready");
            PD2[lala].PID = -1;
            PD2[lala].Start_Index = -1;

            printf("Line number %d \n ", k);
        }
    }
    printf("/////////////////////////////////////////////////////////////////////// \n");
    No_of_Processes = count - 1;
    for (int l = 1; l < count; l++)
    {
        printf("THE ID %d ", (PD2[l - 1]).ID);
        printf("THE Arrival %d ", (PD2[l - 1]).ArrivalTime);
        printf("THE Time %d ", (PD2[l - 1]).RunTime);
        printf("THE Priority %d ", (PD2[l - 1]).Priority);
        printf("THE memsize %d ", (PD2[l - 1]).memsize);

        printf("Line %d Ya khaleeed \n ", l);
    }
    // printf("%d\n",(PD2[]).ID);
}
void clearResources(int);

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    Process *PD2;
    ReadFile(PD2);
    int i = 0;
    printf("%d\n", (head[0]).ID);
    PD2 = head;
    printf("%d\n", (PD2[i]).ID);
    key_t key_id;

    key_id = ftok("MyKey", 45);

     msgq_id = msgget(key_id, 0666 | IPC_CREAT);
    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    int algotype;
    printf("Choose The number of the scheduling algorithm \n");
    printf("1 - HPF \n2 - SRTN \n3 - RR\n");
    scanf("%d", &algotype);
    char buffer[5];

    char buffer2[5];
    char buffer3[5];

    int Q = 0;

    if (algotype == 3)
    {

        printf("Write the quantum ");
        scanf("%d", &Q);
    }
    sprintf(buffer, "%d", No_of_Processes);
    sprintf(buffer2, "%d", algotype);
    sprintf(buffer3, "%d", Q);
    // system("./clk.out &");
    int pid = fork();

    if (pid == 0)
    {
        execl("scheduler", buffer, buffer2, buffer3, NULL);
    }
    int pid2 = fork();

    if (pid2 == 0)
    {
        execl("clk", NULL);
    }
    // exit(0);

    initClk();

    int count = 0;
    int clk = 0;
    i = 0;

    while (count < No_of_Processes)
    {
        if (getClk() >= (PD2[i]).ArrivalTime)
        {
            Process p = (PD2[i]);
            struct msgbuff buffer;
            buffer.mtype = 3;
            buffer.mprocess = p;
            printf("%d\n", p.ID);
            printf("%d\n", p.ArrivalTime);
            int send_val = msgsnd(msgq_id, &buffer, sizeof(buffer.mprocess), !IPC_NOWAIT);
            if (send_val == -1)
            {
                printf("cannot send the process");
            }
            else
            {
                printf("we send process %d \n", buffer.mprocess.ID);
            }
            count++;
            i++;
        }
        // clk++;
    }
    
    while (1)
    {
    }
    
    // msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
}

void clearResources(int signum)
{
    msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
    exit(0);
    // TODO Clears all resources in case of interruption
}
