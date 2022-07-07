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
#define TABLE_SIZE 20000

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
	int memsize;
	int Start_Index;
	int timedone;

	char state[15];

} Process;

// MAP
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct entry_t
{
	int key;
	int value;

} entry_t;

typedef struct
{
	entry_t **entries;
} ht_t;

entry_t *ht_pair(int key, int value)
{
	// allocate the entry
	entry_t *entry = malloc(sizeof(entry_t) * 1);
	entry->key = key;
	entry->value = value;

	return entry;
}
ht_t *ht_create(void)
{
	// allocate table
	ht_t *hashtable = malloc(sizeof(ht_t) * 1);

	// allocate table entries
	hashtable->entries = malloc(sizeof(entry_t *) * TABLE_SIZE);

	// set each to null (needed for proper operation)
	int i = 0;
	for (; i < TABLE_SIZE; ++i)
	{
		hashtable->entries[i] = NULL;
	}

	return hashtable;
}
void ht_set(ht_t *hashtable, int key, int value)
{

	// try to look up an entry set
	entry_t *entry = hashtable->entries[key];

	// no entry means slot empty, insert immediately
	if (entry == NULL)
	{
		hashtable->entries[key] = ht_pair(key, value);
		return;
	}
	if (entry != NULL && hashtable->entries[key]->value == -1)
	{
		hashtable->entries[key]->value = value;
		return;
	}
}
int ht_get(ht_t *hashtable, int key)
{

	// try to find a valid slot
	entry_t *entry = hashtable->entries[key];

	// no slot means no entry
	if (entry == NULL)
	{
		return -1;
	}
	if (entry != NULL && hashtable->entries[key]->value == -1)
	{
		return -1;
	}
	return entry->value;
}
int ht_del(int key, ht_t *hashtable)
{
	entry_t *entry = hashtable->entries[key];
	if (entry == NULL)
	{
		return -1;
	}
	if (entry != NULL && hashtable->entries[key]->value == -1)
	{
		return -1;
	}
	hashtable->entries[key]->value = -1;
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////

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
//trav
bool IsEmpty(queue *q)
{
	if (q->head == NULL && q->tail == NULL)
		return true;
	else
		return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

// PAIR
//////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Pair
{
	int first;
	int second;
} Pair;

/////////////////////////////////////////////////////////////////////////////////////////////////////

// LINKED LIST
//////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct node
{
	struct Pair p1;
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

bool insert(linked_list *L, Pair p1)
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

Pair removehead(linked_list *L)
{
	// check if the queue is empty
	// if (L->head == NULL)
	// 	return NULL;

	// save the head of the queue
	node *tmp = L->head;
	// save the result that we gonna return
	Pair result = tmp->p1;

	// take it off
	L->head = L->head->next;
	if (L->head == NULL)
	{
		L->tail = NULL;
	}
	free(tmp);
	return result;
}

bool remove_in_pos(linked_list *L, int i)
{
	if (L->head == NULL)
		return false;
	int pos = 1;
	node *tmp = L->head;
	node *temp = L->head->next;
	if (i == 0)
	{
		L->head = L->head->next;
		if (L->head == NULL)
		{
			L->tail = NULL;
		}
		printf("we remove pair %d & %d \n", tmp->p1.first, tmp->p1.second);
		free(tmp);
		return true;
	}

	while (tmp != NULL && tmp->next != NULL)
	{
		if (i == pos)
		{
			tmp->next = temp->next;
			if (L->tail == temp)
			{
				L->tail = tmp;
			}
			printf("we remove pair %d & %d \n", tmp->p1.first, tmp->p1.second);
			free(temp);
			return true;
		}
		pos++;
		tmp = tmp->next;
		temp = tmp->next;
	}
	return false;
}

bool listEmpty(linked_list *L)
{
	if (L->head == NULL)
		return true;
}

int size(linked_list *L)
{
	int count = 0;
	node *tmp = L->head;
	while (tmp != NULL)
	{
		count++;
		tmp = tmp->next;
	}
	return count;
}

void traverse(linked_list *L)
{
	node *tmp = L->head;
	while (tmp != NULL)
	{
		tmp = tmp->next;
	}
}

void SortLinkedList(linked_list *L)
{
	node *nod = NULL, *temp = NULL;
	Pair tempvar; // temp variable to store node data
	nod = L->head;
	// temp = node;//temp node to hold node data and next link
	while (nod != NULL)
	{
		temp = nod;
		while (temp->next != NULL) // travel till the second last element
		{
			if (temp->p1.first > temp->next->p1.first) // compare the data of the nodes
			{
				tempvar = temp->p1;
				temp->p1 = temp->next->p1; // swap the data
				temp->next->p1 = tempvar;
			}
			temp = temp->next; // move to the next element
		}
		nod = nod->next; // move to the next node
	}
}

// Process *findP(linked_list *L, Process p1)
// {
// 	node *tmp = L->head;
// 	while (tmp != NULL)
// 	{
// 		if (tmp->p1.PID == p1.PID)
// 			return &tmp->p1;
// 		tmp = tmp->next;
// 	}
// 	return NULL;
// }
// Process *retriveP(linked_list *L, int pid)
// {
// 	node *tmp = L->head;
// 	while (tmp != NULL)
// 	{
// 		if (tmp->p1.PID == pid)
// 			return &tmp->p1;
// 		tmp = tmp->next;
// 	}
// 	return NULL;
// }
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

linked_list *memory;
linked_list *memoryhead;
const int sized = 11;
ht_t *map;
// allocate and deallocate
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool deallocate(int id)
{

	bool merge = false;
	bool de = false;
	// If no such starting address available
	// if(mp.find(id) == mp.end())
	// {
	//     //cout << "Sorry, invalid free request\n";
	//     return;
	// }
	if (ht_get(map, id) == -1)
	{
		printf("there is no such process \n");
		return false;
	}

	// Size of block to be searched
	// int n = ceil(log(mp[id]) / log(2));
	int n = ceil(log(ht_get(map, id)) / log(2));
	int i, buddyNumber, buddyAddress;

	// Add the block in free list
	Pair p;
	p.first = id;
	p.second = id + pow(2, n) - 1;
	insert(&memory[n], p);
	printf("deallocation done from %d to  %d\n", id, p.second);
	// memory[n].push_back(make_pair(id,id + pow(2, n) - 1));
	//  cout << "Memory block from " << id
	//       << " to "<< id + pow(2, n) - 1
	//       << " freed\n";

	// Calculate buddy number
	buddyNumber = id / ht_get(map, id);
	// buddyNumber = id / mp[id];

	if (buddyNumber % 2 != 0)
		buddyAddress = id - pow(2, n);
	else
		buddyAddress = id + pow(2, n);

	node *tmp = memory[n].head;
	int j = 0;
	while (tmp != NULL)
	{
		if (tmp->p1.first == buddyAddress)
		{
			if (buddyNumber % 2 == 0)
			{

				Pair p0;
				p0.first = id;
				p0.second = id + 2 * (pow(2, n)) - 1;
				printf("Coalescing of blocks starting at %d and %d was done \n", p0.first, p0.second);
				insert(&memory[n + 1], p0);
			}
			else
			{

				Pair p0;
				p0.first = buddyAddress;
				p0.second = buddyAddress + 2 * (pow(2, n)) - 1;
				printf("Coalescing of blocks starting at %d and %d was done \n", p0.first, p0.second);
				insert(&memory[n + 1], p0);
			}
			SortLinkedList(&memory[n + 1]);
			remove_in_pos(&memory[n], j);
			remove_in_pos(&memory[n], size(&memory[n]) - 1);
			printf("Siuuuuuuuuuuu\n");
			merge = true;
			de = true;
			break;
		}

		tmp = tmp->next;
		j++;
	}
	//printf("Siuuuuuuuuuuu222\n");

	ht_del(id, map);
	//printf("Siuuuuuuuuuuu333\n");
	n++;

	while (n <= 10 && merge)
	{
		merge = false;
		tmp = memory[n].head;
		for (int k = 0; k < size(&memory[n]) / 2; k++)
		{
			int sze = pow(2, n);
			int start = tmp->p1.first;
			// Calculate buddy number
			buddyNumber = start / sze;
			// buddyNumber = id / mp[id];

			if (buddyNumber % 2 != 0)
				buddyAddress = start - pow(2, n);
			else
				buddyAddress = start + pow(2, n);

			node *temp = tmp->next;
			int z = k + 1;
			while (temp != NULL)
			{
				if (temp->p1.first == buddyAddress)
				{
					if (buddyNumber % 2 == 0)
					{

						Pair p0;
						p0.first = start;
						p0.second = start + 2 * (pow(2, n)) - 1;
						printf("Coalescing22 of blocks starting at %d and %d was done \n", p0.first, p0.second);
						insert(&memory[n + 1], p0);
					}
					else
					{
						Pair p0;
						p0.first = buddyAddress;
						p0.second = buddyAddress + 2 * (pow(2, n)) - 1;
						printf("Coalescing22 of blocks starting at %d and %d was done \n", p0.first, p0.second);
						insert(&memory[n + 1], p0);
					}
					merge = true;
					SortLinkedList(&memory[n + 1]);
					remove_in_pos(&memory[n], k);
					remove_in_pos(&memory[n], size(&memory[n]) - 1);
					break;
				}
				z++;
				temp = temp->next;
			}
			tmp = tmp->next;
			n++;
		}
		
	}
	printf("We did deallocate\n");
	return de;

}

bool allocate(int sz, int *start)
{
	//printf("Hello World!\n");

	// Calculate index in free list
	// to search for block if available
	int n = ceil(log(sz) / log(2));
	if (n > 10)
		return false;

	// Block available
	if (size(&memory[n]) > 0)
	{
		memory = memoryhead;
		Pair temp = removehead(&memory[n]);
		printf("Memory from %d to %d allocated\n", temp.first, temp.second);

		// map starting address with
		// size to make deallocating easy
		// map[temp.first] = temp.second - temp.first + 1;
		*start = temp.first;
		ht_set(map, temp.first, temp.second - temp.first + 1);
		return true;
	}
	else
	{
		memory = memoryhead;
		int i;
		for (i = n + 1; i < sized; i++)
		{

			// Find block size greater than request
			if (size(&memory[i]) != 0)
			{

				break;
			}
		}

		// If no such block is found
		// i.e., no memory block available
		if (i == sized)
		{
			//printf("Sorry, failed to allocate memory \n");
			return false;
		}

		// If found
		else
		{
			Pair temp;
			memory = memoryhead;
			temp = removehead(&memory[i]);

			i--;

			for (; i >= n; i--)
			{

				// Divide block into two halves
				Pair pair1, pair2;
				pair1.first = temp.first;
				pair1.second = temp.first + (temp.second - temp.first) / 2;

				pair2.first = temp.first + (temp.second - temp.first + 1) / 2;
				pair2.second = temp.second;

				memory = memoryhead;
				 printf("the first half is start from %d to %d \n", pair1.first, pair1.second);
				insert(&memory[i], pair1);

				// Push them in free list
				memory = memoryhead;
				 printf("the second half is start from %d to %d \n", pair2.first, pair2.second);
				insert(&memory[i], pair2);
				memory = memoryhead;
				temp = removehead(&memory[i]);
			}
			*start = temp.first;
			printf("Memory from %d to %d allocated\n", temp.first, temp.second);

			// map[temp.first] = temp.second - temp.first + 1;
			ht_set(map, temp.first, temp.second - temp.first + 1);
			return true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
