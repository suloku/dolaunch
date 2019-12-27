#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <fat.h>
#include <sdcard/gcsd.h>
#include "aram/sidestep.h"

static int initFAT(){

		__io_gcsd2.startup();
		if (!__io_gcsd2.isInserted())
		{
			//printf ("No SD2SP2 inserted! Using embedded config.\n\n");
			return 0;
		}
		if (!fatMountSimple ("fat", &__io_gcsd2))
		{
			//printf("Error Mounting SD fat! Using embedded config.\n\n");
			return 0;
		}
		return 1;
}

int main(int argc, char *argv[])
{
	//does more than it should
	VIDEO_Init();

	int have_sd = initFAT();
	if (have_sd){
		FILE *fp = fopen("fat:/autoexec.dol", "rb");
		if (fp) {
			fseek(fp, 0, SEEK_END);
			int size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			if ((size > 0) && (size < (AR_GetSize() - (64*1024)))) {
				u8 *dol = (u8*) memalign(32, size);
				if (dol) {
					fread(dol, 1, size, fp);
						DOLtoARAM(dol, 0, NULL);
				}
				//We shouldn't reach this point
				if (dol != NULL) free(dol);
			}
			fclose(fp);
		}
			
	}
	return 0;
}
