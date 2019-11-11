////////////////////////////////////////////////////////////
//Operating System Simulator
//Description: This program simulates a process
// scheduler using round robin scheduling.
//
//Author: Stephen Kalen
//Class: CMSC 312-001 
//Language: C
///////////////////////////////////////////////////////////

/******************************************************
 * Includes all of the header files that conatain the
 * necessary functions for this program
 *****************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/******************************************************
 * Creates the constants for the program.
 *****************************************************/
const char CRITICALFLAG[4] = "CRIT";// flag to indicate the beginning of a critical section
const char CRITALFLAGEND[7] = "CRITEND";// flag to indicate the end of a critical section
const char EXITFLAG[3] = "EXE";// flag to indicate the end of the program
const int MEMSIZE = 4096;// size of main memory for this system

/******************************************************
 * Defines the struct "process" with a name, total 
 * runtime, ammount of memory it will take up, state, 
 * current instruction number, number of cycles 
 * the process has been on the cpu for, boolean 
 * variable for if process needs a critical section,
 * and boolean variable for if a process is in a 
 * critical section
 *****************************************************/
typedef struct {
	char name[15];
	int totalRuntime;
	int memory;
	int state;//0 = new, 1 = ready, 2 = waiting, 3 = finished
	int instructionNumber;
	int numberOfCycles;
	int needsCriticalSection;//0 = false, 1 = true
	int inCriticalSection;//0 = false, 1 = true
}process;

/******************************************************
 * defines the struct for a node in the queue
 * 
 * The implementation of a queue is based on the
 * explaination here: 
 * https://www.geeksforgeeks.org/queue-linked-list-implementation/
 *****************************************************/
 struct QNode { 
	process pro;
    struct QNode* next; 
}; 

/******************************************************
 * defines the queue struct
 *****************************************************/
struct Queue { 
    struct QNode *front, *rear; 
}; 

/******************************************************
 * newNode() creates a new node to be added to the 
 * queue
 *****************************************************/
struct QNode* newNode(process p) { 
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode)); 
	temp->pro = p;
    temp->next = NULL; 
    return temp; 
} 

/******************************************************
 * createQueue() creates a queue 
 *****************************************************/
struct Queue* createQueue() { 
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue)); 
    q->front = q->rear = NULL; 
    return q; 
} 

/******************************************************
 * enQueue() adds a node (process) to the queue
 *****************************************************/
void enQueue(struct Queue* q, process p) { 
    struct QNode* temp = newNode(p); 
    if (q->rear == NULL) { 
        q->front = q->rear = temp; 
        return; 
    } 
    q->rear->next = temp; 
    q->rear = temp; 
} 

/******************************************************
 * deQueue() removes the node at the front of the Queue
 * and returns the process held there
 *****************************************************/
process deQueue(struct Queue* q) { 
    struct QNode* temp = q->front; 
    free(temp); 
    q->front = q->front->next; 
    if (q->front == NULL) 
        q->rear = NULL; 
    return temp->pro; 
} 

/******************************************************
 * QIsEmpty() returns 1 if the queue is empty and 0 if
 * it isn't
 *****************************************************/
int QIsEmpty(struct Queue* q){
	if (q->front == NULL) 
        return 1;
	else
		return 0;
}

/******************************************************
 * QFront() returns the first process in the queue
 * without dequeueing it
 *****************************************************/
process QFront(struct Queue* q){
	if(q->front != NULL)
	   return q->front->pro;
 }

/******************************************************
 * createProcess() creates a process given the name
 * of the program given by the user and returns it
 *****************************************************/
process createProcess(char programName[], FILE *openFile){
	 process currentProcess;
	 char buff[15];
	 int numbuff;
	 fscanf(openFile, "%s", buff);
	 if(strcmp(buff, "Name:") == 0);
		 fscanf(openFile, "%s", currentProcess.name);
	 fscanf(openFile, "%s", buff);
	 if(strcmp(buff, "Total") == 0);{
		 fscanf(openFile, "%s", buff);
		 fscanf(openFile, "%d", numbuff);
		 currentProcess.totalRuntime = (numbuff + rand());
	 }
	 fscanf(openFile, "%s", buff);
	 if(strcmp(buff, "Memory:") == 0);{
		 fscanf(openFile, "%d", numbuff);
		 currentProcess.memory = (numbuff + rand());
		}
	 currentProcess.instructionNumber = 1;
	 currentProcess.state = 0;
	 currentProcess.inCriticalSection = 0;
	 currentProcess.needsCriticalSection = 0;
	 return currentProcess;
 }
 
/******************************************************
 * Creates the necessary variables for the following
 * functions
 *****************************************************/
struct Queue* readyQueue;
struct Queue* waitingQueue;
int waitingQueueSize;
int lockAvailable;
struct Queue* lockQueue;
int systemCycles;

/******************************************************
 * mutexLock() allows a process to aquire the lock and
 * run instructions in the critical section of the 
 * process. It also returns the remaining time it has
 * on the cpu clock. If none left, it returns 0.
 *****************************************************/ 
int mutexLock(process currentProcess, FILE *openFile, int clockTime){
	 lockAvailable = 0;
	 char str[9];
	 int instructionClocks = 0;
	 while(clockTime > 0){
		 fscanf(openFile,"%s" "%d", str, instructionClocks);
		 currentProcess.instructionNumber++;
		 if(strcmp(str, CRITALFLAGEND) == 0){
			 currentProcess.needsCriticalSection = 0;
			 currentProcess.inCriticalSection = 0;
			 break;
		 }
		 else
			clockTime = clockTime - instructionClocks;
	 }
		 if(currentProcess.needsCriticalSection == 1){
			 enQueue(readyQueue, currentProcess);
			 return 0;
		 }
		 else{
			 lockAvailable = 1;
			 return clockTime;
		 }
 }

/******************************************************
 * checkWaitingQueue() checks all the processes in the
 * waiting queue and if there is room in main memory, 
 * it will add processes to main memory in the order
 * they were added to the queue
 *****************************************************/
void checkWaitingQueue(int memUsed){
	 int i;
	 for(i=0;i<waitingQueueSize;i++){
		process waitingProcess = deQueue(waitingQueue);
		 if((waitingProcess.memory + memUsed) > MEMSIZE){
			 enQueue(waitingQueue, waitingProcess);
			 waitingQueueSize++;
		 }
		 else{
			 enQueue(readyQueue, waitingProcess);
			 waitingQueueSize--;
		 }
	 }
 }

/******************************************************
 * checkMemory() checks if a given process will fit 
 * into main memory, adds it to the ready queue if it 
 * will or the waiting queue if it cant, and retuns 
 * the new size of main memory
 *****************************************************/
 int checkMemory(process p, int memUsed){
	 if((p.memory + memUsed) > MEMSIZE){
		 p.state = 2;
		 enQueue(waitingQueue, p);
		 waitingQueueSize++;
		 return memUsed;
	 }
	 else{
		 p.state = 1;
		 enQueue(readyQueue, p);
		 return memUsed + p.memory;
	}
 }
 
 /******************************************************
 * This is the main function of the program
 *****************************************************/
 int main(int argc, char *argv[]){
	 /*****************************************************
	 * this sets gets the user input process from the 
	 * command line. if there are none, it tells the user
	 * and exits. It also sets the number of system cylces
	 * as 0 and sets the ammount of used memory at 0.
	 *****************************************************/
	 int numProcesses = argc;
	 int allProcessesComplete = 0;
	 if(numProcesses == 0){
		 printf("Please give names of programs as command line arguments.");
		 return 1;
	 }
	 systemCycles = 0;
	 int currentMemUsed = 0;
	 /*****************************************************
	 * this creates the ready queue which will store the
	 * processes as they are running, the waiting queue for 
	 * storing processes that cant fit into main memory, 
	 * and the lock queue for processes that are waiting to 
	 * aquire the mutex lock. Then it runs through and 
	 * creates the processes for each user specified program 
	 * and adds it to the appropriate queue.
	 *****************************************************/
	 readyQueue = createQueue();
	 waitingQueue = createQueue();
	 lockQueue = createQueue();
	 lockAvailable = 1;// 1 if available, 0 if not.
	 int i;
	 for(i=1; i<numProcesses;i++){
		 char programName[sizeof(argv[i])];
		 char* s = strcpy(programName, argv[i]);
		 FILE *openFile = fopen(argv[i], "r" );
		 process newProcess = createProcess(programName, openFile);
		 currentMemUsed = checkMemory(newProcess, currentMemUsed);
		 int closeFile = fclose(openFile);
	 }
	 /*****************************************************
	 * This while loop simulates the cpu constantly running
	 * with a process from the queue and if the end
	 * of the file is reached, it is not added back into 
	 * the queue. otherwise it is added to the back of the
	 * queue and the next process gets its time on the cpu.
	 * for each cylce of the system, the process gets to
	 * run 100 clock time worth of instructions. The loop
	 * runs until there are no processes left in the ready
	 * queue.
	 *****************************************************/
	 while((QIsEmpty(readyQueue) == 0)){
		 int clockTime = 100;
		 /****************************************
		  * checks each process in the waiting
		  * queue to see if it can fit into main
		  * memory
		  ***************************************/
		 if(QIsEmpty(waitingQueue) == 0)
			checkWaitingQueue(currentMemUsed);
		 /****************************************
		  * gets process from ready queue and
		  * opens the corresponding file
		  ***************************************/
		 process currentProcess = deQueue(readyQueue);
		 FILE *openFile = fopen(currentProcess.name, "r" );
		 /****************************************
		  * continues running a process with a
		  * mutex lock if it is in a critical
		  * section or checks if it can aquire
		  * the mutex lock if it needs to
		  ***************************************/
		 if(currentProcess.inCriticalSection){
				 clockTime = mutexLock(currentProcess, openFile, clockTime);
			 }
			 else if(currentProcess.needsCriticalSection == 1){
				 if(lockAvailable == 1 && strcmp(QFront(lockQueue).name, currentProcess.name) == 0){
					 clockTime = mutexLock(currentProcess, openFile, clockTime);
				 }
			 }
		 /****************************************
		  * Reads through the file to the place
		  * it left off on last time it was on
		  * the cpu so that the os reads from
		  * the correct instruction.
		  ***************************************/
		 int i;
		 int currentLine = (currentProcess.instructionNumber + 4);
		 char buff[15];
		 for(i=0; i<currentLine;i++){
			 fgets(buff, 15, openFile);
		 }
		 /****************************************
		  * This loop simulates the cpu running 
		  * with a round robin scheduler where
		  * each process can run for 100 cpu
		  * clock time, then it loads the next
		  * process in the queue. When reading 
		  * the file, the os checks for critical
		  * sections and the end of the file.
		  ***************************************/
		 char str[15];
		 int instructionClocks = 0;
		 while(clockTime > 0){
			 fscanf(openFile,"%s" "%d", str, instructionClocks);
			 currentProcess.instructionNumber++;
			 if(strcmp(str, EXITFLAG) == 0){
				 currentProcess.state = 3;
				 break;
			 }
			 else if(strcmp(str, CRITICALFLAG) == 0){
				 currentProcess.needsCriticalSection = 1;
				 if(lockAvailable == 1){
					 currentProcess.inCriticalSection = 1;
					 clockTime = mutexLock(currentProcess, openFile, clockTime);
					 if(clockTime <= 0){
						break;
					}
					else
						continue;
				 }
				 else{
					 enQueue(lockQueue, currentProcess);
					 break;
				 }
			 }
			 clockTime = clockTime - instructionClocks;
		 }
		 currentProcess.numberOfCycles++;
		 if(currentProcess.state == 1){
			 enQueue(readyQueue, currentProcess);
		 }
		 else if(currentProcess.state = 3){
			 printf(currentProcess.name," has finnished");
			 currentMemUsed = currentMemUsed - currentProcess.memory;
		 }
		 int closeFile = fclose(openFile);
		 systemCycles++;
	 }
	 printf("All processes have completed their execution! Exiting operating system.");
	 return 0;
 }