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
 * Defines the struct "process" with a name, total 
 * runtime, state, current instruction number, and
 * number of cycles the process has been on the cpu for
 *****************************************************/
typedef struct {
	char name[15];
	int totalRuntime;
	int state;//0 = new, 1 = ready, 2 = end
	int instructionNumber;
	int numberOfCycles;
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
 * createProcess() creates a process given the name
 * of the program given by the user.
 *****************************************************/
 process createProcess(char programName[], FILE *openFile){
	 
	 process currentProcess = {"", 0, 0, 0, 0,};
	 char buff[15];
	 char* s;
	 s = strcpy(currentProcess.name, programName);
	 fscanf(openFile, "%s", buff);
	 if(strcmp( buff, "Name:") == 0);{
		 fscanf(openFile, "%s", buff);
	 }
	 fscanf(openFile, "%s", buff);
	 if(strcmp( buff, "Total") == 0);{
		 fscanf(openFile, "%s", buff);
		 fscanf(openFile, "%d", buff);
		 int t = (uintptr_t) buff;
		 currentProcess.totalRuntime = (t + rand());
		 currentProcess.state = 0;
	 }
	 return currentProcess;
 }
 
 /******************************************************
 * This is the main function of the program
 *****************************************************/
 int main(int argc, char *argv[]){
	 /*****************************************************
	 * this sets gets the user input process from the 
	 * command line. if there are none, it tells the user
	 * and exits. It also sets the number of system cylces
	 * as 0.
	 *****************************************************/
	 int numProcesses = argc;
	 int allProcessesComplete = 0;
	 if(numProcesses == 0){
		 printf("Please give names of programs as command line arguments.");
		 return 1;
	 }
	 int systemCycles = 0;
	 /*****************************************************
	 * this creates the ready queue which will store the
	 * processes as they are running. then it runs through
	 * and creates the processes for each user specified
	 * program and adds it to the ready queue.
	 *****************************************************/
	 struct Queue* readyQueue = createQueue();
	 int i;
	 for(i=1; i<numProcesses;i++){
		 char programName[sizeof(argv[i])];
		 char* s = strcpy(programName, argv[i]);
		 FILE *openFile = fopen(argv[i], "r" );
		 process newProcess = createProcess(programName, openFile);
		 newProcess.state = 1;
		 enQueue(readyQueue, newProcess);
		 int closeFile = fclose(openFile);
	 }
	 /*****************************************************
	 * this while loop simulates the cpu constantly running
	 * with a process running from the queue and if the end
	 * of the file is reached, it is not added back into 
	 * the queue. otherwise it is added to the back of the
	 * queue and the next process gets its time on the cpu.
	 * for each cylce of the system, the process gets to
	 * run 100 clock time worth of instructions.
	 *****************************************************/
	 int processDone = 0;
	 while((QIsEmpty(readyQueue) == 0)){
		 int clockTime = 100;
		 process currentProcess = deQueue(readyQueue);
		 FILE *openFile = fopen(currentProcess.name, "r" );
		 int i;
		 int currentLine = (currentProcess.instructionNumber + 4);
		 for(i=0; i<currentLine;i++){
			 char buff[50];
			 fgets(buff, 50, openFile);
			 if(strcmp(buff, "EXE") == 0){
				 break;
				 processDone = 1;
			 }
		 }
		 char str[9];
		 int instructionClocks = 0;
		 while(clockTime > 0){
			fscanf(openFile,"%s" "%d", str, instructionClocks);
			clockTime = clockTime - instructionClocks;
		 }
		 currentProcess.numberOfCycles++;
		 if(processDone == 0){
			 currentProcess.numberOfCycles++;
			 enQueue(readyQueue, currentProcess);
		 }
		 else{
			 printf(currentProcess.name," has finnished");
			 currentProcess.state = 2;
			 processDone = 0;
		 }
		 int closeFile = fclose(openFile);
		 systemCycles++;
	 }
	 return 0;
 }