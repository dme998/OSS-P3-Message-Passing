/* CS4760 Project 3:Message Passing and Operating System Simulator
 * Author: Daniel Eggers
 * Instructor: Mark Hauschild
 * Date: October 20, 2020 
 */

#include <iostream>
#include <unistd.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>    //shared memory
#include <sys/shm.h>    //shared memory
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>      // perror
#define IPC_NOCREAT (0)
#define PERM (0666)
#define IPC_RESULT_ERROR (-1)

using namespace std;

int BLOCK_SIZE = (sizeof(int) * 3);  //shared memory block size
int PROJ_ID = 2;  //arbitrary value for ftok (match this with oss ftok call)

typedef struct {
  long mtype;
  char mtext[80];
} mymsg_t;

/*
int kms(int *shm_array) {
  //free memory and terminate

  // Detach from Shared Memory 
  cout << "user: now detatching from shared memory." << endl;
  if (shmdt((void *)shm_array) == IPC_RESULT_ERROR) {
    perror("perror: shmdt");
  }
  
  return 0;
}*/


int main() {
  
  cout << "user: hello world" << endl;
  sleep(1);

  
  /* SHARED MEMORY */
  
  int shared_block_id;  //shmid
  key_t shmkey;         //obtained via ftok()
  int *shm_array;       //pointer to shm block

  //obtain key
  shmkey = ftok("oss", 2);
  if (shmkey == IPC_RESULT_ERROR) {
    perror("user: ftok");
  }
  
  //check for existing shared memory (does not create)
  shared_block_id = shmget(shmkey, BLOCK_SIZE, 0);
  if ( shared_block_id == IPC_RESULT_ERROR ) {
    //printf("%s", strerror(errno));
    perror("user: shmget");
    exit(1);
  }

  //attach to existing shared memory 
  shm_array = (int *)shmat(shared_block_id, NULL, 0);
  if ( shm_array == (int *)IPC_RESULT_ERROR ) {
    //printf("%s", strerror(errno));
    perror("user: shmat");
    exit(1);
  }

  
  cout << "user: reading from shared memory..." << endl;
  printf("...retrieved: %d", shm_array[0] );
  printf("%d ", shm_array[1]);
  printf("%d ", shm_array[2]);

  cout << "user: modifying shared memory: 9 10 11..." << endl;
  shm_array[0] = 9;
  shm_array[1] = 10;
  shm_array[2] = 11;

  cout << "user: reading from shared memory..." << endl;
  printf("...retrieved: %d ", shm_array[0] );
  printf("%d ", shm_array[1]);
  printf("%d ", shm_array[2]);
  


  /* MESSAGE QUEUES */ 
  
  //variables
  int msqid;
  key_t msqkey = 1984;  //TODO arbitrary value
  mymsg_t mymsg;
  int msqsize;

  //receive message
  msqid = msgget(msqkey, PERM | IPC_CREAT);
  if (msqid == IPC_RESULT_ERROR) {
    perror("perror: msgget");
  }
  else {
    cout << "user: msgget successful" << endl;
  }
  
  sleep(1);
  if ( msgrcv( msqid, &mymsg, 80, 0, IPC_NOWAIT) == IPC_RESULT_ERROR ) {
      perror("perror: user msgrcv");
  }
  else {
    cout << "user: received message: " << mymsg.mtext << endl;
  }


  //send message back (assuming oss hasn't terminated)
  strcpy(mymsg.mtext, "hello master");
  if ( msgsnd(msqid, &mymsg, strlen(mymsg.mtext)+1, IPC_NOWAIT) == IPC_RESULT_ERROR ) {
    perror("perror: user msgsnd");
  }
  else {
    cout << "user: send msg: " << mymsg.mtext << endl;
  }



  cout << "user: awake for critical section" << endl;
  /* CRITICAL SECTION */
  bool hasFlag = false; //master starts with flag (user should default to false)
  long mtype_flag = 3;
  mymsg.mtype = mtype_flag;  //distinguish flag messages from earlier tests
  int debugLoopBreak = 0;
  //strcpy(mymsg.mtext, "FLAG");
  
  for(int i=0; i<3; i++) {
    sleep(1);
    cout << "user: loop number: " << i << endl;
    //parbegin
    while (hasFlag == false) {
      //cout << "user: while hasFlag is FALSE" << endl;
      if ( msgrcv(msqid, &mymsg, 80, mtype_flag, IPC_NOWAIT) == IPC_RESULT_ERROR ) {
        perror("perror: oss msgrcv");
        sleep(1); //pause before trying msgq again
        debugLoopBreak++;
        if (debugLoopBreak >= 5) {exit(1); exit(1);}
      }
      else {
        cout << "user: flag obtained." << mymsg.mtext << endl;
        hasFlag = true;
      }
    }

    //CRITICAL
    if (hasFlag) {
      cout << "user: shared memory read: " << shm_array[2] << endl;
      shm_array[2] = 99;
      cout << "user: shared memory write: " << shm_array[2] << endl;
      
      //parend
      if ( msgsnd(msqid, &mymsg, strlen(mymsg.mtext) + 1, IPC_NOWAIT) == IPC_RESULT_ERROR ) {
        perror("perror: oss msgsnd");
        exit(1); //TODO kms
      }
      else {
        cout << "user: flag given up: " << mymsg.mtext << endl;
        hasFlag = false;
      }
    }
  } // i-loop



  cout << "user: termination successful." << endl;
  return 0;
}
