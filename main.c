#include "basics.h"
#include "daemonCreation.h"

int main(int argc, char *argv[]){

	daemonCreation(0);

	sleep(40);	

	exit(EXIT_SUCCESS);
}
