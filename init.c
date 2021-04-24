// init: The initial user-level program

#include "user.h"

void
mymain(void)
{
	fork();
	fork();
	int i = 0;
	for(;;){
		if(i%5000000==0)
			print("process %d run from cpu %d!\n");
		i=(i+1)%5000000;
	}
//	fork();
}
