#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define TRUE  (1)
#define FALSE (0)
#define OK (0)
#define ERROR (-1)
#define UNINITIALIZED (-100)
#define MAX_BASELINES (28)

typedef struct __attribute__((packed)) codehDef {
  
  char v_name[12];/* label 			*/
  short  icode	; /* index for a code word	*/
  char code[26]	; /* the code word		*/
  short ncode	; /* # chars in code word	*/
  
} codehDef;
/* the size of codeh is 42 bytes */

typedef struct __attribute__((packed)) sphDef {
  int 	sphid     ; /* spectrum id #              */
  int 	blhid     ; /* proj. baseline id #        */
  int 	inhid     ; /* integration id #           */
  short	igq       ; /* gain qual int code         */
  short	ipq       ; /* passband qual int code     */
  short	iband     ; /* spectral band int code     */
  short	ipstate   ; /* pol state int code         */
  float	tau0      ; /* tau at zenith              */
  double vel      ; /* velocity (vctype) (km/s)   */
  float	vres      ; /* velocity res.              */
  double fsky     ; /* center sky freq.           */
  float	fres      ; /* frequency res. (MHz)       */
  double gunnLO   ; /* Frequency of first LO      */
  double cabinLO  ; /* Frequency of BDA LO        */
  double corrLO1  ; /* Freq. of first Corr. LO    */
  double corrLO2  ; /* Freq. of second Corr. LO   */
  float	integ     ; /* integration time           */
  float	wt        ; /* weight (sec/tssb**2)       */
  int   flags     ; /* per-baseline flag info     */  
  float	vradcat   ; /* catalog radial velocity    */
  short	nch       ; /* # channels in spectrum     */
  short	nrec      ; /* # of records w/i inh#      */
  int 	dataoff   ; /* byte offset for data       */
  double rfreq    ; /* rest frequency (GHz)       */
  short corrblock ; /* Correlator block number    */
                    /* 0 for c1 chunk             */ 
  short corrchunk ; /* Correlator chunk number    */
		    /* NOT the sxx chunk name -   */
		    /* this value is always 1->4, */
		    /* except for the c1 chunk    */
  int spareint1   ; /* Spare int for future use   */
  int spareint2   ; /* Spare int for future use   */
  int spareint3   ; /* Spare int for future use   */
  int spareint4   ; /* Spare int for future use   */
  int spareint5   ; /* Spare int for future use   */
  int spareint6   ; /* Spare int for future use   */
  double sparedbl1; /* Spare double for future    */
  double sparedbl2; /* Spare double for future    */
  double sparedbl3; /* Spare double for future    */
  double sparedbl4; /* Spare double for future    */
  double sparedbl5; /* Spare double for future    */
  double sparedbl6; /* Spare double for future    */
} sphDef;

typedef struct chunkSpec {
  int sourceChunk;
  int startChan;
  int endChan;
  int nAve;
  int iband;
  struct chunkSpec *next;
} chunkSpec;

chunkSpec *newChunkList = NULL;

int isLegalN(int n) {
  return ((n == 1) || (n == 2) || (n == 4) || (n == 8) || (n == 16) || (n == 32) ||
	  (n == 64) || (n == 128) || (n == 256) || (n == 512) || (n == 1024));
}

void printUsage(char *name) {
  printf("Usage:\n");
  printf("%s -i {input directory} -o {output directory} [-d] {chunk spec}. {chunk spec} ...\n", name);
  exit(0);
}

int main (int argc, char **argv)
{
  int i, inFId, nRead, outFId, nSynth, justRegrid, eChan, sChan;
  int newBandCounter = 0;
  int schInFId, schOutFId, lastInhid, schDataSize, codesInFId, codesOutFId;
  int outputDefault = FALSE;
  int gotInput = FALSE;
  int gotOutput = FALSE;
  int header[2];
  int nSynthSize = 0;
  int newDataSize = 0;
  int newHeader[2], newSize;
  int outPtr = 0;
  short *data = NULL;
  short *newData = NULL;
  char *inDir = NULL;
  char *outDir = NULL;
  char shellCommand[1000], fileName[1000];
  chunkSpec *newChunk = NULL;
  chunkSpec *lastChunk = NULL;
  codehDef oldCode;
  sphDef oldSp, newSp;

  while ((i = getopt(argc, argv, "di:o:")) != -1) {
    switch (i) {
    case 'd':
      outputDefault = TRUE;
      break;
    case 'i':
      gotInput = TRUE;
      inDir = optarg;
      break;
    case 'o':
      gotOutput = TRUE;
      outDir = optarg;
      break;
    default:
      printUsage(argv[0]);
    }
  }
  if (!gotInput || !gotOutput)
    printUsage(argv[0]);
  if (optind >= argc) {
    fprintf(stderr, "You must specify at least one chunk specification\n");
    printUsage(argv[0]);
  } else
    nSynth = argc-optind;
  /* printf("Will produce %d synthetic chunks\n", nSynth); */
  if (!outputDefault && (nSynth < 3))
    justRegrid = TRUE;
  else
    justRegrid = FALSE;
  if (outputDefault) {
    /* Define two "new" chunks which are just the raw SWARM chunks without any changes */
    for (i = 49; i <= 50; i++) {
      newChunk = malloc(sizeof(*newChunk));
      if (newChunk == NULL) {
	perror("newChunk malloc");
	exit(ERROR);
      }
      newChunk->sourceChunk = i;
      newChunk->startChan = 0;
      newChunk->endChan = 16383;
      newChunk->nAve = 1;
      newChunk->iband = 49+newBandCounter++;
      newChunk->next = NULL;
      if (newChunkList == NULL)
	newChunkList = newChunk;
      else
	lastChunk->next = newChunk;
      lastChunk = newChunk;    
    }
  }
  for (i = 0; i < nSynth; i++) {
    int chunk, n, nRead;

    nRead = sscanf(argv[i+optind], "%d:%d:%d:%d", &chunk, &sChan, &eChan, &n);
    if (nRead != 4) {
      fprintf(stderr, "Error in new chunk spcification #%d\n", i+1);
      fprintf(stderr, "Cannot parse chunk specification \"%s\"\n", argv[i+optind]);
      exit(ERROR);
    }
    if ((chunk < 49) || (chunk > 50)) {
      fprintf(stderr, "Error in new chunk spcification #%d\n", i+1);
      fprintf(stderr, "\"%s\" is an invalid chunk specifier - input chunk (%d) must be 49 or 50\n",
	      argv[i+optind], chunk);
      exit(ERROR);
    }
    if (!isLegalN(n)) {
      fprintf(stderr, "Error in new chunk spcification #%d\n", i+1);
      fprintf(stderr,
	      "Illegal number of channels to average (%d), must be a power of 2 between 1 and 1024 (inclusive)\n", n);
      exit(ERROR);
    }
    if (sChan > eChan) {
      fprintf(stderr, "Error in new chunk spcification #%d\n", i+1);
      fprintf(stderr,
	      "The starting channel number must be less than the ending channel number\n");
      exit(ERROR);
    }
    if ((sChan < 0) || (sChan > 16382)) {
      fprintf(stderr, "Error in new chunk spcification #%d\n", i+1);
      fprintf(stderr,
	      "The starting channel number must be between 0 and 16382\n");
      exit(ERROR);
    }
    if ((eChan < 1) || (sChan > 16383)) {
      fprintf(stderr, "Error in new chunk spcification #%d\n", i+1);
      fprintf(stderr,
	      "The ending channel number must be between 1 and 16383\n");
      exit(ERROR);
    }
    if ((eChan - sChan +1) % n) {
      fprintf(stderr, "Error in new chunk spcification #%d\n", i+1);
      fprintf(stderr,
	      "The total number of input channels (%d) is not evenly divisible by %d - aborting\n",
	      eChan-sChan+1, n);
      exit(ERROR);
    }
    newChunk = malloc(sizeof(*newChunk));
    if (newChunk == NULL) {
      perror("newChunk malloc");
      exit(ERROR);
    }
    newChunk->sourceChunk = chunk;
    newChunk->startChan = sChan;
    newChunk->endChan = eChan;
    newChunk->nAve = n;
    newChunk->iband = 49+newBandCounter++;
    newChunk->next = NULL;
    if (newChunkList == NULL)
      newChunkList = newChunk;
    else
      lastChunk->next = newChunk;
    lastChunk = newChunk;
  }
  if (outputDefault)
    nSynth += 2;

  /* Make the destination directory */
  if (mkdir(outDir, 0777)) {
    perror("making output diectory");
    exit(ERROR);
  }
  /* Copy the files which do not need modification */
  printf("Copying files which need no modification\n");
  printf("\tantennas\n");
  sprintf(shellCommand, "cp %s/antennas %s/", inDir, outDir);
  system(shellCommand);
  printf("\tautoCorrelations\n");
  sprintf(shellCommand, "cp %s/autoCorrelations %s/", inDir, outDir);
  /* system(shellCommand); */
  printf("\tbl_read\n");
  sprintf(shellCommand, "cp %s/bl_read %s/", inDir, outDir);
  system(shellCommand);
  if (justRegrid) {
    printf("\tcodes_read\n");
    sprintf(shellCommand, "cp %s/codes_read %s/", inDir, outDir);
    system(shellCommand);
  }
  printf("\tcodeVersions\n");
  sprintf(shellCommand, "cp %s/codeVersions %s/", inDir, outDir);
  system(shellCommand);
  printf("\teng_read\n");
  sprintf(shellCommand, "cp %s/eng_read %s/", inDir, outDir);
  system(shellCommand);
  printf("\tin_read\n");
  sprintf(shellCommand, "cp %s/in_read %s/", inDir, outDir);
  system(shellCommand);
  printf("\tmodeInfo\n");
  sprintf(shellCommand, "cp %s/modeInfo %s/", inDir, outDir);
  system(shellCommand);
  printf("\tplot_me files\n");
  sprintf(shellCommand, "cp %s/plot_me* %s/", inDir, outDir);
  system(shellCommand);
  printf("\tprojectInfo\n");
  sprintf(shellCommand, "cp %s/projectInfo* %s/", inDir, outDir);
  system(shellCommand);
  printf("\ttsys_read\n");
  sprintf(shellCommand, "cp %s/tsys_read* %s/", inDir, outDir);
  system(shellCommand);
  printf("\twe_read\n");
  sprintf(shellCommand, "cp %s/we_read* %s/", inDir, outDir);
  system(shellCommand);
  /* Now make the new files with the additional chunks */
  printf("Making the files which need modification\n");

  if (!justRegrid) {
    int haveSeenBand = FALSE;
    int haveWrittenNewCodes = FALSE;
    int lastCode = -1;

    /* printf("\tAdding new codes\n"); */
    sprintf(fileName, "%s/codes_read", inDir);
    codesInFId = open(fileName, O_RDONLY);
    if (codesInFId < 0) {
      perror("Open of codes_read");
      exit(ERROR);
    }
    sprintf(fileName, "%s/codes_read", outDir);
    codesOutFId = open(fileName, O_WRONLY|O_CREAT);
    if (codesOutFId < 0) {
      perror("Open of codes_read (new)");
      exit(ERROR);
    }
    nRead = sizeof(oldCode);
    while (nRead == sizeof(oldCode)) {
      nRead = read(codesInFId, &oldCode, sizeof(oldCode));
      if (nRead == sizeof(oldCode)) {
	/*
	printf("\tv_name = \"%s\", icode = %d, code = \"%s\", ncode = %d\n",
	       oldCode.v_name, oldCode.icode, oldCode.code, oldCode.ncode);
	*/
	if (!haveWrittenNewCodes) {
	  if ((!haveSeenBand) && (!strcmp(oldCode.v_name, "band")) && (oldCode.code[0] == 's')) {
	    /* printf("OK, here come the bands\n"); */
	    haveSeenBand = TRUE;
	  }
	  if ((haveSeenBand) && (strcmp(oldCode.v_name, "band"))) {
	    int i;
	    codehDef newCode;

	    /* printf("Here come the NEW bands (%d)\n", lastCode); */
	    bcopy(&oldCode, &newCode, sizeof(newCode));
	    sprintf(newCode.v_name, "band");
	    for (i = lastCode+1; i < lastCode+1+(nSynth-2); i++) {
	      sprintf(newCode.code, "s%02d", i);
	      newCode.icode = newCode.ncode = i;
	      /*
	      printf("\t\tv_name = \"%s\", icode = %d, code = \"%s\", ncode = %d\n",
		     newCode.v_name, newCode.icode, newCode.code, newCode.ncode);
	      */
	      write(codesOutFId, &newCode, sizeof(newCode));
	    }
	    haveWrittenNewCodes = TRUE;
	  }
	}
	write(codesOutFId, &oldCode, sizeof(oldCode));
	lastCode = oldCode.icode;
      }
    }
  }

  sprintf(fileName, "%s/sch_read", inDir);
  schInFId = open(fileName, O_RDONLY);
  if (schInFId < 0) {
    perror("Open of sch_read");
    exit(ERROR);
  }
  sprintf(fileName, "%s/sch_read", outDir);
  schOutFId = open(fileName, O_WRONLY|O_CREAT);
  if (schOutFId < 0) {
    perror("Open of sch_read (new)");
    exit(ERROR);
  }
  sprintf(fileName, "%s/sp_read", inDir);
  inFId = open(fileName, O_RDONLY);
  if (inFId < 0) {
    perror("Open of sp_read");
    exit(ERROR);
  }
  sprintf(fileName, "%s/sp_read", outDir);
  outFId = open(fileName, O_WRONLY|O_CREAT);
  if (outFId < 0) {
    perror("Open of sp_read (new)");
    exit(ERROR);
  }
  
  lastInhid = schDataSize = UNINITIALIZED;
  nRead = sizeof(oldSp);
  nSynthSize = 0;
  if (!justRegrid) {
    /* Calculate how much larger than the input sch_read buffer the output sch_read buffer will be */
    chunkSpec *ptr;
    
    ptr = newChunkList;
    
    while (ptr != NULL) {
      nSynthSize += 2*MAX_BASELINES*(4*(1 + (1 + ptr->endChan - ptr->startChan)/ptr->nAve) + 2);
      ptr = ptr->next;
    }
    nSynthSize *= 2; /* Paranoia */
  }
  while (nRead == sizeof(oldSp)) {
    int found = FALSE;
    int factor = 1;

    nRead = read(inFId, &oldSp, sizeof(oldSp));
    if (nRead == sizeof(oldSp)) {
      if (oldSp.inhid != lastInhid) {
	int schNRead;

	if ((oldSp.inhid % 100) == 0)
	  printf("Processing scan: %d\n", oldSp.inhid);
	if (data != NULL) {
	  /* Not the first record - need to write out last scan's data */

	  newHeader[1] = outPtr*sizeof(short);
	  write(schOutFId, &newHeader[0], 2*sizeof(int));
	  if (outPtr*sizeof(short) > newDataSize) {
	    fprintf(stderr, "Output buffer overflow (2): max: %d, now: %d - abort\n", newDataSize, outPtr);
	    exit(ERROR);
	  }
	  write(schOutFId, &newData[0], outPtr*sizeof(short));
	}
	schNRead = read(schInFId, &header[0], 2*sizeof(int));
	if (schNRead != 2*sizeof(int)) {
	  fprintf(stderr, "Only got %d bytes from sch_read - should have gotten %ld\n", schNRead, 2*sizeof(int));
	  exit(ERROR);
	}
	if (schDataSize == UNINITIALIZED) {
	  /* printf("sch data size is %d\n", header[1]); */
	  schDataSize = header[1];
	} else if (schDataSize != header[1]) {
	  fprintf(stderr, "data section size change - was %d, is now %d\n", schDataSize, header[1]);
	  exit(ERROR);
	}
	if (oldSp.inhid != header[0]) {
	  fprintf(stderr, "sp thinks inhid is %d, sch thinks inhid = %d - abort\n", oldSp.inhid, header[0]);
	  exit(ERROR);
	}
	if (data == NULL) {
	  data = (short *)malloc(schDataSize);
	  if (data == NULL) {
	    perror("malloc of data");
	    exit(ERROR);
	  }
	  newDataSize = schDataSize + nSynthSize;
	  newData = (short *)malloc(newDataSize);
	  if (newData == NULL) {
	    perror("malloc of newData");
	    exit(ERROR);
	  }
	}
	newHeader[0] = header[0];
	newSize = outPtr = 0;
	
	schNRead = read(schInFId, data, schDataSize);
	if (schNRead != schDataSize) {
	  fprintf(stderr, "Didn't get the data I needed from sch_read - wanted %d, got %d - abort\n",
		  schDataSize, schNRead);
	  exit(ERROR);
	}
	lastInhid = oldSp.inhid;
      }
      found = FALSE;
      if ((oldSp.iband == 49) || (oldSp.iband == 50)) {
	chunkSpec *ptr;

	ptr = newChunkList;

	while (ptr != NULL) {
	  if (oldSp.iband == ptr->sourceChunk) {
	    int i, j, oldPtr;
	    double realSum, imagSum, real, imag;
	    
	    found = TRUE;
	    sChan = ptr->startChan;
	    eChan = ptr->endChan;
	    factor = ptr->nAve;
	    
	    /* Regrid the chunk! */
	    
	    oldPtr = oldSp.dataoff/2;
	    newData[outPtr] = data[oldPtr];
	    /* printf("\tReducing the resolution of band %d by a factor of %d\n", oldSp.iband, factor); */
	    bcopy(&oldSp, &newSp, sizeof(newSp));
	    newSp.dataoff = outPtr*2;
	    newSp.vres *= factor;
	    if ((sChan != 0) || (eChan != (oldSp.nch-1))) {
	      double n, f, fr, fs, fe;
	      
	      f = oldSp.fsky;
	      fr = oldSp.fres/1.0e3;
	      n = (double)oldSp.nch;
	      fs = f - fr*(n/2.0 - (double)sChan);
	      fe = f - fr*(n/2.0 - (double)eChan);
	      newSp.fsky = (fs + fe)/2.0;
	    }
	    newSp.fres *= factor;
	    newSp.iband = ptr->iband;
	    outPtr++;
	    for (i = sChan/factor; i < (eChan+1)/factor; i++) {
	      realSum = imagSum = 0.0;
	      for (j = 0; j < factor; j++) {
		real = (double)data[1 + oldPtr + 2*j + i*factor*2];
		imag = (double)data[2 + oldPtr + 2*j + i*factor*2];
		realSum += real;
		imagSum += imag;
	      }
	      realSum /= (double)factor;
	      imagSum /= (double)factor;
	      newData[outPtr++] = (short)realSum;
	      newData[outPtr++] = (short)imagSum;
	    }
	    newSp.nch = (oldSp.nch - sChan - ((oldSp.nch-1) - eChan))/factor;
	    write(outFId, &newSp, sizeof(newSp));
	  }
	  ptr = ptr->next;
	}
      }
      if (!found) {
	/* Nothing needs to be done to this band - just pass it through */
	bcopy(&data[oldSp.dataoff/2], &newData[outPtr], 4*oldSp.nch + 2);
	oldSp.dataoff = outPtr*2;
	outPtr +=  2*oldSp.nch + 1;
	write(outFId, &oldSp, sizeof(oldSp));
      }
    }
  }
  newHeader[1] = outPtr*sizeof(short);
  write(schOutFId, &newHeader[0], 2*sizeof(int));
  if (outPtr*sizeof(short) > newDataSize) {
    fprintf(stderr, "Output buffer overflow (2): max: %d, now: %d - abort\n", newDataSize, outPtr);
    exit(ERROR);
  }
  write(schOutFId, &newData[0], outPtr*sizeof(short));
  sprintf(shellCommand, "chmod 666 %s/*", outDir);
  system(shellCommand);
  return(0);
}
