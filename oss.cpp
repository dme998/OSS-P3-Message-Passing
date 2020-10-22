/* CS4760 Project 3:Message Passing and Operating System Simulator
 * Author: Daniel Eggers
 * Instructor: Mark Hauschild
 * Date: October 20, 2020 
 */

#include <iostream>
#include <getopt.h>     //for cmdl parsing
#include <stdlib.h>     //for atoi
#include <cstdio>       //for printf
#include <string.h>     //for cpp strings
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
bool verbose;                        //-v cmdl option
volatile sig_atomic_t gSTOP = 0;     //signal handling: to declare program should stop running
volatile sig_atomic_t gSHM = 0;      //signal handling: to declare that shared memory was created
volatile sig_atomic_t gMSQ = 0;      //signal handling: to declare that message queue was created

typedef struct {
  long mtype;
  char mtext[80];
} mymsg_t;


/* ****************************************** *
 *               FUNCTIONS                    *
 * *******************************************/

/* print help message when -h is specified */
void printHelp() {
  printf("%s\n%s\n%s\n%s\n%s\n%s\n",
    "Available usage options:",
    "  -h      show this message.",
    "  -c X    max number of child processes spawned (default: 5, max: 100)",
    "  -l X    specify filename for output log (default: \"log\")",
    "  -t X    time in seconds till all processes terminate (default: 20)",
    "  -v      verbose: prints a walkthrough of the process from start to finish"
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

/* signal handler */
void handler(int sig) { 
  write(STDOUT_FILENO, " oss: signal received\n", 22);
  gSTOP = 1; 
}


/* cleanup at end of program (not meant for early termination)  */
int endProgram(int msqid) {
  //TODO terminate child processes before self //TODO have a timeout period
  if(verbose){cout << "oss is terminating (waiting for children)..." << endl;}
  pid_t wpid;
  int status = 0;
  while( (wpid = wait(&status)) > 0 );
  if(verbose) {cout << "oss: no more children." << endl;}
  
  /* Free Shared Memory */
  if(verbose){cout << "oss: now freeing memory..." << endl;}
  if ( shmctl(getSharedMemory(THIS_FILE, BLOCK_SIZE), IPC_RMID, NULL) == IPC_RESULT_ERROR ) {
    perror("perror: shmctl");
    return 1;
  }
  else { 
    printf("oss: shared Memory freed.\n");
  }

  /* Free Message Queue */
  if(verbose){cout << "oss: now removing message queue..." << endl;}
  if ( msgctl(msqid, IPC_RMID, NULL) == IPC_RESULT_ERROR ) {
    perror("perror: oss msgctl");
    return 1;
  }
  else {
    if(verbose){printf("oss: message queeue destroyed.\n");}
  }

  if(verbose){cout << "oss: termination successful." << endl;}
  return 0;
}


/* early termination (use with signal handling) */
void kms(int msqid) {
  //freeMemory, freeQueue used for whether or not to free shm and msq with ctl functions

  if (gSHM = 1) {
    if ( shmctl(getSharedMemory(THIS_FILE, BLOCK_SIZE), IPC_RMID, NULL) == IPC_RESULT_ERROR ) {
      perror("oss: shmctl");
    }
  }
  if (gMSQ = 1) {
    if ( msgctl(msqid, IPC_RMID, NULL) == IPC_RESULT_ERROR ) {
      perror("oss: msgctl");
    }
  }
  
  //TODO kill child processes using pids obtained via fork
  
  if(verbose){cout << "oss: terminating from signal" << endl;}
  exit(1);
}


/* ****************************************** *
 *              MAIN FUNCTION                 *
 * *******************************************/
int main(int argc, char *argv[]) {
  signal(SIGINT, handler);
  signal(SIGALRM, handler);

  /* cmdl args */
  int option;              //getopt option
  int pr_max = 5;          //max number of child processes spawned (def: 5) 
  string logfile = "log";  //filename for log file
  int ttk = 20;            //time to kill self and all children (def: 20)
  verbose = false;         //if true, lots of extra info will be printed to console (global)
  
  /* shared memory values */
  int shm_clock_s = 0;     //shared clock (seconds)
  int shm_clock_ns = 0;    //shared clock (nanoseconds)
  int shm_pid = 0;         //shared user process id for termination 

  if (verbose) {cout << "oss: hello world" << endl;}

  while ( (option = getopt(argc, argv, "hvc:l:t:")) != -1 ) {
  
    switch(option) {
      
      case 'h':
        printHelp();
        return(1);
        break;
      case 'c':
        pr_max = atoi( argv[(optind-1)] );
        if(verbose){cout << "max children specified: " << pr_max << endl;}
        if (pr_max > MAX_C) {
          printWarning(1);
          pr_max = MAX_C;
        }
        break;
      case 'l':
        logfile = argv[(optind-1)];
        if(verbose){cout << "logfile specified: " << logfile << endl;}
        break;
      case 't':
        ttk = atoi( argv[(optind-1)] ); 
        if(verbose){cout << "timer specified: " << ttk << endl;}
        if (ttk <= 0) {
          return(1);
        }
        alarm(ttk);
        break;
      case 'v':
        cout << "extra verbosity is on." << endl;
        verbose = true;
        break;
      case '?':
        //TODO
        return 1;
    }
  
  }
  
  if(gSTOP == 1) {exit(1);};
  
  
  /** OSS Process **/
  
  /* Shared Memory */
  int *shm_array; //shm pointer

  // Allocate and attach to shared memory
  shm_array = (int*)attachSharedMemory(THIS_FILE, BLOCK_SIZE);
  gSHM = 1; 
  if (verbose) {cout << "oss: shm created" << endl;}
  if (shm_array == NULL) {
      printf("Error: couldn't get memory block.\n");
    return 1;
  }
    
  //TODO define clock and get it into shared memory
  if (verbose) {cout << "oss: writing into shared memory: 4, 5, 6." << endl;}
  shm_array[0] = 4;
  shm_array[1] = 5;
  shm_array[2] = 6;

  if (verbose) {
    cout << "oss: read from shared memory: ";
    printf("%d ", shm_array[0] );
    printf("%d ", shm_array[1] );
    printf("%d\n", shm_array[2] );
  }

  if(gSTOP == 1) {kms(-1);}

  fexe(); // fork, exec


  /* Message Queues */
  int msqid; 
  key_t msqkey = 1984;  //TODO arbitrary value, use ftok
  mymsg_t mymsg;
  int msqsize;
  
  // Setup msq
  msqid = msgget(msqkey, PERM | IPC_CREAT);
  if (msqid == IPC_RESULT_ERROR) {
    perror("oss msgget");
  }
  else {
    gMSQ = 1;
    if (verbose) {cout << "oss: msgget successful" << endl;}
  }
  sleep(1);
  
  // Send TEST message
  strcpy( mymsg.mtext, "hello user" );  
  if ( msgsnd(msqid, &mymsg, strlen(mymsg.mtext) + 1, IPC_NOWAIT) == IPC_RESULT_ERROR ) {
    perror("oss msgsnd");
  }
  if (verbose) {printf("oss send: %s\n", mymsg.mtext);}
  sleep(1);
    
  // Receive TEST message 
  if ( msgrcv( msqid, &mymsg, 80, 0, MSG_NOERROR) == IPC_RESULT_ERROR ) {
    perror("oss msgrcv");
  }
  else {
    if (verbose) {cout << "oss received message: " << mymsg.mtext << endl;}
  }


  /* CRITICAL SECTION */
  if (verbose) {cout << "oss: ready for critical section" << endl;}
  bool hasFlag = true; //master starts with flag (user should default to false)
  strcpy(mymsg.mtext, "FLAG");
  long mtype_flag = 3;
  mymsg.mtype = mtype_flag;  //distinguish flag messages from earlier test messages
  
  for(int i=0; i<3; i++) {
    if(gSTOP == 1) {kms(msqid);}
    sleep(1);
    if (verbose) {cout << "oss: loop number: " << i << endl;}
    
    //parbegin
    while (hasFlag == false) {
      //cout << "oss: enter while (hasFlag == false)" << endl;
      if ( msgrcv(msqid, &mymsg, 80, mtype_flag, IPC_NOWAIT) == IPC_RESULT_ERROR ) {
        perror("oss msgrcv");
        sleep(1); //pause before trying msgq again
      }
      else {
        if (verbose) {cout << "oss: flag obtained." << mymsg.mtext << endl;}
        hasFlag = true;
      }
    }

    //CRITICAL
    if (hasFlag) {
      if (verbose) {cout << "oss: now in the critical section." << endl;}
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
  if (verbose) {cout << "oss: detatching shared memory." << endl;}
  if (shmdt((void *)shm_array) == IPC_RESULT_ERROR) {
    perror("perror: shmdt");
  }

  return endProgram(msqid);
}
