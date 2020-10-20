/* CS4760 Project 3:Message Passing and Operating System Simulator
 * Author: Daniel Eggers
 * Instructor: Mark Hauschild
 * Date: October 20, 2020 
 */

#include <iostream>
#include <unistd.h>
#include<sys/msg.h>
#include <shmfunctions.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>

using namespace std;

typedef struct {
  long mtype;
  char mtext[80];
} mymsg_t;


int main() {
  cout << "user: hello world" << endl;
  //sleep(1);

  int *shm_array;

  shm_array = (int *)attachSharedMemory("oss", 12);
  if (shm_array == NULL) {
    printf("Error getting memblock\n");
    return 1;
  }
  
  //cout << "Child reading from shared memory..." << endl;
  //printf("...retrieved: \n%d\n", shm_array[0] );
  //printf("%d\n", shm_array[1]);
  //printf("%d\n", shm_array[2]);

  //cout << "Child modifying shared memory: 9 10 11..." << endl;
  shm_array[0] = 9;
  shm_array[1] = 10;
  shm_array[2] = 11;

  //cout << "user: reading from shared memory..." << endl;
  //printf("...retrieved: \n%d\n", shm_array[0] );
  //printf("%d\n", shm_array[1]);
  //printf("%d\n", shm_array[2]);
  


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
  strcpy(mymsg.mtext, "hello master\n");
  if ( msgsnd(msqid, &mymsg, strlen(mymsg.mtext)+1, IPC_NOWAIT) == IPC_RESULT_ERROR ) {
    perror("perror: user msgsnd");
  }
  else {
    cout << "user: send msg: " << mymsg.mtext << endl;
  }
  sleep(2);
  cout << "user: termination successful." << endl;
  return 0;
}
