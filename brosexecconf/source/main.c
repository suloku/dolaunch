#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <fat.h>
#include <sdcard/gcsd.h>
#include "aram/sidestep.h"

//#define TESTDOLPHIN //Disables SD gecko check

#ifdef USEBACKGROUND
	#include "libpng/pngu/pngu.h"
	#ifdef TESTDOLPHIN
		#include "bckgrnd.h"
	#endif
#endif

#define CONFIGFILE "fat:/autoboot/autoconf.txt"
#define MAX_CONFIG_LINE 256

#define myprintf(...) if(!printflag) printf(__VA_ARGS__);


static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void (*PSOreload)() = (void(*)())0x80001800;
void Initialise() {

	VIDEO_Init();
	PAD_Init();
 
	rmode = VIDEO_GetPreferredMode(NULL);

	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
 
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

}

static int initFAT(){

	int slotagecko = 0;
	int slotbgecko = 0;
	int i =0;
	s32 ret, memsize, sectsize;

	for(i=0;i<10;i++){
		ret = CARD_ProbeEx(CARD_SLOTA, &memsize, &sectsize);
		//myprintf("Ret: %d", ret);
		if (ret == CARD_ERROR_WRONGDEVICE)
		{
			slotagecko = 1;
			break;
		}
	}

	if(slotagecko == 0) //Sd gecko not in Slot A, check if it's in Slot B
	{
		for(i=0;i<10;i++){
			ret = CARD_ProbeEx(CARD_SLOTB, &memsize, &sectsize);
			//printf("Ret: %d", ret);
			if (ret == CARD_ERROR_WRONGDEVICE)
			{
				slotbgecko = 1;
				break;
			}
		}
	}

	if (slotagecko)
	{//Memcard in SLOT B, SD gecko in SLOT A
		//This will ensure SD gecko is recognized if inserted or changed to another slot after GCMM is executed
		for(i=0;i<10;i++){
			ret = CARD_Probe(CARD_SLOTA);
			if (ret == CARD_ERROR_WRONGDEVICE)
				//myprintf ("SDGecko detected...\n\n");
				break;
		}
		__io_gcsda.startup();
		if (!__io_gcsda.isInserted())
		{
			//myprintf ("No SD Gecko inserted! Using embedded config.\n\n");
			return 0;
		}
		if (!fatMountSimple ("fat", &__io_gcsda))
		{
			//myprintf("Error Mounting SD fat! Using embedded config.\n\n");
			return 0;
		}
	}else if (slotbgecko) //Memcard in SLOT A, SD gecko in SLOT B
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
			//myprintf ("No SD Gecko inserted! Using default config.\n\n");
			return 0;
		}
		if (!fatMountSimple ("fat", &__io_gcsdb))
		{
			//myprintf("Error Mounting SD fat! Using default config.\n\n");
			return 0;
		}
	}else
	{
		//Memcard in slot A or B, SD2SP2 in Serial Port 2 (since we couldn't detect an sdgecko in either slot)
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
	}
	
	return 1;
}

//Config

	int timer = -1;
	char image[256];
	int printflag = 0;
	char def[256] = "fat:/autoexec.dol";
	char def_title[256];
	char buttonA[256];
	char buttonA_title[256];
	char buttonB[256];
	char buttonB_title[256];
	char buttonY[256];
	char buttonY_title[256];
	char buttonX[256];
	char buttonX_title[256];
	char buttonL[256];
	char buttonL_title[256];
	char buttonR[256];
	char buttonR_title[256];
	char buttonD[256];
	char buttonD_title[256];
	char buttonU[256];
	char buttonU_title[256];
	char buttonZT[256];
	char buttonZT_title[256];
	char buttonLT[256];
	char buttonLT_title[256];
	char buttonRT[256];
	char buttonRT_title[256];
	char buttonS[256];
	char buttonS_title[256];

int readparseconf(char * config) {
   FILE * pFile;
   char mystring [MAX_CONFIG_LINE];

   pFile = fopen (config , "r");
   if (pFile == NULL){ myprintf ("Error opening %s\n", config); return -1;}
   else {
     while(1){
		if (fgets (mystring , MAX_CONFIG_LINE-1 , pFile) == NULL ) break;
		if (mystring[0]=='#') continue;
		if (mystring[0]=='\n') continue;
		if (mystring[0]=='\r') continue;

		if (strncmp("IMAGE=", mystring, 6)==0){
            strcpy(image, mystring+6);
            image[strcspn(image, "\r\n")] = 0;
		}

		if (strncmp("NOPRINT=", mystring, 8)==0){
            printflag = atoi(mystring+8);
		}
		
		if (strncmp("TIMER=", mystring, 6)==0){
            timer = atoi(mystring+6);
		}

		if (strncmp("DEFAULT=", mystring, 8)==0){
            strcpy(def, mystring+8);
            def[strcspn(def, "\r\n")] = 0;
		}
		if (strncmp("DEF_TITLE=", mystring, 10)==0){
            strcpy(def_title, mystring+10);
            def_title[strcspn(def_title, "\r\n")] = 0;
		}

		if (strncmp("A=", mystring, 2)==0){
            strcpy(buttonA, mystring+2);
            buttonA[strcspn(buttonA, "\r\n")] = 0;
		}
		if (strncmp("A_TITLE=", mystring, 8)==0){
            strcpy(buttonA_title, mystring+8);
            buttonA_title[strcspn(buttonA_title, "\r\n")] = 0;
		}
		if (strncmp("B=", mystring, 2)==0){
            strcpy(buttonB, mystring+2);
            buttonB[strcspn(buttonB, "\r\n")] = 0;
		}
		if (strncmp("B_TITLE=", mystring, 8)==0){
            strcpy(buttonB_title, mystring+8);
            buttonB_title[strcspn(buttonB_title, "\r\n")] = 0;
		}
		
		if (strncmp("Y=", mystring, 2)==0){
            strcpy(buttonY, mystring+2);
            buttonY[strcspn(buttonY, "\r\n")] = 0;
		}
		if (strncmp("Y_TITLE=", mystring, 8)==0){
            strcpy(buttonY_title, mystring+8);
            buttonY_title[strcspn(buttonY_title, "\r\n")] = 0;
		}

		if (strncmp("X=", mystring, 2)==0){
            strcpy(buttonX, mystring+2);
            buttonX[strcspn(buttonX, "\r\n")] = 0;
		}
		if (strncmp("X_TITLE=", mystring, 8)==0){
            strcpy(buttonX_title, mystring+8);
            buttonX_title[strcspn(buttonX_title, "\r\n")] = 0;
		}

		if (strncmp("L=", mystring, 2)==0){
            strcpy(buttonL, mystring+2);
            buttonL[strcspn(buttonL, "\r\n")] = 0;
		}
		if (strncmp("L_TITLE=", mystring, 8)==0){
            strcpy(buttonL_title, mystring+8);
            buttonL_title[strcspn(buttonL_title, "\r\n")] = 0;
		}

		if (strncmp("R=", mystring, 2)==0){
            strcpy(buttonR, mystring+2);
            buttonR[strcspn(buttonR, "\r\n")] = 0;
		}
		if (strncmp("R_TITLE=", mystring, 8)==0){
            strcpy(buttonR_title, mystring+8);
            buttonR_title[strcspn(buttonR_title, "\r\n")] = 0;
		}

		if (strncmp("D=", mystring, 2)==0){
            strcpy(buttonD, mystring+2);
            buttonD[strcspn(buttonD, "\r\n")] = 0;
		}
		if (strncmp("D_TITLE=", mystring, 8)==0){
            strcpy(buttonD_title, mystring+8);
            buttonD_title[strcspn(buttonD_title, "\r\n")] = 0;
		}

		if (strncmp("U=", mystring, 2)==0){
            strcpy(buttonU, mystring+2);
            buttonU[strcspn(buttonU, "\r\n")] = 0;
		}
		if (strncmp("U_TITLE=", mystring, 8)==0){
            strcpy(buttonU_title, mystring+8);
            buttonU_title[strcspn(buttonU_title, "\r\n")] = 0;
		}

		if (strncmp("ZT=", mystring, 3)==0){
            strcpy(buttonZT, mystring+3);
            buttonZT[strcspn(buttonZT, "\r\n")] = 0;
		}
		if (strncmp("ZT_TITLE=", mystring, 9)==0){
            strcpy(buttonZT_title, mystring+9);
            buttonZT_title[strcspn(buttonZT_title, "\r\n")] = 0;
		}

		if (strncmp("LT=", mystring, 3)==0){
            strcpy(buttonLT, mystring+3);
            buttonLT[strcspn(buttonLT, "\r\n")] = 0;
		}
		if (strncmp("LT_TITLE=", mystring, 9)==0){
            strcpy(buttonLT_title, mystring+9);
            buttonLT_title[strcspn(buttonLT_title, "\r\n")] = 0;
		}

		if (strncmp("RT=", mystring, 3)==0){
            strcpy(buttonRT, mystring+3);
            buttonRT[strcspn(buttonRT, "\r\n")] = 0;
		}
		if (strncmp("RT_TITLE=", mystring, 9)==0){
            strcpy(buttonRT_title, mystring+9);
            buttonRT_title[strcspn(buttonRT_title, "\r\n")] = 0;
		}

		if (strncmp("S=", mystring, 2)==0){
            strcpy(buttonS, mystring+2);
            buttonS[strcspn(buttonS, "\r\n")] = 0;
		}
		if (strncmp("S_TITLE=", mystring, 8)==0){
            strcpy(buttonS_title, mystring+8);
            buttonS_title[strcspn(buttonS_title, "\r\n")] = 0;
		}
	}

     fclose (pFile);
   }
   return 0;
}

void waitA(){
	myprintf("Press A to continue\n");
	while (1){
		PAD_ScanPads();
		if (PAD_ButtonsDown(0) & PAD_BUTTON_A){
			while (PAD_ButtonsHeld(0) & PAD_BUTTON_A){
				PAD_ScanPads();
			}
			break;
		}
		VIDEO_WaitVSync();
	}
}


int main(int argc, char *argv[])
{
	Initialise();
	myprintf("\x1b[2;5H");
	myprintf("Configurable sd dol launcher 0.1 by suloku  '15\n");

	int have_sd = 0;
	while(1){
		have_sd=initFAT();
		if (!have_sd){
			while(1){
				printf("\x1b[4;5H");
				printf("Couldn't mount SD Gecko. Insert one now and press A.\n");
				PAD_ScanPads();
				if( PAD_ButtonsDown(0) & PAD_BUTTON_A ) {
					break;
				}
				
				VIDEO_WaitVSync();
			}
		}
#ifndef TESTDOLPHIN
		if (have_sd){
#else
		if (1){
#endif
			break;
		}
	}
	if(readparseconf(CONFIGFILE)<0)waitA();

	time_t now, boottime;
	time(&boottime);
		
	char bootpath[256];
	char clipath[256];
	strcpy(bootpath, def);

loopback:

VIDEO_ClearFrameBuffer (rmode, xfb, COLOR_BLACK);
#ifdef USEBACKGROUND
	PNGUPROP imgProp;
	IMGCTX ctx;

	#ifndef TESTDOLPHIN
		
		if ( !(ctx = PNGU_SelectImageFromDevice (image)) ){
			if (strlen(image)>0) myprintf ("PNGU_SelectFileFromDevice failed!\n");
		}
	#else
		if ( !(ctx = PNGU_SelectImageFromBuffer (bckgrnd)) ){
			myprintf ("PNGU_SelectFileFromBuffer failed!\n");
		}
	#endif	
		else{
			if (PNGU_GetImageProperties (ctx, &imgProp) != PNGU_OK){
				myprintf ("PNGU_GetImageProperties failed!\n");
			}else{
				if (PNGU_DecodeToYCbYCr (ctx, imgProp.imgWidth, imgProp.imgHeight, xfb, 640 - imgProp.imgWidth) != PNGU_OK){
					myprintf ("PNGU_DecodeToYCbYCr failed!\n");
				}
			}
			PNGU_ReleaseImageContext (ctx);
		}

#endif

	int boot;
	FILE * fp;
	int size;

	while(1) {
		boot = 0;
		time(&now);
		myprintf("\x1b[5;6H");
		if (timer >=0){
			//myprintf("Timer    : %d\n", timer);
			myprintf(" Timer    : %02.0f\n", (double)timer - difftime(now, boottime));
		}else{
			myprintf(" Timer    : disabled\n");
		}
		myprintf("\x1b[6;6H Default  : %s\n\n\n", (strlen(def_title)>1)?def_title:def);
		myprintf("\x1b[9;6H Button A : %s\n", (strlen(buttonA_title)>1)?buttonA_title:buttonA);
		myprintf("\x1b[10;6H Button B : %s\n", (strlen(buttonB_title)>1)?buttonB_title:buttonB);
		myprintf("\x1b[11;6H Button Y : %s\n", (strlen(buttonY_title)>1)?buttonY_title:buttonY);
		myprintf("\x1b[12;6H Button X : %s\n", (strlen(buttonX_title)>1)?buttonX_title:buttonX);
		myprintf("\x1b[13;6H Button L : %s\n", (strlen(buttonL_title)>1)?buttonL_title:buttonL);
		myprintf("\x1b[14;6H Button R : %s\n", (strlen(buttonR_title)>1)?buttonR_title:buttonR);
		myprintf("\x1b[15;6H Button D : %s\n", (strlen(buttonD_title)>1)?buttonD_title:buttonD);
		myprintf("\x1b[16;6H Button U : %s\n", (strlen(buttonU_title)>1)?buttonU_title:buttonU);
		myprintf("\x1b[17;6H Button ZT: %s\n", (strlen(buttonZT_title)>1)?buttonZT_title:buttonZT);
		myprintf("\x1b[18;6H Button LT: %s\n", (strlen(buttonLT_title)>1)?buttonLT_title:buttonLT);
		myprintf("\x1b[19;6H Button RT: %s\n", (strlen(buttonRT_title)>1)?buttonRT_title:buttonRT);
		myprintf("\x1b[20;6H Button S : %s\n", (strlen(buttonS_title)>1)?buttonS_title:buttonS);

/*		
		if (timer >=0){
			myprintf("\x1b[4;50H");
			myprintf("**********\n");
			myprintf("\x1b[5;50H");
			myprintf("*   %02.0f   *\n", (double)timer - difftime(now, boottime));
			myprintf("\x1b[6;50H");
			myprintf("**********\n");
		}
*/
		PAD_ScanPads();

		int buttonsDown = PAD_ButtonsDown(0);
		
		if( buttonsDown & PAD_BUTTON_A ) {
			strcpy(bootpath, buttonA);
			boot=1;
		}else if( buttonsDown & PAD_BUTTON_B ) {
			strcpy(bootpath, buttonB);
			boot=1;
		}else if( buttonsDown & PAD_BUTTON_Y ) {
			strcpy(bootpath, buttonY);
			boot=1;
		}else if( buttonsDown & PAD_BUTTON_X ) {
			strcpy(bootpath, buttonX);
			boot=1;
		}else if( buttonsDown & PAD_BUTTON_LEFT ) {
			strcpy(bootpath, buttonL);
			boot=1;
		}else if( buttonsDown & PAD_BUTTON_RIGHT ) {
			strcpy(bootpath, buttonR);
			boot=1;
		}else if( buttonsDown & PAD_BUTTON_DOWN ) {
			strcpy(bootpath, buttonD);
			boot=1;
		}else if( buttonsDown & PAD_BUTTON_UP ) {
			strcpy(bootpath, buttonU);
			boot=1;
		}else if( buttonsDown & PAD_TRIGGER_Z ) {
			strcpy(bootpath, buttonZT);
			boot=1;
		}else if( buttonsDown & PAD_TRIGGER_L ) {
			strcpy(bootpath, buttonLT);
			boot=1;
		}else if( buttonsDown & PAD_TRIGGER_R ) {
			strcpy(bootpath, buttonRT);
			boot=1;
		}else if (buttonsDown & PAD_BUTTON_START) {
			strcpy(bootpath, buttonS);
			boot=1;
		}
		
		if (timer >=0){if ( difftime(now, boottime) >= timer) boot=1;}

		if (boot){
			fp = fopen ( bootpath, "rb" );
			if (fp==NULL){
				myprintf("\x1b[22;5H                                                                   ");
				myprintf("\x1b[22;6H");
				myprintf("Can't open %s, booting default dol.", bootpath);
				strcpy(bootpath, def);
				fp = fopen ( bootpath, "rb" );
				if (fp==NULL){
					if (strcmp(bootpath, "fat:/autoexec.dol") != 0){
						myprintf("\x1b[23;5H                                                                   ");
						myprintf("\x1b[23;6H");
						myprintf("Can't open %s, booting autoexec.dol.", bootpath);
						sprintf(bootpath, "fat:/autoexec.dol");
						fp = fopen ( bootpath, "rb" );
						if (fp==NULL) {
							myprintf("\x1b[24;5H                                                                   ");
							myprintf("\x1b[25;5H                                                                   ");
							myprintf("\x1b[24;6H");
							myprintf ("Failed to open autoexec.dol. It would be wise to have it at your\x1b[25;6Hsdcard root. Please, check your configuration file.\n\n");	
							myprintf("\x1b[26;5H                                                                   ");
							myprintf("\x1b[27;5H                                                                   ");
							myprintf("\x1b[27;6H");
						}
					}else{
							myprintf("\x1b[23;5H                                                                   ");
							myprintf("\x1b[24;5H                                                                   ");
							myprintf("\x1b[23;6H");
						myprintf ("Failed to open autoexec.dol. It would be wise to have it at your\x1b[24;6Hsdcard root. Please, check your configuration file.\n\n");	
						myprintf("\x1b[25;5H                                                                   ");
						myprintf("\x1b[26;5H                                                                   ");
						myprintf("\x1b[26;6H");
					}
				}
			}
			if (fp != NULL){
				fseek(fp, 0, SEEK_END);
				size = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				if ((size > 0) && (size < (AR_GetSize() - (64*1024)))) {
					u8 *dol = (u8*) memalign(32, size);
					if (dol) {
						fread(dol, 1, size, fp);
						//CLI support
							strcpy(clipath, bootpath);
							clipath[strlen(clipath)-3]='c';
							clipath[strlen(clipath)-2]='l';
							clipath[strlen(clipath)-1]='i';
							FILE * fp2;
							int size2;
							fp2 = fopen ( clipath, "rb" );
							if (fp2!=NULL){
								fseek(fp2, 0, SEEK_END);
								size2 = ftell(fp2);
								fseek(fp2, 0, SEEK_SET);
								// Build a command line to pass to the DOL
								int argc2 = 0;
								char *argv2[1024];
								char *cli_buffer = memalign(32, size2+1); // +1 to append null character if needed
								if(cli_buffer) {
									fread(cli_buffer, 1, size2, fp2);
									fclose(fp2);
									//add a terminating null character for last argument if needed
									if (cli_buffer[size2-1] != '\0'){
										cli_buffer[size2] = '\0';
										size2 += 1;
									}

									// CLI parse
									argv2[argc2] = bootpath;
									argc2++;
									// First argument is at the beginning of the file
									if(cli_buffer[0] != '\r' && cli_buffer[0] != '\n') {
										argv2[argc2] = cli_buffer;
										argc2++;
									}
									// Search for the others after each newline
									int i;
									for(i = 0; i < size2; i++) {
										if(cli_buffer[i] == '\r' || cli_buffer[i] == '\n') {
											cli_buffer[i] = '\0';
										}
										else if(cli_buffer[i - 1] == '\0') {
											argv2[argc2] = cli_buffer + i;
											argc2++;

											if(argc2 >= 1024)
												break;
										}
									}
								}
								DOLtoARAM(dol, argc2, argc2 == 0 ? NULL : argv2);
							}else{
								DOLtoARAM(dol, 0, NULL);
							}
					}
					//We shouldn't reach this point
					if (dol != NULL) free(dol);
					myprintf("Not a valid dol File! ");
				}
				fclose(fp);
			}
			waitA();
			goto loopback;
		}
		VIDEO_WaitVSync();
	}
	return 0;
}
