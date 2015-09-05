#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK 8192
#define GCIHEAD 64
#define GCICOMMENT 0x520

#define be16(x) ((((x)>>8)&0xFF)|(((x)<<8))&0xFF00)
#define be32(x) (((x)<<24)|(((x)>>8)&0xFF00)|(((x)<<8)&0xFF0000)|((x)>>24))

#define CLIMAGIC "CL@MAG"

#define myfree(x) if(x!=NULL)free(x)

typedef unsigned short u16;

int main (int argc, char *argv[]) {
	FILE * pFile;
	long gcisize;
	long clisize;
	char * gcibuffer;
	char * clibuffer;
	size_t result;
	char *magic = CLIMAGIC;

	char outpath[256];
	char clipath[256];

    printf("\nCLI Embedder 0.1 by suloku 15'\n");
    printf("******************************\n\n");

    if(argc != 2){
        printf("This program will embedd a cli file into a gci file created with dol2gci.\n\n");
        printf("Usage: cliembed.exe mydol.gci\n");
        printf("       or drag and drop to executable file\n");
        printf("\nCli file must have the same name and be in the same path as gci file:\n\tswiss.gci and swiss.cli");
        printf("\n\nPress enter to continue...");
        getchar();
        exit(-1);
    }

    //Prepare
    strcpy(outpath, argv[1]);
    outpath[strlen(outpath)-4] = '\0';
    strcat(outpath, "-CLI.gci");

    strcpy(clipath, argv[1]);
    clipath[strlen(clipath)-3] = '\0';
    strcat(clipath, "cli");

    printf("Embedding %s into %s\n", clipath, argv[1]);
    printf("\nOutfile: %s\n\n", outpath);

    //READ DOL
	pFile = fopen ( argv[1] , "rb" );
	if (pFile==NULL) {printf ("Can't open %s", argv[1]); printf("\n\nPress enter to continue..."); getchar(); exit (1);}
	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	gcisize = ftell (pFile);
	rewind (pFile);
	// allocate memory to contain the whole file:
	gcibuffer = (char*) malloc (sizeof(char)*gcisize);
	if (gcibuffer == NULL) {printf ("Memory error"); printf("\n\nPress enter to continue..."); getchar(); exit (2);}
	// copy the file into the buffer:
	result = fread (gcibuffer,1,gcisize,pFile);
	if (result != gcisize) {printf ("Reading error"); printf("\n\nPress enter to continue..."); getchar(); exit (3);}
	/* the whole file is now loaded in the memory buffer. */
	// terminate
	fclose (pFile);

	//READ CLI
    pFile = fopen ( clipath , "rb" );
	if (pFile==NULL) {printf ("Can't open %s", clipath); printf("\n\nPress enter to continue..."); getchar(); exit (1);}

	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	clisize = ftell (pFile);
	rewind (pFile);

	// allocate memory to contain the whole file:
	clibuffer = (char*) malloc (sizeof(char)*clisize);
	if (clibuffer == NULL) {printf ("Memory error"); printf("\n\nPress enter to continue..."); getchar(); exit (2);}

	// copy the file into the buffer:
	result = fread (clibuffer,1,clisize,pFile);
	if (result != clisize) {printf ("Reading error"); printf("\n\nPress enter to continue..."); getchar(); exit (3);}

	/* the whole file is now loaded in the memory buffer. */

	// terminate
	fclose (pFile);

	printf ("GCI size: %d bytes (%d blocks)\n", (gcisize-GCIHEAD), !((gcisize-GCIHEAD)%BLOCK)?((gcisize-GCIHEAD)/BLOCK):((gcisize-GCIHEAD)/BLOCK)+1);
	printf ("Cli size: %d bytes\n", clisize);

    long offset = 0;
    int i = 0;
    int found = 0;
    char padmagic = 0xCD;
        while(1){
            if (memcmp(&gcibuffer[offset], &padmagic, 1) == 0 ){
                //printf("Found pads at %d\n", offset);
                for (i=0; i<16*2;i++){
                    if (memcmp(&gcibuffer[offset+i], &padmagic, 1) != 0){
                        break;
                    }
                }
                if (i==((16*2))) found = 1;
            }
            if (found) break;
            if (offset >= gcisize) break;
            offset++;
        }
    if(!found){
        printf("\nCan't find space (at least 32 0xCD bytes) to embed cli file!");
        printf("\nDo you want to add 1 block to your gci file? (y/n)");
        char c;

        while (1){
            c=getchar();
            getchar();
            if (c=='y' || c=='Y'){
                break;
            }else if (c=='n' || c=='N'){
                printf("\n\nThis gci file doesn't have enough space to embed the cli file.\n\nPress enter to continue...");
                getchar(); exit (3);
            }
        }
        //User wants to fatten gci file to embed the cli file
        strcpy(outpath, argv[1]);
        outpath[strlen(outpath)-4] = '\0';
        strcat(outpath, "-fat.gci");
        printf("\n\nCreating padded gci file at %s\n", outpath);

        //Create a pad block
        char* padding = (char*) malloc (sizeof(char)*BLOCK);
        memset (padding, 0xCD, BLOCK);
        //Update gci header to add one block
        u16 blocksize;
        memcpy(&blocksize,gcibuffer+0x38, 2);
        blocksize = be16(blocksize);
        blocksize = blocksize + 1;
        printf("New GCI blocksize: %d\n", blocksize);
        blocksize=be16(blocksize);
        memcpy(gcibuffer+0x38, &blocksize, 2);

        //write fatted gci file
        pFile = fopen (outpath, "wb");
        if (pFile==NULL) {printf ("Can't open %s", outpath); printf("\n\nPress enter to continue..."); getchar(); exit (1);}
        int ret;
        ret = fwrite (gcibuffer , sizeof(char), gcisize, pFile);
        //printf ("Written = %d, expected %d\n", ret, gcisize);
        ret = fwrite (padding , sizeof(char), BLOCK, pFile);
        //printf ("Written = %d, expected %d\n", ret, BLOCK);
        fclose (pFile);
        printf("\n\n%s created! Now use that file with this program to embed the cli file. Remember that the cli file must have the same name as the gci file.\n\n Press enter to continue...", outpath);
        getchar();
        myfree(padding);
        myfree (gcibuffer);
        myfree (clibuffer);
        return 0;
    } else{
        printf("GCI pad starts at: %d\n", offset);
        printf("Padding size: %d\n", gcisize-offset);
        int remaining = (gcisize-offset)-clisize-strlen(magic)*2;
        printf("Remaining pad after embed: %d\n", remaining);

        if( remaining <= 0){printf ("\nSorry, your CLI file can't fit in this gci file, it is %d bytes too big.", remaining*(-1)); printf("\n\nPress enter to continue..."); getchar(); exit (3);}

        //add cli file to dol buffer

        memcpy(gcibuffer+gcisize-clisize-strlen(magic)*2, magic, strlen(magic));
        memcpy(gcibuffer+gcisize-clisize-strlen(magic), clibuffer, clisize);
        memcpy(gcibuffer+gcisize-strlen(magic), magic, strlen(magic));

        printf("Writing cli to gci file...\n");

        pFile = fopen (outpath, "wb");
        if (pFile==NULL) {printf ("Can't open %s", outpath); printf("\n\nPress enter to continue..."); getchar(); exit (1);}
        fwrite (gcibuffer , sizeof(char), gcisize, pFile);
        fclose (pFile);
    }

  myfree (gcibuffer);
  myfree (clibuffer);

  printf("\nEmbedding complete! Press enter to continue...");
  getchar();
  return 0;
}
