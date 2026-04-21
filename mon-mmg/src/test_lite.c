

#include <stdio.h>


#include "egads.h"


int main(int argc, char *argv[]) 

{

	int i, j;

	const char  *OCCrev;


	printf("Hello\n");


	// look at EGADS revision

    EG_revision(&i, &j, &OCCrev);

    printf("\n Using EGADS %2d.%02d with %s\n\n", i, j, OCCrev);


	return 0;

}

//gcc -I$HOME/ESP/EngSketchPad/include test_lite.c -L$HOME/ESP/EngSketchPad/lib -legadslite -lm -lpthread -o test_lite

