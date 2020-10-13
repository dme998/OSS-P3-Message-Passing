/* CS4760 Project 3:Message Passing and Operating System Simulator
 * Author: Daniel Eggers
 * Instructor: Mark Hauschild
 * Date: October 20, 2020 
 */

#include <iostream>
#include <shmfunctions.h>

using namespace std;

int main() {
  std::cout << "hello world" << std::endl;

  int *shm_array;

  shm_array = (int *)attachSharedMemory("oss", 12);
  if (shm_array == NULL) {
    printf("Error getting memblcok\n");
    return 1;
  }
  
  cout << "Child reading from shared memory..." << endl;
  printf("...retrieved: \n%d\n", shm_array[0] );
  printf("%d\n", shm_array[1]);
  printf("%d\n", shm_array[2]);

  cout << "Child modifying shared memory..." << endl;
  shm_array[0] = 9;
  shm_array[1] = 10;
  shm_array[2] = 11;

  cout << "Child reading from shared memory..." << endl;
  printf("...retrieved: \n%d\n", shm_array[0] );
  printf("%d\n", shm_array[1]);
  printf("%d\n", shm_array[2]);

  return 0;
}
