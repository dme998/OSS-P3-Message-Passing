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

#define MAX_C (100)     //max allowed value for option -c

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


/* print advisory messages for minor unexpectancies (non-error, non-fatal)*/
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
}


int main(int argc, char *argv[]) {
  cout << "oss is running." << endl;

  int option;              //getopt option
  int pr_max = 5;          //max number of child processes spawned (def: 5) 
  string logfile = "log";  //filename for log file
  int ttk = 20;            //time to kill self and all children (def: 20)

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
        return 1;
    }
  
  }
 
  return kms();
}
