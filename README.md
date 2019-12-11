# Stephen_Kalen_CMSC312_2019
Implementation of a simulated operating system

Operating Systems Project Documentation
1. General Description:
This operating system uses a module OS structure with separate modules(methods) for
different parts of the OS that communicate through the main method of the program. The
modules include a round robin scheduling algorithm, a mutex lock critical section
resolving scheme, a process creation module, an external I/O module(simulated with a
random chance for I/O events to occur(keyboard plugged in, etc.)), memory
management module(checkMem method), and modules that simulate communication
with the user.
2. Parameters/Functionalities:
To compile, you can use the make command, or compile Operating_System_Simulator.c
specifically. To run operatingSys.exe, you will need to enter the names of the given
program files(2d_Video_Game.txt, etc.) as command line arguments:
operatingSys.exe 2d_Video_Game.txt 3d_Video_Game.txt Email_Application.txt
Text_Processor.txt
Then the OS will prompt the user for an input with the following choices(all of which are
case sensitive and to be entered as written below, without parameters):
“EXE” - Runs through the OS without stopping.
“PAUSE” - Will prompt the user for a number of system cycles to run before pausing and
will prompt again after.
“NEW” - Will prompt the user for the name of a program file to create a process from.
The name should contain the extension of the file.
Prompts that follow after this will have each of the previous options as well as the
following:
“YIELD” - Stops running the current process, puts it back in the ready queue, and
continues on the next cycle with the next process in the queue.
“OUT” - Prints relevant PCB content for the current process.
3. Requirements:
● (Requirement 1)Process implementation and PCB:
  ○  Lines: 38-48, 156-183
● (2)Critical section within each process:
  ○ (In program files): CRIT, CRITEND
● (3)Critical section resolving scheme:
  ○ Lines: 204-272, 557-564, 620-637
● (8)Scheduler:
  ○ Lines: 613-649
● (14)Basic memory and operations on it:
  ○ Lines: 258-271, 280-291
● (18)I/O interrupts and handlers
  ○ Lines: 297-312, 318-325
● (23)Loading external processes and generating new ones on user request:
  ○ Lines: 156-183, 454-468, 427-441
