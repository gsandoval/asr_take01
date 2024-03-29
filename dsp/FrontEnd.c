/******************************************************************************
*
*      ETSI STQ WI007 DSR Front-End Feature Extraction Algorithm
*      C-language software implementation 
*      Version 1.1.3   May, 2003
*
******************************************************************************/
/*---------------------------------------------------------------------------
 *
 * Mel Cepstrum Parameterisation for Distributed Speech Recognition (DSR),
 * command line parsing and main function
 *
 * FILE NAME: FrontEnd.c
 * PURPOSE:   This file contains the main part of DSR front-end. Speech samples
 *            are read from input waveform file frame by frame. Feature
 *            extraction is performed for each frame by calling basic
 *            functions of the basic FE function package (see FEfunc.c).
 *            Feature vectors are output to file in HTK format.
 *            Command line arguments are handled by a command line parsing
 *            function.
 *
 *---------------------------------------------------------------------------*/

/*-----------------
 * File Inclusions
 *-----------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "fileio.h"
#include "FEfunc.h"

/*------------------------------------
 * Definition of Front-end Parameters 
 *------------------------------------*/
#define SAMPLING_FREQ_1           8  /*8kHz */
#define SAMPLING_FREQ_2          11  /*11kHz */
#define SAMPLING_FREQ_3          16  /*16kHz */

#define FRAME_LENGTH_1          200  /*25ms */
#define FRAME_LENGTH_2          256  /*23.27ms */
#define FRAME_LENGTH_3          400  /*25ms */

#define FRAME_SHIFT_1            80  /*10ms */
#define FRAME_SHIFT_2           110  /*10ms */
#define FRAME_SHIFT_3           160  /*10ms */

#define PRE_EMPHASIS           0.97

#define ENERGYFLOOR_FB        -50.0  /*0.0 */
#define ENERGYFLOOR_logE      -50.0  /*4.0 */

#define FFT_LENGTH_1            256
#define FFT_LENGTH_2            256
#define FFT_LENGTH_3            512
#define MAXWINDOWSIZE           512

#define NUM_CHANNELS             23
#define STARTING_FREQ_1        64.0  /*55.401825 */
#define STARTING_FREQ_2        64.0  /*63.817818 */
#define STARTING_FREQ_3        64.0  /*74.238716 */

#define NUM_CEP_COEFF            13  /* c1..c12 + c0 */

/*-------------------------------
 * Global Definitions and Macros
 *-------------------------------*/
#define	BOOLEAN int
#define	FALSE 0
#define	TRUE (!FALSE)
#define WAIT_A_KEY while( !getchar() );
#define PRINTMOD 15
#define IEEE_LE 0
#define IEEE_BE 1

/*----------------------------------------------
 * Global Variable Definitions and Declarations
 *----------------------------------------------*/
BOOLEAN QuietMode = FALSE,      /* Supress output to stderr */
  FsSpecified = FALSE,          /* Sampling frequency specified */
  SwapSpecified = FALSE,        /* Byte swap for raw data files specified */
  InputKindSpecified = FALSE,   /* Input file format specified */
  NoOutHeaderSpecified = FALSE, /* No output HTK header option specified */
  Noc0 = FALSE,                 /* No c0 coefficient to output feature vector */
  NologE = FALSE;               /* No logE component to output feature vector */ 

FILE *fp_in = NULL,             /* Input HTK, NIST or raw data file */
 *fp_out = NULL;                /* Output HTK file */

char InFilename[199],           /* Name of input file */
  OutFilename[199],             /* Name of output HTK file */
  InputKind[10] = "NIST";       /* Input file format */

int SamplingFrequency = 16000,  /* SamplingFrequency */
  NativeByteOrder = IEEE_LE,    /* Native byte ordering */
  InputByteOrder,               /* Default input byte ordering */
  OutParmKind=8262;             /* Output parameter kind (MFCC, MFCC_E, MFCC_0, */
                                /* or MFCC_0_E as default) */

/*----------------------------------------------------------------------------
 * FUNCTION NAME: ParseCommLine
 *
 * PURPOSE:       Parses command line arguments, opens input and output files
 *
 * INPUT:
 *   argc         Number of command line arguments
 *   argv         Array of command line arguments
 *
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   FALSE        In case of any errors
 *   TRUE         Otherwise
 *---------------------------------------------------------------------------*/
static BOOLEAN 
ParseCommLine (int argc, char *argv[])
{
    int mark = 0;

    while (argc)
    {
        if (strcmp (argv[mark], "-q") == 0)
        {
            QuietMode = TRUE;
            --argc;
            ++mark;
        }
        else if (strcmp (argv[mark], "-fs") == 0)
        {
            FsSpecified = TRUE;
            --argc;
            ++mark;
            SamplingFrequency = 1000 * atoi (argv[mark]);
            --argc;
            ++mark;
        }
        else if (strcmp (argv[mark], "-swap") == 0)
        {
            SwapSpecified = TRUE;
            --argc;
            ++mark;
        }
        else if (strcmp (argv[mark], "-F") == 0)
        {
            InputKindSpecified = TRUE;
            --argc;
            ++mark;
            strcpy (InputKind, argv[mark]);
            --argc;
            ++mark;
        }
        else if (strcmp (argv[mark], "-noh") == 0)
        {
            NoOutHeaderSpecified = TRUE;
            --argc;
            ++mark;
        }
        else if (strcmp (argv[mark], "-noc0") == 0)
        {
            Noc0 = TRUE;
            --argc;
            ++mark;
        }
        else if (strcmp (argv[mark], "-nologE") == 0)
        {
            NologE = TRUE;
            --argc;
            ++mark;
        }
        else if (argv[mark][0] == '-')
        {
            fprintf (stderr, "WARNING:  Un-recognized flag '%s' !\r\n", argv[mark]);
            --argc;
            ++mark;
        }
        else
        {
            if (fp_out)         /* Third argument string ERROR! */
            {
                fprintf (stderr, "    ERROR:   Too many input arguments!\r\n");
                return FALSE;
            }
            else if (fp_in)     /* Second argument string (output HTK file) */
            {
                strcpy (OutFilename, argv[mark]);
                fp_out = fopen (OutFilename, "wb");
                if (fp_out == NULL)
                {
                    fprintf (stderr, "    ERROR:   Could not open file '%s' !\r\n",
			    OutFilename);
                    return FALSE;
                }
            }

            else
                /* First argument string (input file) */
            {
                strcpy (InFilename, argv[mark]);
                fp_in = fopen (argv[mark], "rb");
                if (fp_in == NULL)
                {
                    fprintf (stderr, "    ERROR:   Could not open file '%s' !\r\n",
			    argv[mark]);
                    return FALSE;
                }
            }
            --argc;
            ++mark;
        }
    }

    if (!fp_in || !fp_out)      /* Input and output files must be given */
    {
        fprintf (stderr, "    ERROR:   Input and output files must be given!\r\n");
        return FALSE;
    }

    if (strcmp (InputKind, "NIST") &&
        strcmp (InputKind, "HTK") &&
        strcmp (InputKind, "RAW"))
    {
        fprintf (stderr, "    ERROR:   Invalid input file format '%s'!\r\n", InputKind);
        return FALSE;
    }

    if (strcmp (InputKind, "RAW") && FsSpecified)
        fprintf (stderr, "WARNING:   Sampling frequency needs to be specified only for raw data files.\r\n");

    if (strcmp (InputKind, "RAW") && SwapSpecified)
        fprintf (stderr, "WARNING:   Byte swapping needs to be specified only for raw data files if necessary.\r\n");

    if ( Noc0 && NologE ) TextToParmKind( "MFCC", &OutParmKind );
    else if ( Noc0 ) TextToParmKind( "MFCC_E", &OutParmKind );
    else if ( NologE ) TextToParmKind( "MFCC_0", &OutParmKind );

    return TRUE;
}

/*----------------------------------------------------------------------------
 * FUNCTION NAME: main
 *
 * PURPOSE:       Main front-end operations from input speech samples to output
 *                feature vectors. See embedded comments.
 *
 * INPUT:
 *   argc         Number of command line arguments (passed to ParseCommLine)
 *   argv         Array of command line arguments (passed to ParseCommLine)
 *
 * OUTPUT
 *   none
 *
 * RETURN VALUE
 *   TRUE         In case of any errors
 *   FALSE (0)    Otherwise
 *---------------------------------------------------------------------------*/
extern int 
main (int argc, char *argv[])
{
    int i, TmpInt, CFBSize, CFBPointer;
	int feature_number, g;

    long FrameLength, FrameShift, FFTLength, FrameCounter = 0;

    float LogEnergy, StartingFrequency, EnergyFloor_FB,
      EnergyFloor_logE, FloatBuffer[MAXWINDOWSIZE + 1],
      FloatWindow[MAXWINDOWSIZE / 2], *pDCTMatrix, *CircFloatBuffer;

    NIST_Header InNheader;
    HTK_Header InHheader, OutHheader;
    FFT_Window FirstWindow;

    fprintf (stderr,"\r\n    ETSI STQ WI007 DSR Front-End Feature Extraction Algorithm");
    fprintf (stderr,"\r\n    C-language software implementation");
    fprintf (stderr,"\r\n    Version 1.1.2   April 3, 2000\r\n\n");

    /*----------------*/
    /* Initialization */
    /*----------------*/
    EnergyFloor_FB = (float) exp ((double) ENERGYFLOOR_FB);
    EnergyFloor_logE = (float) exp ((double) ENERGYFLOOR_logE);
    InputByteOrder=NativeByteOrder;

    if (ParseCommLine (argc - 1, argv + 1))
    {
        /*-----------------------------------------------------------------*/
        /* Read input header, extract sampling frequency and byte ordering */
        /*-----------------------------------------------------------------*/
        if (!strcmp (InputKind, "NIST"))
        {
            if (!ReadNISTHeader (fp_in, &InNheader))
            {
                fprintf (stderr, "    ERROR:   Invalid NIST header !\r\n");
                goto _FaultExit;
            }
            SamplingFrequency = InNheader.SampleRate;
            if (strcmp (InNheader.SampleByteFormat, "10"))
                InputByteOrder = IEEE_LE;
            else
                InputByteOrder = IEEE_BE;
        }
        else if (!strcmp (InputKind, "HTK"))
        {
            if (!ReadHTKHeader (fp_in, &InHheader, InputByteOrder!=IEEE_BE ))
            {
                fprintf (stderr, "    ERROR:   Invalid HTK header !\r\n");
                goto _FaultExit;
            }
            /* 625->16kHz, 1250->8kHz, 909->11kHz */
            SamplingFrequency = 10 * floor ((float) 1e6 /
					    (float) InHheader.sampPeriod);
            InputByteOrder = IEEE_BE;
        }
        /*-------------------------------------------------------------------*/
        /* Set parameters FrameLength, FrameShift and FFTLength according to */
        /* the current sampling frequency                                    */
        /*-------------------------------------------------------------------*/
        if (SamplingFrequency == SAMPLING_FREQ_1 * 1000)
        {
            FrameLength = FRAME_LENGTH_1;
            FrameShift = FRAME_SHIFT_1;
            FFTLength = FFT_LENGTH_1;
            StartingFrequency = STARTING_FREQ_1;
        }
        else if (SamplingFrequency == SAMPLING_FREQ_2 * 1000)
        {
            FrameLength = FRAME_LENGTH_2;
            FrameShift = FRAME_SHIFT_2;
            FFTLength = FFT_LENGTH_2;
            StartingFrequency = STARTING_FREQ_2;
        }
        else if (SamplingFrequency == SAMPLING_FREQ_3 * 1000)
        {
            FrameLength = FRAME_LENGTH_3;
            FrameShift = FRAME_SHIFT_3;
            FFTLength = FFT_LENGTH_3;
            StartingFrequency = STARTING_FREQ_3;
        }
        else
        {
            fprintf (stderr, "    ERROR:   Invalid sampling frequency '%d'!\r\n",
		    SamplingFrequency);
            goto _FaultExit;
        }

        /*------------------------------------------------------------*/
        /* Write output header (number of frames to be updated later) */
        /*------------------------------------------------------------*/
        OutHheader.nSamples = 0;
        OutHheader.sampPeriod = 100000;  /* 10000.0 us (100 Hz) */
        OutHheader.sampSize = (NUM_CEP_COEFF - (Noc0 ? 1:0) + (NologE ? 0:1))*4;
        OutHheader.sampKind = OutParmKind;

        if ( !NoOutHeaderSpecified ) WriteHTKHeader (fp_out, &OutHheader);

	/*------------------------------------------------*/
	/* Memory allocation and initialization for input */
	/* circular float buffer                          */
	/*------------------------------------------------*/
	CFBSize=FrameLength+2;
	CircFloatBuffer=(float*)malloc(sizeof(float)*CFBSize);
	if ( !CircFloatBuffer )
	  {
	  fprintf (stderr, "    ERROR:   Memory allocation error occured!\r\n");
	  goto _FaultExit;
	  }
	CircFloatBuffer[0]=0.0;
	CircFloatBuffer[1]=0.0;
	CFBPointer=0;

	/*-------------------------------------------------------*/
	/* Initialization of FE data structures and input buffer */
	/*-------------------------------------------------------*/
	InitializeHamming (FloatWindow, (int) FrameLength);
	InitFFTWindows (&FirstWindow, StartingFrequency,
			(float) SamplingFrequency, FFTLength, NUM_CHANNELS);
	ComputeTriangle (&FirstWindow);
	pDCTMatrix = InitDCTMatrix (NUM_CEP_COEFF, NUM_CHANNELS);

	ReadWave(fp_in, CircFloatBuffer, CFBSize, CFBPointer+2, FrameLength-FrameShift,
		 (SwapSpecified || InputByteOrder != NativeByteOrder));

	DCOffsetFilter( CircFloatBuffer, CFBSize, &CFBPointer, FrameLength-FrameShift );

        /*----------------------------------------------------------------*/
        /*                       Framing                                  */
        /*----------------------------------------------------------------*/
        while (ReadWave (fp_in, CircFloatBuffer, CFBSize, (CFBPointer+2)%CFBSize, FrameShift,
                   (SwapSpecified || InputByteOrder != NativeByteOrder)))
        {
            FrameCounter++;

	    /*-------------------*/
	    /* DC offset removal */
	    /*-------------------*/
	    DCOffsetFilter( CircFloatBuffer, CFBSize, &CFBPointer, FrameShift );

	    /*------------------*/
	    /* logE computation */
	    /*------------------*/
            LogEnergy = 0.0;
            for (i = 0; i < FrameLength; i++)
                LogEnergy += CircFloatBuffer[(CFBPointer+i+3)%CFBSize] * CircFloatBuffer[(CFBPointer+i+3)%CFBSize];

            if (LogEnergy < EnergyFloor_logE)
                LogEnergy = ENERGYFLOOR_logE;
            else
                LogEnergy = (float) log ((double) LogEnergy);
	    
	    /*-----------------------------------------------------*/
	    /* Pre-emphasis, moving from circular to linear buffer */
	    /*-----------------------------------------------------*/
            for (i = 0; i < FrameLength; i++)
                FloatBuffer[i] = CircFloatBuffer[(CFBPointer+i+3)%CFBSize] -
		  PRE_EMPHASIS * CircFloatBuffer[(CFBPointer+i+2)%CFBSize];

            /*-----------*/
            /* Windowing */
            /*-----------*/
            Window (FloatBuffer, FloatWindow, (int) FrameLength);

            /*-----*/
            /* FFT */
            /*-----*/

            /* Zero padding */
            for (i = FrameLength; i < FFTLength; i++)
                FloatBuffer[i] = 0.0;

            /* Real valued, in-place split-radix FFT */
            TmpInt = (int) (log10 (FFTLength) / log10 (2));
            rfft (FloatBuffer, FFTLength, TmpInt); /*TmpInt = log2(FFTLength)*/

            /* Magnitude spectrum */
            FloatBuffer[0] = (float) fabs ((double) FloatBuffer[0]);  /* DC */
            for (i = 1; i < FFTLength / 2; i++)  /* pi/(N/2), 2pi/(N/2), ...,
						    (N/2-1)*pi/(N/2) */
                FloatBuffer[i] = (float) sqrt ((double) FloatBuffer[i] *
					       (double) FloatBuffer[i] +
                                               (double) FloatBuffer[FFTLength - i] *
					       (double) FloatBuffer[FFTLength - i]);
            FloatBuffer[FFTLength / 2] = (float) fabs ((double) FloatBuffer[FFTLength / 2]);  /* pi/2 */

            /*---------------*/
            /* Mel filtering */
            /*---------------*/
            MelFilterBank (FloatBuffer, &FirstWindow);

            /*-------------------------------*/
            /* Natural logarithm computation */
            /*-------------------------------*/
            for (i = 0; i < NUM_CHANNELS; i++)
                if (FloatBuffer[i] < EnergyFloor_FB)
                    FloatBuffer[i] = ENERGYFLOOR_FB;
                else
                    FloatBuffer[i] = (float) log ((double) FloatBuffer[i]);

            /*---------------------------*/
            /* Discrete Cosine Transform */
            /*---------------------------*/
            DCT (FloatBuffer, pDCTMatrix, NUM_CEP_COEFF, NUM_CHANNELS);

            /*--------------------------------------*/
            /* Append logE after c0 or overwrite c0 */
            /*--------------------------------------*/
            FloatBuffer[NUM_CHANNELS + NUM_CEP_COEFF - (Noc0 ? 1:0)] = LogEnergy;

            /*---------------*/
            /* Output result */
            /*---------------*/
            //WriteHTKFeature (fp_out, &FloatBuffer[NUM_CHANNELS], (short) (NUM_CEP_COEFF - (Noc0 ? 1:0) + (NologE ? 0:1)) );
			feature_number = (int) (NUM_CEP_COEFF - (Noc0 ? 1:0) + (NologE ? 0:1));
			for (g = 0; g < feature_number; ++g) {
				printf("%f", FloatBuffer[NUM_CHANNELS + g]);
				if (g < feature_number - 1) {
					printf("\t");
				}
			}
			printf("\n");
			

            /*---------------------------*/
            /* Display processing status */
            /*---------------------------*/
            if (!QuietMode && !(FrameCounter % PRINTMOD))
            {
                fprintf (stderr, "\rProcessing status: %ld frames ...", FrameCounter);
                fflush (stderr);
            }

        }

	/*----------------*/
	/* Memory release */
	/*----------------*/
	free(CircFloatBuffer);
	free(pDCTMatrix);
	ReleaseFFTWindows(&FirstWindow);

        /*----------------------------------------*/
        /* Correct number of frames in HTK header */
        /*----------------------------------------*/
        OutHheader.nSamples = FrameCounter;
        if ( !NoOutHeaderSpecified ) WriteHTKHeader (fp_out, &OutHheader);

        /*------------------------------*/
        /* Close input and output files */
        /*------------------------------*/
        fclose (fp_in);
        fclose (fp_out);

        /*----------------------*/
        /* Display final status */
        /*----------------------*/
        if (!QuietMode)
            fprintf (stderr, "\rProcessed: %ld Frames.                      \r\n",
		    FrameCounter);

    }                           /* if ( ParsCommLine... ) */
    else
    {
        fprintf (stderr, "\r\n    USAGE:");
        fprintf (stderr, "   %s infile HTK_outfile [options]\r\n", argv[0]);
        fprintf (stderr, "\r\n    OPTIONS:\r\n");
        fprintf (stderr, "     -q            Quiet Mode                                 (%s)\r\n", QuietMode ? "TRUE" : "FALSE");
        fprintf (stderr, "     -F    format  Input file format (NIST,HTK,RAW)           (%s)\r\n", InputKind);
        fprintf (stderr, "     -fs   freq    Sampling frequency in kHz (%d,%d,%d)        (%d)\r\n", SAMPLING_FREQ_1, SAMPLING_FREQ_2, SAMPLING_FREQ_3, SamplingFrequency / 1000);
        fprintf (stderr, "     -swap         Change input byte ordering                 (%s)\r\n", SwapSpecified ? "Swapped" : "Native");
        fprintf (stderr, "                   (Native byte ordering is %s)\r\n", NativeByteOrder ? "ieee-be" : "ieee-le");
	fprintf (stderr, "     -noh          No HTK header to output file               (%s)\r\n", NoOutHeaderSpecified ? "TRUE" : "FALSE" );
	fprintf (stderr, "     -noc0         No c0 coefficient to output feature vector (%s)\r\n", Noc0 ? "TRUE" : "FALSE" );
	fprintf (stderr, "     -nologE       No logE component to output feature vector (%s)\r\n", NologE ? "TRUE" : "FALSE" );

    }

    return FALSE;

  _FaultExit:
    if (fp_in)
        fclose (fp_in);
    if (fp_out)
        if (fclose (fp_out))
            fprintf (stderr, "Can't rewrite output file!\r\n");
    return TRUE;
}
