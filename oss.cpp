/* CS4760 Project 3:Message Passing and Operating System Simulator
 * Author: Daniel Eggers
 * Instructor: Mark Hauschild
 * Date: October 20, 2020 
 */

#include <iostream>
#include <getopt.h>     //for cmdl parsing
#include <stdlib.h>     //for atoi
#include <cstdio>       //for printf
#include <string>       //for cpp strings
//#include <unistd.h>
//#include <<cstdlib>
//#include <cstring>

#include <shmfunctions.h>

using namespace std;


/* print help message when -h is specified */
void printHelp() {
  printf("%s\n%s\n%s\n%s\n%s\n",
    "Available usage options:",
    "  -h      show this message.",
    "  -c X    max number of child processes spawned (default: 5, max: 100)",
    "  -l X    specify filename for output log (default: \"log\")",
    "  -t X    time in seconds till all processes terminate (default: 20)"
  );
}


/* print advisory messages for minor unexpectancies (non-error, non-fatal) */
void printWarning(int warning_code) {
  cout << "Warning: ";
  switch(warning_code) {
    case 1: 
      cout<< "specified parameter of -c is too high.  Capping at 100." <<endl;
      break;
    default: 
      cout<< "something unexpected happened, but the program will still run." <<endl;
      break;
  }
}


/* cleanup, terminate self and all child processes  */
bool kms() {
  //TODO terminate child processes before self
  cout << "oss is terminating." << endl;
  return 0;
}


int main(int argc, char *argv[]) {

  const int MAX_C (100);               //max allowed value for option -c
  const char* THIS_FILE = "oss";       //filename for shared memory association
  int BLOCK_SIZE = (sizeof(int) * 3);  //shared memory block size

  /* cmdl args */
  int option;              //getopt option
  int pr_max = 5;          //max number of child processes spawned (def: 5) 
  string logfile = "log";  //filename for log file
  int ttk = 20;            //time to kill self and all children (def: 20)

  /* shared memory values */
  int shm_clock_s = 0;     //shared clock (seconds)
  int shm_clock_ns = 0;    //shared clock (nanoseconds)
  int shm_pid = 0;         //shared user process id for termination 

  cout << "oss is running." << endl;

  while ( (option = getopt(argc, argv, "hc:l:t:")) != -1 ) {
  
    switch(option) {
      
      case 'h':
        printHelp();
        break;
      case 'c':
        pr_max = atoi( argv[(optind-1)] );
        cout << pr_max << endl;

        if (pr_max > MAX_C) {
          printWarning(1);
          pr_max = MAX_C;
        }
        break;
      case 'l':
        logfile = argv[(optind-1)];
        cout << logfile << endl;
        break;
      case 't':
        ttk = atoi( argv[(optind-1)] ); 
        cout << ttk << endl;
        break;
      case '?':
        //TODO
        return 1;
    }
  
  }

  
  /* OSS Process  */
  int *shm_array;

  shm_array = (int*)attachSharedMemory(THIS_FILE, BLOCK_SIZE);
   
  if (shm_array == NULL) {
    printf("Error: couldn't get memory block.\n");
    return 1;
  }
  cout << "Pointer: " << shm_array << endl;
  cout << "Pointer: " << *shm_array << endl;
  cout << "Pointer: " << &shm_array << endl;
  
  //TODO define clock and get it into shared memory
  //cout << "Let's write into shared memory: " << endl;
  shm_array[0] = 4;
  shm_array[1] = 5;
  shm_array[2] = 6;

  cout << "Now let's read from shared memory: " << endl;
  printf("%d\n", shm_array[0] );
  printf("%d\n", shm_array[1] );
  printf("%d\n", shm_array[2] );

  //cout << "Now detatching." << endl;
  //detachSharedMemory(shm_array);


  return 0;
}
