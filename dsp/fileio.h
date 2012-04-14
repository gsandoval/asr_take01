/******************************************************************************
*
*      ETSI STQ WI007 DSR Front-End Feature Extraction Algorithm
*      C-language software implementation 
*      Version 1.1.3   May, 2003
*
******************************************************************************/
/*----------------------------------------------------------------------------
 *
 * FILE NAME: fileio.h
 * PURPOSE:   General purpose function package for DSR file I/O operations.
 *            Various input file formats (RAW, NIST, HTK) are supported.
 *            Output file format is always HTK. Package consists of functions
 *            for reading different file headers and input data and writing
 *            output HTK header and output data.
 *
 *---------------------------------------------------------------------------*/

#ifndef _FILEIO_H
#define _FILEIO_H

/*-----------------
 * Data Structures 
 *-----------------*/
typedef struct
{
    long nSamples;              /* Structure for HTK header */
    long sampPeriod;
    short sampSize;
    short sampKind;
}
HTK_Header;

typedef struct
{
    char Type[30];              /* Structure for NIST header */
    short HeaderSize;
    char DatabaseID[10];
    char DatabaseVersion[10];
    char UtteranceID[30];
    short ChannelCount;
    long SampleCount;
    long SampleRate;
    short SampleMin;
    short SampleMax;
    short SamplenBytes;
    char SampleByteFormat[10];
    short SampleSigBits;
    char SampleCoding[10];
    long SampleChecksum;
}
NIST_Header;

/*---------------------
 * Function Prototypes
 *---------------------*/
int ReadNISTHeader (FILE * fp_in, NIST_Header * header);
int ReadHTKHeader (FILE * fp_in, HTK_Header * header, int Swap );
void WriteHTKHeader (FILE * fp_out, HTK_Header * header);
void WriteHTKFeature (FILE * fp_out, float *out, short fea_len);
int ReadWave(FILE *fp_in, float *CircBuff, int BSize, int BPointer, int nSamples, int Swap );
int TextToParmKind( char *instr, int *parmKind );

#endif
