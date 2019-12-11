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
#include <time.h>

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
	char name[25];//name of program
	int totalRuntime;//approximation of how long the process will take to execute
	int memory;//size of the process in memory
	int state;//0 = new, 1 = ready, 2 = waiting, 3 = finished, 4 = running
	int ioWaitTime;//-1 = No IO operation running, 0 = IO operation complete, positive int if IO operation is needed
	int instructionNumber;// number of instructions ran by the process
	int numberOfCycles;//number of cycles that the process ran on the cpu
	int needsCriticalSection;//0 = false, 1 = true
	int inCriticalSection;//0 = false, 1 = true
}process;

 struct QNode { 
	process pro;
    struct QNode* next; 
	struct QNode* prev;
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
	temp->prev = NULL;
    return temp; 
} 

/******************************************************
 * createQueue() creates a queue 
 *****************************************************/
struct Queue* createQueue() { 
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue)); 
    q->front = NULL; 
	q->rear = q->front;
    return q; 
} 

/******************************************************
 * enQueue() adds a node (process) to the queue
 *****************************************************/
void enQueue(struct Queue* q, process p) { 
    struct QNode* temp = newNode(p); 
    if (q->rear == NULL) { 
        q->front = temp; 
		q->rear = temp;
        return; 
    } 
	if (q->rear->next == NULL) {
		temp->next = q->front;
		q->rear = temp;
		q->front->prev = q->rear;
		return;
	}
	temp->next = q->rear;
    q->rear = temp; 
	q->rear->next->prev = q->rear;
} 

/******************************************************
 * deQueue() removes the node at the front of the Queue
 * and returns the process held there
 *****************************************************/
process deQueue(struct Queue* q) { 
	struct QNode* temp = q->front; 
	q->front = temp->prev;
	process returnProcess = temp->pro;
    free(temp); 
    if (q->front == NULL) 
        q->rear = NULL; 
	else {
		q->front->next = NULL;
	}
    return returnProcess;
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

int randomNum1000(int num) {
	int random = rand() % 1000;
	num += random;
	return num;
}

int numProcesses;
int numTotalProcesses;

/******************************************************
 * createProcess() creates a process given the name
 * of the program given by the user and returns it
 *****************************************************/
process createProcess( FILE *openFile){
	 process currentProcess;
	 currentProcess.ioWaitTime = -1;
	 currentProcess.numberOfCycles = 0;
	 currentProcess.instructionNumber = 0;
	 currentProcess.inCriticalSection = 0;
	 currentProcess.needsCriticalSection = 0;
	 char buff[15];
	 int numbuff;
	 fscanf(openFile, "%s", buff);
	 if(strcmp(buff, "Name:") == 0);
		 fscanf(openFile, "%s", currentProcess.name);
	 fscanf(openFile, "%s", buff);
	 if(strcmp(buff, "Total") == 0);{
		 fscanf(openFile, "%s", buff);
		 fscanf(openFile, "%d", &numbuff);
		 currentProcess.totalRuntime = randomNum1000(numbuff);
	 }
	 fscanf(openFile, "%s", buff);
	 if(strcmp(buff, "Memory:") == 0);{
		 fscanf(openFile, "%d", &numbuff);
		 currentProcess.memory = randomNum1000(numbuff);
		}
	 currentProcess.state = 0;
	 numProcesses++;
	 numTotalProcesses++;
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
int currentMemUsed;
int nextProcess;
int currentPauseCycleNum;

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
		 fscanf(openFile,"%s" "%d", str, &instructionClocks);
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
	 int k;
	 for(k=0;k<waitingQueueSize;k++){
		process waitingProcess = deQueue(waitingQueue);
		if(waitingProcess.ioWaitTime > -1){
			enQueue(waitingQueue, waitingProcess);
		}
		else if((waitingProcess.memory + memUsed) > MEMSIZE){
			enQueue(waitingQueue, waitingProcess);
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
 * the new size of main memory.
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
 * checkMemory2() checks if a given process will fit
 * into main memory (only during user prompts), adds 
 * it to the ready queue if it will or the waiting 
 * queue if it cant, and changes size of memory 
 * accordingly.
 *****************************************************/
 void checkMemory2(process p) {
	 if ((p.memory + currentMemUsed) > MEMSIZE) {
		 p.state = 2;
		 enQueue(waitingQueue, p);
		 waitingQueueSize++;
	 }
	 else {
		 p.state = 1;
		 enQueue(readyQueue, p);
		 currentMemUsed += p.memory;
	 }
 }

 /******************************************************
 * ioWaitingTimer() takes the io time from a given
 * process and waits that long in cpu time.
 *****************************************************/
 void ioWaitingTimer(int remainingClockTime){
	 int timePassed = 100 - remainingClockTime;
	 int l;
	 for (l = 0; l < waitingQueueSize; l++) {
		 process waitingProcess = deQueue(waitingQueue);
		 if(waitingProcess.ioWaitTime > 0) {
			 waitingProcess.ioWaitTime = waitingProcess.ioWaitTime - timePassed;
			 if (waitingProcess.ioWaitTime <= 0) {
				 waitingProcess.ioWaitTime = -1;
				 enQueue(readyQueue, waitingProcess);
				 return;
			 }
		 }
		 enQueue(waitingQueue, waitingProcess);
	 }
 }

 /******************************************************
 * randomEvent() runs and has a random chance of
 * an IO event occuring.
 *****************************************************/
 void randomEvent() {
	 int random = rand() % 12;
	 int random2 = rand() % 4;
	 char devices[4][20] = {"Keyboard", "Mouse", "Printer", "Monitor"};
	 if(random == 5) {
		 printf("I/O device detected: %s\n", devices[random2]);
	 }
 }

 /******************************************************
 * userPromptFirst() prompts the user before running
 * any processes with three options. EXE, PAUSE, and 
 * NEW.
 *****************************************************/
 int userPromptFirst() {
	 int loop = 1;
	 int pauseCycles = -1;
	 while (loop == 1) {
		 printf("%d Processes are set to be run on the OS.\n", numTotalProcesses);
		 printf("Please give an input:\n");
		 printf("	-'EXE': Runs through the OS normally to completeion\n");
		 printf("	-'PAUSE': Prompts user for number of cycles to pause while running the OS\n");
		 printf("	-'NEW': Prompts user for name of another process to be added to the scheduler\n\n");
		 char input[25];
		 fgets(input, 25, stdin);
		 printf("\n");
		 if (strcmp(input, "EXE\n") == 0) {
			 return pauseCycles;
		 }
		 else if (strcmp(input, "PAUSE\n") == 0) {
			 printf("Please give number of cycles to run before pausing:\n\n");
			 gets(input, 25, stdin);
			 sscanf(input, "%d", &pauseCycles);
			 printf("\n");
			 return pauseCycles;
		 }
		 else if (strcmp(input, "NEW\n") == 0) {
			 printf("Please input full name of program to turn into process:\n\n");
			 gets(input, 25, stdin);
			 printf("\n");
			 FILE* openFile = fopen(input, "r");
			 if (openFile == NULL) {
				 printf("Error opening program: %s\n", input);
				 continue;
			 }
			 process newProcess = createProcess(openFile);
			 int closeFile = fclose(openFile);
			 if (newProcess.state == 0) {
				 checkMemory2(newProcess);
			 }
		 }
		 else {
			 printf("Please enter a valid command.\n");
		 }
		 free(input);
	 }
 }

 /******************************************************
 * printProcessInfo() takes a process and prints all 
 * relevant info in the PCB.
 *****************************************************/
 void printProcessInfo(process currentProcess) {
	 printf("Current Process Info:\n");
	 printf("	Name: %s\n", currentProcess.name);
	 printf("	Total Runtime: %d\n", currentProcess.totalRuntime);
	 printf("	Memory: %d\n", currentProcess.memory);
	 printf("	Process State: %d\n", currentProcess.state);
	 printf("	Instruction Number: %d\n", currentProcess.instructionNumber);
	 printf("	Number of Cycles On CPU: %d\n", currentProcess.numberOfCycles);
	 printf("	Needs Critical Section: %d\n", currentProcess.needsCriticalSection);
	 printf("	In Critical Section: %d\n\n", currentProcess.inCriticalSection);
 }

 /******************************************************
 * userPrompt() prompts the user durring OS execution
 * with three options. EXE, PAUSE, YIELD, OUT, and NEW.
 *****************************************************/
 int userPrompt(int pauseCycles, process currentProcess, int systemCycles) {
	 int loop = 1;
	 while (loop == 1) {
		 printf("%d System cycles have occured. Please give an input:\n", systemCycles);
		 printf("	-'EXE': Runs through the os normally to completeion\n");
		 printf("	-'PAUSE': Prompts user for number of cycles to pause while running the OS\n");
		 printf("	-'YIELD': Stops current process from its cycle time and lets next process in ready queue take its place\n");
		 printf("	-'OUT': Prints information about current process.\n");
		 printf("	-'NEW': Prompts user for name of another process to be added to the scheduler\n\n");
		 char input[10];
		 fgets(input, 10, stdin);
		 printf("\n");
		 if (strcmp(input, "EXE\n") == 0) {
			 return -1;
		 }
		 else if (strcmp(input, "YIELD\n") == 0) {
			 printf("%s is now being skipped for this system cycle.\n", currentProcess.name);
			 nextProcess = 1;
			 return pauseCycles;
		 }
		 else if (strcmp(input, "PAUSE\n") == 0) {
			 printf("Please give new number of cycles to run before pausing:\n\n");
			 fgets(input, 10, stdin);
			 sscanf(input, "%d", &pauseCycles);
			 printf("\n");
			 return pauseCycles;
		 }
		 else if (strcmp(input, "OUT\n") == 0) {
			 printProcessInfo(currentProcess);
			 continue;
		 }
		 else if (strcmp(input, "NEW\n") == 0) {
			 printf("Please input full name of program to turn into process:\n\n");
			 gets(input, 25, stdin);
			 printf("\n");
			 FILE* openFile = fopen(input, "r");
			 if (openFile == NULL) {
				 printf("Error opening program: %s\n", input);
				 continue;
			 }
			 process newProcess = createProcess( openFile);
			 int closeFile = fclose(openFile);
			 if (newProcess.state == 0) {
				 checkMemory2(newProcess);
			 }
		 }
		 else {
			 printf("Please enter a valid command.\n");
		 }
		 free(input);
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
	 * as 0 and sets the ammount of used memory at 0. It
	 * also sets the pauseCycles at its default of -1 and
	 * currentPauseCycleNum at 0;
	 *****************************************************/
	 srand(time(NULL));
	 printf("Starting OS...\n\n");
	 int numPrograms = argc - 1;
	 int allProcessesComplete = 0;
	 if(numPrograms <= 0){
		 printf("Please give names of program files as command line arguments.\n");
		 return 1;
	 }
	 int systemCycles = 0;
	 currentMemUsed = 0;
	 int pauseCycles = -1;
	 currentPauseCycleNum = 0;
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
	 waitingQueueSize = 0;
	 lockQueue = createQueue();
	 lockAvailable = 1;// 1 if available, 0 if not.
	 int i;
	 int badProcesses = 0;
	 for(i=1; i<(numPrograms + 1);i++){
		 char *programName;
		 programName = argv[i];
		 FILE *openFile = fopen(programName, "r" );
		 if (openFile == NULL) {
			printf("Error opening program: %s\n", programName);
			badProcesses++;
			continue;
		 }
		 process newProcess = createProcess( openFile);
		 int closeFile = fclose(openFile);
		 if (newProcess.state != 0) {
			 continue;
		 }
		 currentMemUsed = checkMemory(newProcess, currentMemUsed);
	 }
	 numProcesses = (numProcesses - badProcesses);
	 numTotalProcesses = (numTotalProcesses - badProcesses);
	 /****************************************************
	 * This prompts the user and sets the pauseCycles
	 * variable accordingly
	 ****************************************************/
	 pauseCycles = userPromptFirst();
	 if (pauseCycles > 0) {
		 currentPauseCycleNum = pauseCycles;
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
	 * queue. It also sets the nextProcess variable to 
	 * zero.
	 *****************************************************/
	 while ((QIsEmpty(readyQueue) == 0) || (QIsEmpty(waitingQueue) == 0)) {
		 nextProcess = 0;
		 systemCycles++;
		 int clockTime = 100;
		 /****************************************
		 * checks each process in the waiting
		 * queue to see if it can fit into main
		 * memory.
		 ***************************************/
		 if (QIsEmpty(waitingQueue) == 0) {
			 checkWaitingQueue(currentMemUsed);
		 }
		 /****************************************
		 * Gets process from ready queue and
		 * opens the corresponding file. Sets
		 * the state to running and opens program
		 * file.
		 ***************************************/
		 process currentProcess = deQueue(readyQueue);
		 currentProcess.state = 4;
		 char fileName[30];
		 strcpy(fileName, currentProcess.name);
		 strcat(fileName, ".txt");
		 FILE* openFile = fopen(fileName, "r");
		 /****************************************
		 * continues running a process with a
		 * mutex lock if it is in a critical
		 * section or checks if it can aquire
		 * the mutex lock if it needs to
		 ***************************************/
		 if (currentProcess.inCriticalSection) {
			 clockTime = mutexLock(currentProcess, openFile, clockTime);
		 }
		 else if (currentProcess.needsCriticalSection == 1) {
			 if (lockAvailable == 1 && strcmp(QFront(lockQueue).name, currentProcess.name) == 0) {
				 clockTime = mutexLock(currentProcess, openFile, clockTime);
			 }
		 }
		 /****************************************
		 * Reads through the file to the place
		 * it left off on last time it was on
		 * the cpu so that the os reads from
		 * the correct instruction.
		 ***************************************/
		 int j;
		 int currentLine = (currentProcess.instructionNumber + 4);
		 char buff[50];
		 for(j=0; j<(currentLine);j++){
			 fgets(buff, 50, openFile);
		 }
		 /**************************************
		 * This checks for the correct
		 * circumstances for the user to be
		 * prompted and acts accordingly
		 **************************************/
		 char str[15];
		 int instructionClocks = 0;
		 if(pauseCycles > 0 ){
			 if (currentPauseCycleNum == 1) {
				 pauseCycles = userPrompt(pauseCycles, currentProcess, systemCycles);
				 if (pauseCycles > 0)
					 currentPauseCycleNum = pauseCycles;
				 if (nextProcess == 1) {
					 nextProcess = 0;
					 currentProcess.state = 1;
					 enQueue(readyQueue, currentProcess);
					 int closeFile = fclose(openFile);
					 randomEvent();
					 continue;
				 }
			 }
			 else{
				 currentPauseCycleNum--;
			 }
		 }
		 /****************************************
		 * This loop simulates the cpu running
		 * with a round robin scheduler where
		 * each process can run for 100 cpu
		 * clock time, then it loads the next
		 * process in the queue. When reading
		 * the file, the os checks for critical
		 * sections and the end of the file.
		 * It also runs the random event method
		 * for IO into the system.
		 ***************************************/
		 while (clockTime > 0) {
			 fscanf(openFile, "%s" "%d", str, &instructionClocks);
			 currentProcess.instructionNumber++;
			 if (strcmp(str, EXITFLAG) == 0) {
				 currentProcess.state = 3;
				 break;
			 }
			 else if (strcmp(str, CRITICALFLAG) == 0) {
				 currentProcess.needsCriticalSection = 1;
				 if (lockAvailable == 1) {
					 currentProcess.inCriticalSection = 1;
					 clockTime = mutexLock(currentProcess, openFile, clockTime);
					 if (clockTime <= 0) {
						 break;
					 }
					 else {
						 currentProcess.instructionNumber--;
						 continue;
					 }	 
				 }
				 else {
					 enQueue(lockQueue, currentProcess);
					 break;
				 }
			 }
			 else if(strcmp(str, "IO") == 0){
				 currentProcess.ioWaitTime = instructionClocks;
				 currentProcess.state = 2;
				 break;
			 }

			 if (clockTime - instructionClocks < 0){
				currentProcess.instructionNumber--;
				break;
			 }
			 clockTime = clockTime - instructionClocks;
		 }
		 currentProcess.numberOfCycles++;
		 if(currentProcess.state == 4){
			 currentProcess.state = 1;
			 enQueue(readyQueue, currentProcess);
		 }
		 else if(currentProcess.state == 3){
			 currentMemUsed = currentMemUsed - currentProcess.memory;
			 numProcesses--;
		 }
		 int closeFile = fclose(openFile);
		 ioWaitingTimer(clockTime);
		 randomEvent();
	 }
	 /*************************************
	 * Prints the final data for the OS
	 *************************************/
	 printf("\n");
	 printf("All processes have completed their execution!\n");
	 printf("OS finished %d processes in %d clock cycles.\n", numTotalProcesses, systemCycles);
	 printf("Exiting OS...\n");
	 return 0;
 }