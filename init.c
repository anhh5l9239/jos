// init: The initial user-level program

#include "user.h"

void
mymain(void)
{
	fork();
	fork();
	for(;;)
		print("process %d run from cpu %d!\n");
//	fork();
}
