#include <stdio.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#define IPC_RESULT_ERROR (-1) 
#define PERM (0666) //permissions number for shmflg

/* *********************** */
/* Shared Memory Functions */
/* *********************** */

static int getSharedMemory(const char *filename, int size) {
  key_t key;

  // Request a key, linked to a filename
  key = ftok(filename, 0);
  if (key == IPC_RESULT_ERROR) {
    return (IPC_RESULT_ERROR);
  }

  // get shared block of memory  or create it if it doesn't exist
  return shmget(key, size, PERM | IPC_CREAT);
}



char* attachSharedMemory(const char* filename, int size) {
  int shared_block_id = getSharedMemory(filename, size);
  char *pointerToSharedMemory; //result of shmat operation

  if (shared_block_id == IPC_RESULT_ERROR) {
    return NULL;
  }
  
  /* map the shared block into this process's memory and return pointer */
  pointerToSharedMemory = (char *)shmat(shared_block_id, NULL, 0);
    //NULL lets the system decide where to do the mapping
    //3rd param is for additional flags, 0 is default
  if (pointerToSharedMemory == (char *) IPC_RESULT_ERROR) {
    return NULL;
  }  
  
  return pointerToSharedMemory; 
}



bool detachSharedMemory(char *block) {
  return (shmdt(block) != IPC_RESULT_ERROR);
}



bool removeSharedMemory(char *block) {
  int shared_block_id = getSharedMemory(block, 0);

  if (shared_block_id == IPC_RESULT_ERROR) {
    return NULL;
  }
  return ( shmctl(shared_block_id, IPC_RMID, NULL) != IPC_RESULT_ERROR);
    //IPC_RMID marks segment to be destroyed after last process detaches from it
    //NULL here basically means we don't care about getting further info on it
}
