#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

typedef short bool;
#define true 1
#define false 0
#define QUEUE_EMPTY INT_MIN
#define SHKEY 300

typedef struct Process
{
	int ID;
	int PID;
	int ArrivalTime;
	int RunTime;
	int Priority;
	int StartedTime;
	int FinishTime;
	int RemainingTime; // hint
	int WatingTime;	   //=CurrentTime-ArrivalTime-RunTime+RemainingTime(not written yet)
	int TA;
	float WTA;

	char state[15];

} Process;

// QUEUE
////////////////////////////////////////////////////////////////////////////////////////
typedef struct qnode
{
	struct Process *p1;
	struct qnode *next;
} qnode;

typedef struct
{
	qnode *head;
	qnode *tail;
} queue;

void init_queue(queue *q)
{
	q->head = NULL;
	q->tail = NULL;
}

bool enqueue(queue *q, Process *p1)
{

	// create a new node
	qnode *newnode = malloc(sizeof(qnode));
	if (newnode == NULL)
		return false;
	newnode->p1 = p1;
	newnode->next = NULL;

	// if there is a tail , connect that tail to this new node
	if (q->tail != NULL)
	{
		q->tail->next = newnode;
	}

	q->tail = newnode;
	// make sure the head makes sence
	if (q->head == NULL)
	{
		q->head = newnode;
	}
	return true;
}

Process *dequeue(queue *q)
{
	// check if the queue is empty
	// if (q->head ==NULL)return NULL;

	// save the head of the queue
	qnode *tmp = q->head;
	// save the result that we gonna return
	Process *result = tmp->p1;

	// take it off
	q->head = q->head->next;
	if (q->head == NULL)
	{
		q->tail = NULL;
	}
	free(tmp);
	return result;
}
bool IsEmpty(queue *q)
{
	if (q->head == NULL && q->tail == NULL)
		return true;
	else
		return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

// LINKED LIST
//////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct node
{
	struct Process p1;
	struct node *next;
} node;

typedef struct
{
	node *head;
	node *tail;
} linked_list;

void init_list(linked_list *L)
{
	L->head = NULL;
	L->tail = NULL;
}

bool insert(linked_list *L, Process p1)
{

	// create a new node
	node *newnode = malloc(sizeof(node));
	if (newnode == NULL)
		return false;
	newnode->p1 = p1;
	newnode->next = NULL;

	// if there is a tail , connect that tail to this new node
	if (L->tail != NULL)
	{
		L->tail->next = newnode;
	}

	L->tail = newnode;
	// make sure the head makes sence
	if (L->head == NULL)
	{
		L->head = newnode;
	}
	return true;
}

Process removehead(linked_list *L)
{
	// check if the queue is empty
	// if (q->head ==NULL)return NULL;

	// save the head of the queue
	node *tmp = L->head;
	// save the result that we gonna return
	Process result = tmp->p1;

	// take it off
	L->head = L->head->next;
	if (L->head == NULL)
	{
		L->tail = NULL;
	}
	free(tmp);
	return result;
}

void traverse(linked_list *L)
{
	node *tmp = L->head;
	while (tmp != NULL)
	{
		tmp = tmp->next;
	}
}

Process *findP(linked_list *L, Process p1)
{
	node *tmp = L->head;
	while (tmp != NULL)
	{
		if (tmp->p1.PID == p1.PID)
			return &tmp->p1;
		tmp = tmp->next;
	}
	return NULL;
}
Process *retriveP(linked_list *L, int pid)
{
	node *tmp = L->head;
	while (tmp != NULL)
	{
		if (tmp->p1.PID == pid)
			return &tmp->p1;
		tmp = tmp->next;
	}
	return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////

// PRIORITY QUEUE
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	Process *P;
	struct Qnode *next;
	int priority;
	/* data */
} Qnode, *QnodePtr;

typedef struct PriQueue
{
	QnodePtr top;
	QnodePtr tail;
} Queuetype, *PriQueue;

PriQueue initPriQueue()
{
	PriQueue newPriQueue = malloc(sizeof(Queuetype));
	newPriQueue->top = NULL;
	newPriQueue->tail = NULL;
	return newPriQueue;
}

bool isEmpty(PriQueue q)
{
	return (q->top == NULL);
}

void Penqueue(PriQueue q, Process *p, int pri)
{
	QnodePtr newNode = malloc(sizeof(Qnode));
	newNode->P = p;
	newNode->next = NULL;
	newNode->priority = pri;

	if (!isEmpty(q))
	{
		if (newNode->priority < q->top->priority)
		{
			newNode->next = q->top;
			q->top = newNode;
		}
		else if (q->top->next == NULL)
		{
			q->top->next = newNode;
		}
		else
		{
			QnodePtr prevTemp = q->top;
			QnodePtr temp = q->top->next;
			bool notnull = 1;
			while (notnull)
			{
				if (temp->priority > newNode->priority)
				{
					newNode->next = temp;
					prevTemp->next = newNode;
					return;
				}
				if (temp->next != NULL)
				{
					prevTemp = temp;
					temp = temp->next;
				}
				else
					notnull = false;
			}
			// while (temp->next != NULL)
			// {
			// 	if (temp->priority > newNode->priority)
			// 	{
			// 		newNode->next = temp;
			// 		prevTemp->next = newNode;
			// 		return;
			// 	}
			// 	prevTemp = temp;
			// 	temp = temp->next;
			// }

			temp->next = newNode;
			q->tail = newNode;
		}
	}
	else
	{
		q->top = newNode;
		q->tail = newNode;
	}
}

Process *pdequeue(PriQueue q)
{
	if (isEmpty(q))
	{
		return NULL;
	}
	Process *p = q->top->P;
	QnodePtr temp = q->top;
	q->top = q->top->next;
	free(temp);
	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* arg for semctl system calls. */
union Semun
{
	int val;			   /* value for SETVAL */
	struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
	ushort *array;		   /* array for GETALL & SETALL */
	struct seminfo *__buf; /* buffer for IPC_INFO */
	void *__pad;
};

void down(int sem)
{
	struct sembuf p_op;

	p_op.sem_num = 0;
	p_op.sem_op = -1;
	p_op.sem_flg = !IPC_NOWAIT;

	if (semop(sem, &p_op, 1) == -1)
	{
		perror("Error in down()");
		exit(-1);
	}
}

void up(int sem)
{
	struct sembuf v_op;

	v_op.sem_num = 0;
	v_op.sem_op = 1;
	v_op.sem_flg = !IPC_NOWAIT;

	if (semop(sem, &v_op, 1) == -1)
	{
		perror("Error in up()");
		exit(-1);
	}
}

///==============================
// don't mess with this variable//
int *shmaddr; //
//===============================

int getClk()
{
	return *shmaddr;
}

/*
 * All Process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
 */
void initClk()
{
	int shmid = shmget(SHKEY, 4, 0444);
	while ((int)shmid == -1)
	{
		// Make sure that the clock exists
		printf("Wait! The clock not initialized yet!\n");
		sleep(1);
		shmid = shmget(SHKEY, 4, 0444);
	}
	shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All Process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
 */

void destroyClk(bool terminateAll)
{
	shmdt(shmaddr);
	if (terminateAll)
	{
		killpg(getpgrp(), SIGINT);
	}
}
