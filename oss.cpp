/* CS4760 Project 3:Message Passing and Operating System Simulator
 * Author: Daniel Eggers
 * Instructor: Mark Hauschild
 * Date: October 20, 2020 
 */

#include <iostream>
#include <getopt.h>     //for cmdl parsing
#include <stdlib.h>     //for atoi
#include <cstdio>       //for printf
#include <string.h>       //for cpp strings
#include <unistd.h>     //for fork & exec
#include <sys/wait.h>   //for wait
#include <sys/msg.h>    //for message queue
#include <errno.h>      //for ENOMSG, etc
//#include <<cstdlib>
//#include <cstring>

#include <shmfunctions.h>

using namespace std;


/* Globals */
const int MAX_C (100);               //max allowed value for option -c
const char* THIS_FILE = "oss";       //filename for shared memory association
int BLOCK_SIZE = (sizeof(int) * 3);  //shared memory block size


typedef struct {
  long mtype;
  char mtext[80];
} mymsg_t;

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


/* Fork-Exec to launch child process */
int fexe() {
  pid_t mypid = fork();
  if (mypid == -1) {
    perror("perror: fork failed");
  }
  else if (mypid >= 0) {
    //fork successful
    //TODO increment pr_count
  }
  if (mypid == 0) {
    //child process
    char *args[2] = { (char*)("./user"), NULL};
    
    if ( execv(args[0], args) == -1) {
      perror("perror: execv fail");
      return(-1);
    }
  }

  return 0;
}


/* cleanup, terminate self and all child processes  */
int kms(int msqid) {
  //TODO terminate child processes before self //TODO have a timeout period
  cout << "oss is terminating (waiting for children)..." << endl;
  pid_t wpid;
  int status = 0;
  while( (wpid = wait(&status)) > 0 );
  cout << "oss: no more children." << endl;
  
  /* Free Shared Memory */
  cout << "oss: now freeing memory..." << endl;
  if ( shmctl(getSharedMemory(THIS_FILE, BLOCK_SIZE), IPC_RMID, NULL) == IPC_RESULT_ERROR ) {
    perror("perror: shmctl");
    return 1;
  }
  else { 
    printf("oss: shared Memory freed.\n");
  }

  /* Free Message Queue */
  cout << "oss: now removing message queue..." << endl;
  if ( msgctl(msqid, IPC_RMID, NULL) == IPC_RESULT_ERROR ) {
    perror("perror: oss msgctl");
    return 1;
  }
  else {
    printf("oss: message queeue destroyed.\n");
  }

  cout << "oss: termination successful." << endl;
  return 0;
}


/* ****************************************** *
 *              MAIN FUNCTION                 *
 * *******************************************/
int main(int argc, char *argv[]) {

  /* cmdl args */
  int option;              //getopt option
  int pr_max = 5;          //max number of child processes spawned (def: 5) 
  string logfile = "log";  //filename for log file
  int ttk = 20;            //time to kill self and all children (def: 20)

  /* shared memory values */
  int shm_clock_s = 0;     //shared clock (seconds)
  int shm_clock_ns = 0;    //shared clock (nanoseconds)
  int shm_pid = 0;         //shared user process id for termination 

  cout << "oss: hello world" << endl;

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

  
  /** OSS Process **/
  
  /* Allocate and attach to shared memory */
  int *shm_array; //shm pointer
  shm_array = (int*)attachSharedMemory(THIS_FILE, BLOCK_SIZE);
  cout << "oss: shm created" << endl; 
  if (shm_array == NULL) {
      printf("Error: couldn't get memory block.\n");
    return 1;
  }
  //cout << "Pointer: " << shm_array << endl;
  //cout << "Pointer: " << *shm_array << endl;
  //cout << "Pointer: " << &shm_array << endl;
  
  //TODO define clock and get it into shared memory
  //cout << "Let's write into shared memory: 4, 5, 6:" << endl;
  shm_array[0] = 4;
  shm_array[1] = 5;
  shm_array[2] = 6;

  //cout << "Now let's read from shared memory: ";
  //printf("%d ", shm_array[0] );
  //printf("%d ", shm_array[1] );
  //printf("%d\n", shm_array[2] );


  fexe(); // fork, exec
  

  



  /* Message Queues */ 
  int msqid; 
  key_t msqkey = 1984;  //TODO arbitrary value, use ftok
  mymsg_t mymsg;
  int msqsize;

  msqid = msgget(msqkey, PERM | IPC_CREAT);
  if (msqid == IPC_RESULT_ERROR) {
    perror("oss msgget");
  }
  else {
    cout << "oss: msgget successful" << endl;
  }
  sleep(1);
  
  //send message
  strcpy( mymsg.mtext, "hello user" );  
  if ( msgsnd(msqid, &mymsg, strlen(mymsg.mtext) + 1, IPC_NOWAIT) == IPC_RESULT_ERROR ) {
    perror("oss msgsnd");
  }
  printf("oss send: %s\n", mymsg.mtext);
  sleep(4);
    
  //receive message 
  if ( msgrcv( msqid, &mymsg, 80, 0, MSG_NOERROR) == IPC_RESULT_ERROR ) {
    perror("oss msgrcv");
  }
  else {
    cout << "oss received message: " << mymsg.mtext << endl;
  }


  cout << "oss: awake for critical section" << endl;
  /* CRITICAL SECTION */
  bool hasFlag = true; //master starts with flag (user should default to false)
  strcpy(mymsg.mtext, "FLAG");
  long mtype_flag = 3;
  mymsg.mtype = mtype_flag;  //distinguish flag messages from earlier test messages
  
  for(int i=0; i<3; i++) {
    sleep(1);
    cout << "oss: loop number: " << i << endl;
    
    //parbegin
    while (hasFlag == false) {
      //cout << "oss: enter while (hasFlag == false)" << endl;
      if ( msgrcv(msqid, &mymsg, 80, mtype_flag, IPC_NOWAIT) == IPC_RESULT_ERROR ) {
        perror("oss msgrcv");
        sleep(1); //pause before trying msgq again
      }
      else {
        cout << "oss: flag obtained." << mymsg.mtext << endl;
        hasFlag = true;
      }
    }
    //cout << "oss: exit while (hasFlag == false) and entering innermost critsec" << endl;

    //CRITICAL
    if (hasFlag) {
      //cout << "oss: we're in" << endl;
      cout << "oss: shared memory read: " << shm_array[2] << endl;
      shm_array[2] = 11;
      cout << "oss: shared memory write: " << shm_array[2] << endl;
      
      //parend
      if ( msgsnd(msqid, &mymsg, strlen(mymsg.mtext) + 1, IPC_NOWAIT) == IPC_RESULT_ERROR ) {
        perror("oss msgsnd");
        exit(1); //TODO kms
      }
      else {
        cout << "oss: flag given up: " << mymsg.mtext << endl;
        hasFlag = false;
      }
    }
  } // i-loop
   


  /* Detach from Shared Memory */
  //cout << "Now detatching." << endl;
  if (shmdt((void *)shm_array) == IPC_RESULT_ERROR) {
    perror("perror: shmdt");
  }

  return kms(msqid);
}
