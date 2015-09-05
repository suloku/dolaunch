#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <time.h>
#include "aram/sidestep.h"

#define CLIMAGIC "CL@MAG"

/*** Memory File Buffer ***/
//#define MAXFILEBUFFER (1024 * 2048)     /*** 2MB Buffer ***/
//u8 dolbuffer[MAXFILEBUFFER] ATTRIBUTE_ALIGN (32);

card_dir CardList[CARD_MAXFILES];       /*** Directory listing ***/
int cardcount = 0;
int listpos = 0;

static void *xfb = NULL;

static u8 SysArea[CARD_WORKAREA] ATTRIBUTE_ALIGN(32);
s32 memsize, sectsize;
char company[4];
char gamecode[6];

u32 first_frame = 1;
GXRModeObj *rmode;
void (*PSOreload)() = (void(*)())0x80001800;

void waitA(){
	printf("Press A to continue\n");
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

u8* cli_buffer;
int cli_size;

void getclifrombuffer(u8* gcibuffer, int gcisize){
    //test code for retrieving CLI from gci buffer
    char *magic = CLIMAGIC;
    int offset = 0;
    if (memcmp (gcibuffer-strlen(magic), magic, strlen(magic))){
        printf ("Magic found\n");
        for(offset = 0; offset < gcisize; offset++){
            if (memcmp(&gcibuffer[offset], magic, strlen(magic)) == 0 ){
                printf("Found first magic at %d\n", offset);
                cli_size = gcisize-offset-strlen(magic)*2;
                printf("Cli size = %d\n", cli_size);
                break;
            }
        }

        if (!(offset >= gcisize)){
            cli_buffer = (char *) malloc (sizeof(char)*cli_size+1); //+1 to be able to append terminating null character later
            memcpy (cli_buffer, gcibuffer+offset+strlen(magic), cli_size );
        }else{
            printf("No embeded cli found\n");
            cli_size = 0;
            cli_buffer = NULL;
        }
    }
}

/*---------------------------------------------------------------------------------
        This function is called if a card is physically removed
---------------------------------------------------------------------------------*/
void card_removed(s32 chn,s32 result)
{
//	printf("Card was removed from slot %s", (chn==CARD_SLOTA?"A":"B"));
	CARD_Unmount(chn);
}

/****************************************************************************
 * MountCard
 *
 * Mounts the memory card in the given slot.
 * CARD_Mount is called for a maximum of 10 tries
 * Returns the result of the last attempted CARD_Mount command.
 ***************************************************************************/
int MountCard(int cslot)
{
        s32 ret = -1;
        int tries = 0;
        int isMounted;
		memset (SysArea, 0, CARD_WORKAREA);
        // Mount the card, try several times as they are tricky
        while ( (tries < 10) && (ret < 0))
        {
                /*** We might have already initialized the Memory Card subsystem with CARD_Init()but, let's reset the
                EXI subsystem, just to make sure we have no problems mounting the card ***/
                EXI_ProbeReset();
                CARD_Init (NULL, NULL);
                //Ensure we start in show all files mode
                CARD_SetCompany(NULL);
                CARD_SetGamecode(NULL);

                /*** Mount the card ***/
                ret = CARD_Mount (cslot, SysArea, card_removed);
                if (ret >= 0) break;

                VIDEO_WaitVSync ();
                tries++;
        }
		/*** Make sure the card is really mounted ***/
        isMounted = CARD_ProbeEx(cslot, &memsize, &sectsize);
        if (memsize > 0 && sectsize > 0)//then we really mounted de card
        {
                return isMounted;
        }
        /*** If this point is reached, something went wrong ***/
        CARD_Unmount(cslot);
        return ret;
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	VIDEO_Init();
	
	rmode = VIDEO_GetPreferredMode(NULL);

	PAD_Init();
	
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
		
	VIDEO_Configure(rmode);
		
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	console_init(xfb,20,64,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*2);
	VIDEO_SetNextFramebuffer(xfb);
	
	int slot = CARD_SLOTA;
	u32 i;
	int tomenu = 0;
	u16 buttonsDown;
	u8 pressed = 0;
	int filesize;
	int bytesdone = 0;
	
	while(1){
		VIDEO_ClearFrameBuffer (rmode, xfb, COLOR_BLACK);
		listpos=0;
		tomenu=0;
		pressed = 0;
		printf("\x1b[2;0H");
		printf("Memory Card Loader 0.2 by Suloku\n");
		printf("********************************\n\n");
		printf("Press A or B button to select a slot.\n");
		while (1){
			PAD_ScanPads();
			buttonsDown=PAD_ButtonsDown(0);
			if (buttonsDown & PAD_BUTTON_A){ slot = CARD_SLOTA; pressed = 1;}
			if (buttonsDown & PAD_BUTTON_B){ slot = CARD_SLOTB; pressed = 1;}
			if (buttonsDown & PAD_BUTTON_START){ PSOreload();}
			VIDEO_WaitVSync();
			if (pressed) break;
		}
		printf("Mounting card in slot %s...", (slot==0?"A":"B"));
		
		int Slot_error = MountCard(slot);
		printf("code %d. ",Slot_error);

		int CardError;
		
		if (Slot_error >= 0) {
			printf("Sector size is %d bytes.\n",sectsize);
			printf("Reading memory card files...");

			card_dir CardDir;
			card_file CardFile;
			
			CardError = CARD_FindFirst(slot, &CardDir, true);

			cardcount = 0;
			while ( CARD_ERROR_NOFILE != CardError ) {
				if (strncmp ("DOLX", (char *)CardDir.gamecode, 4)  ==  0){
					memcpy (&CardList[cardcount], &CardDir, sizeof (card_dir));
					cardcount++;
				}
				CardError = CARD_FindNext(&CardDir);
			}
			printf("finished.\n\n");
			
			if (cardcount == 0){
				printf("No dol files in memory card in slot %s\n\n", (slot==0?"A":"B"));
				waitA();
			}else{
				while (1){
					pressed = 0;
					VIDEO_ClearFrameBuffer (rmode, xfb, COLOR_BLACK);
					printf("\x1b[2;0H");
					printf("Memory Card Loader 0.1 by Suloku\n");
					printf("********************************\n\n");
					printf("Dol files in memory card %s: %d\n\n", (slot==0?"A":"B"), cardcount);
					
					if (listpos!=0) printf ("/\\/\\\n");
					else printf("    \n");
					
					for (i=listpos; i<listpos+10; i++){
						printf ("    ");
						if (i==listpos) printf ("-->");
						printf("%s                    \n",CardList[i].filename);
						if (i>= cardcount )break;
					}
					printf("\x1b[18;0H");
					if (cardcount >=10 && listpos != cardcount-1) printf ("\\/\\/\n");
					else{
						printf("    \n");
					}
					printf("\n\t\tPress B button to go back to slot select.\n\n");

					while (1){
						PAD_ScanPads();
						buttonsDown=PAD_ButtonsDown(0);
						if (buttonsDown & PAD_BUTTON_UP){
							listpos--;
							if (listpos <0) listpos = 0;
							pressed = 1;
						}
						if (buttonsDown & PAD_BUTTON_DOWN){
							listpos++;
							if (listpos >= cardcount-1) listpos = cardcount-1;
							pressed = 1;
						}
						if (buttonsDown & PAD_BUTTON_LEFT){
							listpos-=10;
							if (listpos <0) listpos = 0;
							pressed = 1;
						}
						if (buttonsDown & PAD_BUTTON_RIGHT){
							listpos+=10;
							if (listpos >= cardcount-1) listpos = cardcount-1;
							pressed = 1;
						}
						if (buttonsDown & PAD_BUTTON_A){
							printf("Loading %s...\n", CardList[listpos].filename);
							/*** Initialise for this company & gamecode ***/
							//add null char
							company[2] = gamecode[4] = 0;
							memcpy (company, &CardList[listpos].company, 2);
							memcpy (gamecode, &CardList[listpos].gamecode, 4);							
							CARD_SetCompany(company);
							CARD_SetGamecode(gamecode);
							CardError = CARD_Open(slot ,(char*)&CardList[listpos].filename,&CardFile);
							if (CardError < 0)
							{
									CARD_Unmount (slot);
									printf("Error %d while opening file.\n", CardError);
									waitA();
									
							}else{
								/*** Copy the file contents to the buffer ***/
								filesize = CardFile.len;
								u8 *dolbuffer = (u8*) memalign(32, filesize);
								while (bytesdone < filesize)
								{
										CardError= CARD_Read(&CardFile, dolbuffer+bytesdone, sectsize, bytesdone);
										bytesdone += sectsize;
								}
								CARD_Close(&CardFile);
								CARD_Unmount(slot);
							//boot dol
								//This will load cli_buffer and cli_size
								getclifrombuffer(dolbuffer, filesize);
								if (cli_buffer!=NULL){
									// Build a command line to pass to the DOL
									int argc2 = 0;
									char *argv2[1024];
									//add a terminating null character for last argument if needed
									if (cli_buffer[cli_size-1] != '\0'){
										cli_buffer[cli_size] = '\0';
										cli_size += 1;
									}

									// CLI parse
									char bootpath[CARD_FILENAMELEN+10];
									sprintf(bootpath, "mc%d:/%s", slot, (char*)&CardList[listpos].filename);
									argv2[argc2] = bootpath;
									argc2++;
									// First argument is at the beginning of the file
									if(cli_buffer[0] != '\r' && cli_buffer[0] != '\n') {
										argv2[argc2] = cli_buffer;
										argc2++;
									}
									// Search for the others after each newline
									int i;
									for(i = 0; i < cli_size; i++) {
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
									DOLtoARAM(dolbuffer, argc2, argc2 == 0 ? NULL : argv2);
								}else{
									DOLtoARAM(dolbuffer, 0, NULL);
								}

								//If we get here dol was invalid
								if(dolbuffer != NULL) free(dolbuffer);
								printf("Not a valid dol file.\n");
								waitA();
								tomenu = 1;
							}
							pressed = 1;
						}
						if (buttonsDown & PAD_BUTTON_B){ pressed = 1;tomenu=1;}
						if (buttonsDown & PAD_BUTTON_START){ PSOreload();}
						VIDEO_WaitVSync();
						if (pressed) break;
					}
					if (tomenu) break;
				}
			}
			CARD_Unmount(slot);
		}else{
			printf("\n\nCan't mount card in slot %s!\n", (slot==0?"A":"B"));
			waitA();
		}
	}
	return 0;
}
