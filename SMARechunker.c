#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define TRUE  (1)
#define FALSE (0)
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

typedef struct __attribute__((packed)) blhDef {
  int 	blhid     ; /*	proj. baseline id #  This is a unique identifier for this record         */
  int 	inhid     ; /*	integration id #                                                         */
  short	isb       ; /*	sideband int code                                                        */
  short	ipol      ; /*	polarization int code                                                    */
                    /*  0 = Unknown                                                              */
                    /*  1 = RR                                                                   */
                    /*  2 = RL                                                                   */
                    /*  3 = LR                                                                   */
                    /*  4 = LL                                                                   */
  short	ant1rx    ; /*  This flag is meaningful only in Dual Rx Polar mode, when it is used to   */
                    /*  specify the receiver used on the lower number receiver on the baseline.  */
                    /*  0 = 345 Rx                                                               */
                    /*  1 = 400 Rx                                                               */
                    /*  anything else is bad data now, but additional receivers may be defined   */
                    /*  in the future.                                                           */
  short	ant2rx    ; /*  This flag is meaningful only in Dual Rx Polar mode, when it is used to   */
                    /*  specify the receiver used on the higer number receiver on the baseline.  */
                    /*  0 = 345 Rx                                                               */
                    /*  1 = 400 Rx                                                               */
                    /*  anything else is bad data now, but additional receivers may be defined   */
                    /*  in the future.                                                           */
  short	pointing  ; /*	pointing data int code                                                   */
                    /*  = 1 for no offsets                                                       */
                    /*  = 0 otherwise                                                            */
                    /*  Used to indicate off source ipoint scans.   This flag is not used for    */
                    /* mosaicing offsets, etc.                                                   */
  short	irec      ; /*	receiver int code                                                        */
                    /*  0 = 230                                                                  */
                    /*  1 = 345                                                                  */
                    /*  2 = 400                                                                  */
                    /*  3 = 600                                                                  */
                    /* -1 = Rx unknown                                                           */
  float	u         ; /*	u coord. for bsl (klambda)                                               */
  float	v         ; /*	v coord. for bsl (klambda)                                               */
  float	w         ; /*	w coord. for bsl (klambda)                                               */
  float	prbl      ; /*	projected baseline                                                       */
  float	coh       ; /*	coherence estimate - This is the ratio of the vector average (over       */
                    /*  spectral channels) to the scalar average, over all chunks (weighted by   */
                    /*  channel bandwidth to give equal weight to every Hz interval).            */
  double avedhrs  ; /*	This is the midpoint time for the scan, in hours.                        */
  float	ampave    ; /*	ave continuum amp (vector average of all channels)                       */
  float	phaave    ; /*	ave continuum phase                                                      */
  int 	blsid     ; /*	physical baseline id #  This is an integer label for this baseline       */
  short	iant1     ; /*	antenna number of first antenna in baseline                              */
  short	iant2     ; /*	antenna number of second antenna in baseline                             */
  int ant1TsysOff ; /*  Byte offset to start of Tsys information for first antenna of this       */
                    /*  baseline and this scan.  This is a byte offset to the data in the        */
                    /*  tsys_read file, for the data corresponding to the first antenna on this  */
                    /*  baseline, for one particular scan (identified by inhid).                 */ 
  int ant2TsysOff ; /*  Byte offset for Tsys data for the second antenna in the baseline         */
  short	iblcd     ; /*	baseline int code                                                        */
  float	ble       ; /*	bsl east vector (klambda)                                                */
  float	bln       ; /*	bsl north vector klambda)                                                */
  float	blu       ; /*	bsl up vector   klambda)                                                 */
  int spareint1   ; /*  Spare Int. for (currently used as antenna 1 offset to tsys_read2 file)   */
  int spareint2   ; /*  (currently used as antenna 2 offset to tsys_read2 file)                  */
  int spareint3   ; /*                                                                           */
  int spareint4   ; /*                                                                           */
  int spareint5   ; /*                                                                           */
  int spareint6   ; /*                                                                           */
  double sparedbl1; /* Spare Dbl. for future use                                                 */
  double spatedbl2; /*                                                                           */
  double sparedbl3; /*                                                                           */
  double sparedbl4; /*                                                                           */
  double sparedbl5; /*                                                                           */
  double sparedbl6; /*                                                                           */
} blhDef;
/* the size of blhDef is 158 bytes */

typedef struct __attribute__((packed)) antEngDef {
  int antennaNumber;
  int padNumber;
  int antennaStatus;  /*  Antenna is ON or OFF LINE                                              */
  int trackStatus;    /*  Track is running or not                                                */
  int commStatus;     /*  Data for this integration is valid or not                              */
  int inhid    ;      /*  integration id #                                                       */
  int ints     ;      /*  integration #                                                          */
  double dhrs  ;      /*  hrs from ref_time                                                      */
  double ha    ;      /*  hour angle                                                             */
  double lst   ;      /*  lst                                                                    */
  double pmdaz ;      /*  pointing model correction                                              */
  double pmdel ;
  double tiltx ;
  double tilty ;
  double actual_az ;
  double actual_el ;
  double azoff ;
  double eloff ;
  double az_tracking_error ;
  double el_tracking_error ;
  double refraction ;
  double chopper_x ;
  double chopper_y ;
  double chopper_z ;
  double chopper_angle ;
  double tsys ;
  double tsys_rx2 ;
  double ambient_load_temperature ;
  
} antEngDef;
/* The size of antEngDef is 196 bytes */

typedef struct __attribute__((packed)) inhDef {
  int 	traid     ; /*  track id # Set to the Project ID.   This is the realtime system's         */
                    /*  project ID, created by the "project" command, and has nothing to do with  */
                    /*  the proposal ID                                                           */
  int 	inhid     ; /*  integration id #                                                          */
  int 	ints      ; /*  integration # (scan number)                                               */
                    /*  In reality, same as inhid                                                 */
  float	az        ; /*  azimuth  (degrees)                                                        */
  float	el        ; /*  elevation (degrees)                                                       */
  float	ha        ; /*  hour angle (hours)                                                        */
  short	iut       ; /*  Scan number                                                               */
  short	iref_time ; /*  ref_time int code (points to a codes_read entry)                          */
  double dhrs     ; /*  average time at scan midpoint in hours                                    */
  float	vc        ; /*  Radial vel. - catalog vel. in km/sec                                      */
  double sx       ; /*  x vec. for bsl. (unit vector x component towards source)                  */
  double sy       ; /*  y vec. for bsl.                                                           */
  double sz       ; /*  z vec. for bsl.                                                           */
  float	rinteg    ; /*  actual int time (legth of scan in seconds)                                */
  int 	proid     ; /*  project id #                                                              */
  int 	souid     ; /*  source id #                                                               */
  short	isource   ; /*  source int code (references a codes_read entry)                           */
  short	ivrad     ; /*  Index number for the radial velocity entry in codes_read                  */
  float	offx      ; /*  offset in x (for mosaics) used for RA offset  (arcsec)                    */
  float	offy      ; /*  offset in y  (for mosaics) used for Dec offset (arcsec)                   */
  short	ira       ; /*  ra int code                                                               */
  short	idec      ; /*  dec int code                                                              */
  double rar      ; /*  ra (radians)                                                              */
  double decr     ; /*  declination (radians)                                                     */
  float	epoch     ; /*  epoch for coordinates, always set to 2000.0                               */
  float	size      ; /*  source size (arcsec) Only nonzero for planets                             */
  int spareint1   ; /*  Spare integer for future use                                              */
  int spareint2   ; /*                                                                            */
  int spareint3   ; /*                                                                            */
  int spareint4   ; /*                                                                            */
  int spareint5   ; /*                                                                            */
  int spareint6   ; /*                                                                            */
  double sparedbl1; /*  Spare double for future use                                               */
  double spatedbl2; /*                                                                            */
  double sparedbl3; /*                                                                            */
  double sparedbl4; /*                                                                            */
  double sparedbl5; /*                                                                            */
  double sparedbl6; /*                                                                            */
} inhDef;
/* The size of inhDef is 188 bytes */

typedef struct __attribute__((packed)) wehDef {
  /*
     Note the following arrays contain 11 elements.   Element n is used for antenna n, except in
     the case of n=0, which stores the values from the observatory's weather station on the hangar.
  */
  int scanNumber;      /*  The scan number for which this info applies                            */
  int flags[11];       /*  Flagging information from statusServer.                                */
                       /*  Here are the flags which have been defined:                            */
  /* Note that slot 0 of the following arrays contains the "observatory weather (usually the      */
  /* Hangar weather station weather, and slot n contains the weather from the weather station on  */
  /* antenna n.    The antenna weather stations only measure temperature, pressure and humidity.  */
  float N[11];         /*  The refractivity used for the atmospheric delay correction for each    */
                       /*  antenna.                                                               */
  float Tamb[11];      /*  The ambient temperature (C) used for each antenna's  atmospheric delay */
                       /*  correction.                                                            */
  float pressure[11];  /*  The atmospheric pressure, in mbar, used for each antenna's atmospheric */
                       /*  delay correction.                                                      */
  float humid[11];     /*  The relative humidity, in percent, used for each antenna's atmospheric */
                       /*  delay correction.                                                      */
  float windSpeed[11]; /*  Wind speed in m/sec at each antenna. -1.0 if no hardware exists to     */
                       /*  measure this                                                           */
  float windDir[11];   /*  Wind direction, in radians measured from north through east, for each  */
                       /*  antenna. -1.0 if no hardware exists.                                   */
  float h2o[11];       /*  The boresite precipitable water vapor measured at each antenna.        */
                       /*  -1.0 if no hardware exists.                                            */
} wehDef;
/* The size of wehDef is 356 bytes */

typedef struct chunkSpec {
  int sourceChunk;
  int startChan;
  int endChan;
  int nAve;
  int iband;
  struct chunkSpec *next;
} chunkSpec;

chunkSpec *newChunkList = NULL;

float buffer[33000];
int bufferPtr = 0;
int sWARMOnlyTrack = TRUE;
int sWARMOffset = 0;

int isLegalN(int n) {
  return ((n == 1) || (n == 2) || (n == 4) || (n == 8) || (n == 16) || (n == 32) ||
	  (n == 64) || (n == 128) || (n == 256) || (n == 512) || (n == 1024));
}

void printUsage(char *name) {
  printf("Usage:\n");
  printf("%s -i {input directory} -o {output directory} [-f {first scan number}] [-l {last scan number}] [-d] [-r {n} reduce all SWARM chunks by a factor of n}] {chunk spec} {chunk spec} ...\n", name);
  printf("Use -S for a SWARM-only track\n");
  exit(0);
}

int main (int argc, char **argv)
{
  unsigned int i, j;
  int firstScanNumber = -1;
  int lastScanNumber = -1;
  int globalAverage = FALSE;
  int copyAllScans = TRUE;
  int nRead, nSynth, justRegrid, eChan, sChan;
  int newBandCounter = 0;
  int lastInhid, schDataSize, codesInFId, codesOutFId;
  int outputDefault = FALSE;
  int gotInput = FALSE;
  int gotOutput = FALSE;
  int header[2];
  int nSynthSize = 0;
  int newDataSize = 0;
  int *newHeader = NULL;
  int maxSWARMChunk = 0;
  int newSize;
  int outPtr = 0;
  int nSWARMChunks = 0;
  float invFactor;
  short *data = NULL;
  short *newData = NULL;
  short *bigData = NULL;
  char *inDir = NULL;
  char *outDir = NULL;
  char shellCommand[1000], fileName[1000];
  chunkSpec *newChunk = NULL;
  chunkSpec *lastChunk = NULL;
  codehDef oldCode;
  sphDef oldSp, newSp;
  FILE *inFId, *outFId, *schInFId, *schOutFId;

  while ((i = getopt(argc, argv, "Adi:f:l:o:r:S")) != -1) {
    switch (i) {
    case 'A':
      sWARMOnlyTrack = FALSE;
      sWARMOffset = 48;
      break;
    case 'd':
      outputDefault = TRUE;
      break;
    case 'i':
      gotInput = TRUE;
      inDir = optarg;
      break;
    case 'f':
      firstScanNumber = atoi(optarg);
      copyAllScans = FALSE;
      break;
    case 'l':
      lastScanNumber = atoi(optarg);
      copyAllScans = FALSE;
      break;
    case 'o':
      gotOutput = TRUE;
      outDir = optarg;
      break;
    case 'r':
      globalAverage = atoi(optarg);
      if (globalAverage & (globalAverage - 1)) {
	fprintf(stderr, "The argument for -r must be a power of 2\n");
	exit(ERROR);
      }
      if (globalAverage > 4096) {
	fprintf(stderr, "The argument for -r must be less than 8192\n");
	exit(ERROR);
      }
      break;
    case 'S':
      fprintf(stderr, "The -S switch is no longer needed, SWARM-only is the default\n");
      fprintf(stderr, "Use -A if the track contains ASIC correlator data\n");
      sWARMOnlyTrack = TRUE;
      sWARMOffset = 0;
      break;
    default:
      printUsage(argv[0]);
    }
  }
  if (!gotInput || !gotOutput)
    printUsage(argv[0]);
  {
    int codeFId, done, bandCount;

    sprintf(fileName, "%s/codes_read", inDir);
    codeFId = open(fileName, O_RDONLY);
    if (codeFId < 0) {
      perror("Open input codes_read");
      exit(ERROR);
    }
    done = FALSE;
    bandCount = 0;
    do {
      nRead = read(codeFId, &oldCode, sizeof(oldCode));
      if (nRead == sizeof(oldCode)) {
	if ((!strcmp(oldCode.v_name, "ut") && (bandCount > 1)))
	  done = TRUE;
	else if (!strcmp(oldCode.v_name, "band")) {
	  bandCount++;
	  if (oldCode.icode > sWARMOffset)
	    nSWARMChunks++;
	}
      }
    } while ((nRead == sizeof(oldCode)) && !done);
    close(codeFId);
    maxSWARMChunk = sWARMOffset+nSWARMChunks;
    if (nSWARMChunks == 0) {
      fprintf(stderr, "No SWARM data seen in the input data directory - aborting\n");
      exit(ERROR);
    }
    printf("Number of SWARM bands seen: %d\n", nSWARMChunks);
  }
  if ((optind >= argc) & (!globalAverage)) {
    fprintf(stderr, "You must specify at least one chunk specification or use -r {n}\n");
    printUsage(argv[0]);
  } else if (globalAverage && ((argc-optind) > 0)) {
    fprintf(stderr, "You cannot specify both -r {n} and individual chunk specifications\n");
    exit(ERROR);
  } else
    nSynth = argc-optind;
  if ((!outputDefault && (nSynth <= nSWARMChunks)) || globalAverage)
    justRegrid = TRUE;
  else
    justRegrid = FALSE;
  if (outputDefault) {
    /* Define two "new" chunks which are just the raw SWARM chunks without any changes */
    for (i = sWARMOffset+1; i <= maxSWARMChunk; i++) {
      newChunk = malloc(sizeof(*newChunk));
      if (newChunk == NULL) {
	perror("newChunk malloc");
	exit(ERROR);
      }
      newChunk->sourceChunk = i;
      newChunk->startChan = 0;
      newChunk->endChan = 16383;
      newChunk->nAve = 1;
      newChunk->iband = sWARMOffset+1+newBandCounter++;
      newChunk->next = NULL;
      if (newChunkList == NULL)
	newChunkList = newChunk;
      else
	lastChunk->next = newChunk;
      lastChunk = newChunk;    
    }
  }
  if (globalAverage) {
    for (i = 0; i < nSWARMChunks; i++) {
      newChunk = malloc(sizeof(*newChunk));
      if (newChunk == NULL) {
	perror("newChunk malloc");
	exit(ERROR);
      }
      newChunk->sourceChunk = sWARMOffset+1+i;
      newChunk->startChan = 0;
      newChunk->endChan = 16383;
      newChunk->nAve = globalAverage;
      newChunk->iband = sWARMOffset+1+newBandCounter++;
      newChunk->next = NULL;
      if (newChunkList == NULL)
	newChunkList = newChunk;
      else
	lastChunk->next = newChunk;
      lastChunk = newChunk;
    }
  } else {
    for (i = 0; i < nSynth; i++) {
      int chunk, n, nRead;
      
      nRead = sscanf(argv[i+optind], "%d:%d:%d:%d", &chunk, &sChan, &eChan, &n);
      if (nRead != 4) {
	fprintf(stderr, "Error in new chunk spcification #%d\n", i+1);
	fprintf(stderr, "Cannot parse chunk specification \"%s\"\n", argv[i+optind]);
	exit(ERROR);
      }
      if ((chunk < sWARMOffset+1) || (chunk > maxSWARMChunk)) {
	fprintf(stderr, "Error in new chunk spcification #%d\n", i+1);
	fprintf(stderr, "\"%s\" is an invalid chunk specifier - input chunk (%d) must be between %d and %d inclusive\n",
		argv[i+optind], chunk, sWARMOffset+1, maxSWARMChunk);
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
      newChunk->iband = sWARMOffset+1+newBandCounter++;
      newChunk->next = NULL;
      if (newChunkList == NULL)
	newChunkList = newChunk;
      else
	lastChunk->next = newChunk;
      lastChunk = newChunk;
    }
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
  if (copyAllScans) {
    sprintf(shellCommand, "cp %s/bl_read %s/", inDir, outDir);
    system(shellCommand);
  } else {
    /* Explicit scan numbers were given, so I can't just copy the file */
    int nRead, blFDi, blFDo;
    char fileName[100];
    blhDef blh;

    sprintf(fileName, "%s/bl_read", inDir);
    blFDi = open(fileName, O_RDONLY);
    if (blFDi < 0) {
      perror("Open input bl_read");
      exit(ERROR);
    }
    sprintf(fileName, "%s/bl_read", outDir);
    blFDo = open(fileName, O_WRONLY|O_CREAT);
    if (blFDo < 0) {
      perror("Open output bl_read");
      exit(ERROR);
    }
    do {
      nRead = read(blFDi, &blh, sizeof(blh));
      if (nRead == sizeof(blh)) {
	if ((firstScanNumber <= blh.inhid) && (lastScanNumber >= blh.inhid))
	  write(blFDo, &blh, sizeof(blh));
      }
    } while (nRead == sizeof(blh));
    close(blFDi);
    close(blFDo);
  }
  if (justRegrid) {
    printf("\tcodes_read\n");
    sprintf(shellCommand, "cp %s/codes_read %s/", inDir, outDir);
    system(shellCommand);
  }
  printf("\tcodeVersions\n");
  sprintf(shellCommand, "cp %s/codeVersions %s/", inDir, outDir);
  system(shellCommand);
  printf("\teng_read\n");
  if (copyAllScans) {
    sprintf(shellCommand, "cp %s/eng_read %s/", inDir, outDir);
    system(shellCommand);
  } else {
    /* Explicit scan numbers were given, so I can't just copy the file */
    int nRead, engFDi, engFDo;
    char fileName[100];
    antEngDef antEng;

    sprintf(fileName, "%s/eng_read", inDir);
    engFDi = open(fileName, O_RDONLY);
    if (engFDi < 0) {
      perror("Open input eng_read");
      exit(ERROR);
    }
    sprintf(fileName, "%s/eng_read", outDir);
    engFDo = open(fileName, O_WRONLY|O_CREAT);
    if (engFDo < 0) {
      perror("Open output eng_read");
      exit(ERROR);
    }
    do {
      nRead = read(engFDi, &antEng, sizeof(antEng));
      if (nRead == sizeof(antEng)) {
	if ((firstScanNumber <= antEng.inhid) && (lastScanNumber >= antEng.inhid))
	  write(engFDo, &antEng, sizeof(antEng));
      }
    } while (nRead == sizeof(antEng));
    close(engFDi);
    close(engFDo);
  }
  printf("\tin_read\n");
  if (copyAllScans) {
    sprintf(shellCommand, "cp %s/in_read %s/", inDir, outDir);
    system(shellCommand);
  } else {
    /* Explicit scan numbers were given, so I can't just copy the file */
    int nRead, inFDi, inFDo;
    char fileName[100];
    inhDef inh;

    sprintf(fileName, "%s/in_read", inDir);
    inFDi = open(fileName, O_RDONLY);
    if (inFDi < 0) {
      perror("Open input in_read");
      exit(ERROR);
    }
    sprintf(fileName, "%s/in_read", outDir);
    inFDo = open(fileName, O_WRONLY|O_CREAT);
    if (inFDo < 0) {
      perror("Open output in_read");
      exit(ERROR);
    }
    do {
      nRead = read(inFDi, &inh, sizeof(inh));
      if (nRead == sizeof(inh)) {
	if ((firstScanNumber <= inh.inhid) && (lastScanNumber >= inh.inhid))
	  write(inFDo, &inh, sizeof(inh));
      }
    } while (nRead == sizeof(inh));
    close(inFDi);
    close(inFDo);
  }
  printf("\tmodeInfo\n");
  sprintf(shellCommand, "cp %s/modeInfo %s/", inDir, outDir);
  system(shellCommand);
  printf("\tipoint files\n");
  sprintf(shellCommand, "cp %s/ipointResults.* %s/ 2>/dev/null", inDir, outDir);
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
  if (copyAllScans) {
    sprintf(shellCommand, "cp %s/we_read* %s/", inDir, outDir);
    system(shellCommand);
  } else {
    /* Explicit scan numbers were given, so I can't just copy the file */
    int nRead, weFDi, weFDo;
    char fileName[100];
    wehDef weh;

    sprintf(fileName, "%s/we_read", inDir);
    weFDi = open(fileName, O_RDONLY);
    if (weFDi < 0) {
      perror("Open input we_read");
      exit(ERROR);
    }
    sprintf(fileName, "%s/we_read", outDir);
    weFDo = open(fileName, O_WRONLY|O_CREAT);
    if (weFDo < 0) {
      perror("Open output we_read");
      exit(ERROR);
    }
    do {
      nRead = read(weFDi, &weh, sizeof(weh));
      if (nRead == sizeof(weh)) {
	if ((firstScanNumber <= weh.scanNumber) && (lastScanNumber >= weh.scanNumber))
	  write(weFDo, &weh, sizeof(weh));
      }
    } while (nRead == sizeof(weh));
    close(weFDi);
    close(weFDo);
  }

  /* Now make the new files with the additional chunks */
  printf("Making the files which need modification\n");

  if (!justRegrid) {
    int haveSeenBand = FALSE;
    int haveWrittenNewCodes = FALSE;
    int lastCode = -1;

    printf("\tAdding new codes\n");
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
	if (!haveWrittenNewCodes) {
	  if ((!haveSeenBand) && (!strcmp(oldCode.v_name, "band")) && (oldCode.code[0] == 's')) {
	    haveSeenBand = TRUE;
	  }
	  if ((haveSeenBand) && (strcmp(oldCode.v_name, "band"))) {
	    int i;
	    codehDef newCode;

	    memcpy(&newCode, &oldCode, sizeof(newCode));
	    sprintf(newCode.v_name, "band");
	    for (i = lastCode+1; i < lastCode+1+(nSynth-2); i++) {
	      sprintf(newCode.code, "s%02d", i);
	      newCode.icode = newCode.ncode = i;
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
  schInFId = fopen(fileName, "rb");
  if (schInFId == NULL) {
    perror("Open of sch_read");
    exit(ERROR);
  }
  sprintf(fileName, "%s/sch_read", outDir);
  schOutFId = fopen(fileName, "wb");
  if (schOutFId == NULL) {
    perror("Open of sch_read (new)");
    exit(ERROR);
  }
  sprintf(fileName, "%s/sp_read", inDir);
  inFId = fopen(fileName, "rb");
  if (inFId == NULL) {
    perror("Open of sp_read");
    exit(ERROR);
  }
  sprintf(fileName, "%s/sp_read", outDir);
  outFId = fopen(fileName, "wb");
  if (outFId == NULL) {
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
  printf("\tsp_read and sch_read\n");
  while (nRead == sizeof(oldSp)) {
    int found = FALSE;
    int factor = 1;

    nRead = fread_unlocked(&oldSp, 1, sizeof(oldSp), inFId);
    if (nRead == sizeof(oldSp)) {
      if (copyAllScans || ((firstScanNumber <= oldSp.inhid) && (oldSp.inhid <= lastScanNumber))) {
	if (oldSp.inhid != lastInhid) {
	  int schNRead;
	  
	  if ((oldSp.inhid % 100) == 0)
	    printf("Processing scan: %d\n", oldSp.inhid);
	  if (data != NULL) {
	    /* Not the first record - need to write out last scan's data */
	    
	    newHeader[1] = outPtr*sizeof(short);
	    if (outPtr*sizeof(short) > newDataSize) {
	      fprintf(stderr, "Output buffer overflow (2): max: %d, now: %d - abort\n", newDataSize, outPtr);
	      exit(ERROR);
	    }
	    fwrite_unlocked(&bigData[0], 1, outPtr*sizeof(short) + 2*sizeof(int), schOutFId);
	  }
	  schNRead = fread_unlocked(&header[0], 1, 2*sizeof(int), schInFId);
	  if (schNRead != 2*sizeof(int)) {
	    fprintf(stderr, "Only got %d bytes from sch_read - should have gotten %ld\n", schNRead, 2*sizeof(int));
	    exit(ERROR);
	  }
	  if (schDataSize == UNINITIALIZED) {
	    schDataSize = header[1];
	  } else if (schDataSize != header[1]) {
	    fprintf(stderr, "data section size change at scan %d  - was %d, is now %d\n", oldSp.inhid, schDataSize, header[1]);
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
	    bigData = (short *)malloc(newDataSize + 2*sizeof(int));
	    if (bigData == NULL) {
	      perror("malloc of bigData");
	      exit(ERROR);
	    }
	    newData = &bigData[4];
	    newHeader = (int *)(&bigData[0]);
	  }
	  newHeader[0] = header[0];
	  newSize = outPtr = 0;
	  
	  schNRead = fread_unlocked(data, 1, schDataSize, schInFId);
	  if (schNRead != schDataSize) {
	    fprintf(stderr, "Didn't get the data I needed from sch_read - wanted %d, got %d - abort\n",
		    schDataSize, schNRead);
	    exit(ERROR);
	  }
	  lastInhid = oldSp.inhid;
	}
	found = FALSE;
	if ((oldSp.iband >= sWARMOffset+1) && (oldSp.iband <= maxSWARMChunk)) {
	  int oldMin, oldMax, oldExp;
	  float ratio;
	  chunkSpec *ptr;
	  
	  ptr = newChunkList;
	  
	  while (ptr != NULL) {
	    if (oldSp.iband == ptr->sourceChunk) {
	      unsigned int high, oldPtr, savedPtr;
	      int realIntSum, imagIntSum;
	      short *tData;
	      
	      found = TRUE;
	      sChan = ptr->startChan;
	      eChan = ptr->endChan;
	      factor = ptr->nAve;
	      invFactor = 1.0/((float)factor);
	      
	      /* Regrid the chunk! */
	      
	      oldPtr = oldSp.dataoff/2;
	      newData[outPtr] = data[oldPtr];
	      savedPtr = outPtr;
	      memcpy(&newSp, &oldSp, sizeof(newSp));
	      newSp.dataoff = 2*outPtr++;
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
	      newSp.wt *= factor;
	      newSp.iband = ptr->iband;
	      oldExp = data[oldPtr];
	      tData = &data[1+oldPtr + 2*sChan];
	      high = (eChan+1)/factor;
	      bufferPtr = 0;
	      outPtr += 2;
	      for (i = sChan/factor; i < high; i++) {
		realIntSum = imagIntSum = 0;
		/*
		  This somewhat ugly switch is used to allow the compiler to know the size of the loop
		  for small values of "factor".   That in turn allows the compiler to unroll the loop
		  in these cases.
		*/
		switch (factor) {
		case 1:
		  realIntSum += *tData++;
		  imagIntSum += *tData++;
		  break;
		case 2:
		  for (j = 0; j < 2; j++) {
		    realIntSum += *tData++;
		    imagIntSum += *tData++;
		  }
		  break;
		case 4:
		  for (j = 0; j < 4; j++) {
		    realIntSum += *tData++;
		    imagIntSum += *tData++;
		  }
		  break;
		case 8:
		  for (j = 0; j < 8; j++) {
		    realIntSum += *tData++;
		    imagIntSum += *tData++;
		  }
		  break;
		case 16:
		  for (j = 0; j < 16; j++) {
		    realIntSum += *tData++;
		    imagIntSum += *tData++;
		  }
		  break;
		case 32:
		  for (j = 0; j < 32; j++) {
		    realIntSum += *tData++;
		    imagIntSum += *tData++;
		  }
		  break;
		default:
		  for (j = 0; j < factor; j++) {
		    realIntSum += *tData++;
		    imagIntSum += *tData++;
		  }
		}
		buffer[bufferPtr++] = (float)realIntSum * invFactor;
		buffer[bufferPtr++] = (float)imagIntSum * invFactor;
	      }
	      /* See if the scale factor for this spectrum should be changed */
	      oldMax = -1000000000;
	      oldMin =  1000000000;
	      for (i = 0; i < bufferPtr; i++) {
		if (buffer[i] < oldMin)
		  oldMin = buffer[i];
		if (buffer[i] > oldMax)
		  oldMax = buffer[i];
	      }
	      if ((-oldMin) > oldMax)
		oldMax = -oldMin;
	      if (oldMax != 0)
		ratio = 32767.0f/(float)oldMax;
	      else
		ratio = 1.0f;
	      if (ratio >= 2.0f) {
		int newExp, iscale;
		float scale, scaleFactor;
		
		scale = log(ratio)/M_LN2;
		iscale = (int)scale;
		scaleFactor = powf(2.0f, (float)iscale);
		if (oldExp < 0)
		  newExp = oldExp - iscale;
		else
		  newExp = oldExp + iscale;
		for (i = 0; i < bufferPtr; i++)
		  newData[outPtr++] = buffer[i] * scaleFactor;
		newData[savedPtr] = newExp;
	      } else
		for (i = 0; i < bufferPtr; i++)
		  newData[outPtr++] = buffer[i];
	      newSp.nch = (oldSp.nch - sChan - ((oldSp.nch-1) - eChan))/factor;
	      fwrite_unlocked(&newSp, 1, sizeof(newSp), outFId);
	    }
	    ptr = ptr->next;
	  }
	}
	if (!found) {
	  /* Nothing needs to be done to this band - just pass it through */
	  memcpy(&newData[outPtr], &data[oldSp.dataoff/2], 4*oldSp.nch + 2);
	  oldSp.dataoff = outPtr*2;
	  outPtr +=  2*oldSp.nch + 1;
	  fwrite_unlocked(&oldSp, 1, sizeof(oldSp), outFId);
	}
      } /* End of block executed if the scan number is in range for copying */
    } /* End of block executed if the correct number of bytes are read (in other words, not EOF) */
  } /* End of main while loop processing the data */
  newHeader[1] = outPtr*sizeof(short);
  if (outPtr*sizeof(short) > newDataSize) {
    fprintf(stderr, "Output buffer overflow (2): max: %d, now: %d - abort\n", newDataSize, outPtr);
    exit(ERROR);
  }
  fwrite_unlocked(&bigData[0], 1, outPtr*sizeof(short) + 2*sizeof(int), schOutFId);
  sprintf(shellCommand, "chmod 666 %s/*", outDir);
  system(shellCommand);
  return(0);
}
