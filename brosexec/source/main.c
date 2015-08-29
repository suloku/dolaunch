#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <fat.h>
#include <sdcard/gcsd.h>
#include "aram/sidestep.h"

static int initFAT(){

	int slotagecko = 0;
	int i =0;
	s32 ret, memsize, sectsize;


	for(i=0;i<10;i++){
		ret = CARD_ProbeEx(CARD_SLOTA, &memsize, &sectsize);
		//printf("Ret: %d", ret);
		if (ret == CARD_ERROR_WRONGDEVICE){
			slotagecko = 1;
			break;
		}
	}

	if (slotagecko)
	{//Memcard in SLOT B, SD gecko in SLOT A
		//This will ensure SD gecko is recognized if inserted or changed to another slot after GCMM is executed
		for(i=0;i<10;i++){
			ret = CARD_Probe(CARD_SLOTA);
			if (ret == CARD_ERROR_WRONGDEVICE)
				//printf ("SDGecko detected...\n\n");
				break;
		}
		__io_gcsda.startup();
		if (!__io_gcsda.isInserted())
		{
			//printf ("No SD Gecko inserted! Using embedded config.\n\n");
			return 0;
		}
		if (!fatMountSimple ("fat", &__io_gcsda))
		{
			//printf("Error Mounting SD fat! Using embedded config.\n\n");
			return 0;
		}
	}else //Memcard in SLOT A, SD gecko in SLOT B
	{
		//This will ensure SD gecko is recognized if inserted or changed to another slot after GCMM is executed
		for(i=0;i<10;i++){
			ret = CARD_Probe(CARD_SLOTB);
			if (ret == CARD_ERROR_WRONGDEVICE)
				break;
		}	
		__io_gcsdb.startup();
		if (!__io_gcsdb.isInserted())
		{
			//printf ("No SD Gecko inserted! Using default config.\n\n");
			return 0;
		}
		if (!fatMountSimple ("fat", &__io_gcsdb))
		{
			//printf("Error Mounting SD fat! Using default config.\n\n");
			return 0;
		}
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
