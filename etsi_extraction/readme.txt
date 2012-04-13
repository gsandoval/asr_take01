/******************************************************************************
*
*      ETSI STQ WI007 DSR Front-End Feature Extraction Algorithm
*      C-language software implementation 
*      Version 1.1.3   May, 2003
*
******************************************************************************/

Front-end feature extraction algorithm for Distributed Speech Recognition

General description of the C-language software implementation and examples


These files contain a floating-point C-language implementation of the 
mel-cepstrum based front-end feature extraction algorithm for 
Distributed Speech Recognition (DSR). The implementation is based on 
the ETSI standard document "ETSI ES 201 108", Version 1.1.3.


LIST OF FILES
=============
The source code consists of five files:
 - fileio.[ch]  (general file I/O functions)
 - FEfunc.[ch]  (basic front-end functions)
 - FrontEnd.c   (main front-end function)

In addition two make files are included for UNIX and PC platforms, 
respectively.
 - Makefile     (UNIX makefile)
 - Makefile.gcc (PC makefile)

Example waveform and feature files for installation checking:
(1) Example waveforms at different sampling frequencies, using RAW format with
    UNIX (big endian) byte ordering
 - instchk_16kHz.raw
 - instchk_11kHz.raw
 - instchk_8kHz.raw

(2) The corresponding feature files computed on UNIX platform 
 - instchk_16kHz.fea
 - instchk_11kHz.fea
 - instchk_8kHz.fea

INSTALLING THE SOFTWARE
=======================
This ANSI C implementation can easily be transfered to different platforms.

On a UNIX platform all source files have to be copied to an arbitrary 
directory. The make file can then be updated accordingly, using e.g.
the unix mkmf command (mkmf -f Makefile).

On a PC platform the included makefile can be used, or a different one
can be created by the utilized compiler.

NOTE Since different platforms use different file formats (different byte
orderings), the actual byte ordering has to be specified in file FrontEnd.c
before compilation. The constant NativeByteOrdering has to be set
either to IEEE_BE (for most UNIX machines) or IEEE_LE (for PCs).

The program can then be compiled and linked.

RUNNING THE SOFTWARE
====================
After successful installation the program may be invoked without any command 
line arguments for help:

    ETSI STQ WI007 DSR Front-End Feature Extraction Algorithm
    C-language software implementation 
    Version 1.1.2   April 3, 2000

    ERROR:   Input and output files must be given!

    USAGE:   ./FrontEnd infile HTK_outfile [options]

    OPTIONS:
     -q            Quiet Mode                                 (FALSE)
     -F    format  Input file format (NIST,HTK,RAW)           (NIST)
     -fs   freq    Sampling frequency in kHz (8,11,16)        (16)
     -swap         Change input byte ordering                 (Native)
                   (Native byte ordering is ieee-be)
     -noh          No HTK header to output file               (FALSE)
     -noc0         No c0 coefficient to output feature vector (FALSE)
     -nologE       No logE component to output feature vector (FALSE)

Using the -q option one can suppress the processing status produced by
the program. The input waveform file format is specified by the -F option. 
Supported file formats are NIST, HTK, and RAW (see File Formats section
for details). In case of RAW data files when there is no header before
the data samples the user has to specify the data format, that is sampling 
frequency and byte ordering of the waveform file. Sampling frequency can
be given by using the -fs option. Supported sampling frequencies are 8, 11, 
and 16kHz. 

Once the native byte ordering has been set before compilation (see section 
Installing the Software), the program assumes the input byte ordering for 
RAW data files to be the same as the native byte ordering. 
NOTE The native byte ordering is always visible from command line help. 
If the user wises to use different byte ordering, it can be done by giving 
command line option -swap. If no HTK header is needed in the output
file -noh option has to be used.

Either, or even both of the energy measures (i.e. c0 and logE) can be
suppressed in the output feature vectors. Command line options -noc0 and 
-nologE serve for this purpose. The following table summarizes the effects
of these flags on the output feature vector size, content, and HTK
parameter kind stored in the header.

 -noc0     -nologE   Output feature vector size and format  HTK parameter kind
 -----------------------------------------------------------------------------
 not used  not used  14  (c1,c2,...,c12,c0,logE)            MFCC_0_E
 used      not used  13  (c1,c2,...,c12,logE)               MFCC_E
 not used  used      13  (c1,c2,...,c12,c0)                 MFCC_0
 used      used      12  (c1,c2,...,c12)                    MFCC

FILE FORMATS
============
Three waveform formats are supported. The output feature format is always HTK.

(1) The NIST (or SPHERE) format consists of a 1024 byte long ASCII header
    followed by the data samples. The ASCII header contains information about
    sample size, number of samples, byte ordering, sampling frequency, etc. 
    In NIST format both IEEE_LE and BE byte ordering is possible (and used
    in different commercial databases).

(2) The HTK file format contains a 12-byte long header before the data
    samples. This header tells nothing about byte ordering, and
    therefore UNIX (IEEE_BE) byte ordering is assumed for waveform
    samples. The output feature vectors are also stored in HTK file
    format, but the output byte ordering (of 4-byte floats) always follows
    the native byte ordering of the machine.

(3) The RAW data format contains no header. Sampling frequency and byte 
    ordering have to be specified from command line, otherwise the default
    values (16kHz, Native) are used.

EXAMPLES
========
Since the exact floating point values of feature vector components depend on 
the actual platform and compiler, the example feature files can only be used
for approximate comparisons. In practice a maximum absolute difference
of 6e-4..9e-4 occurs between feature files (all feature vectors, all 
components) produced by UNIX and DOS version of the front-end. Example
waveform files are included for all sampling frequencies (RAW format,
instchk_XXkHz.raw). The corresponding feature files (instchk_XXkHz.fea) were
computed on HP-UNIX platform (processor type 9000/715).

NOTE For raw data files the sampling frequency and byte ordering must be 
specified from command line. Also, the input format must be explicitly
given in case of HTK or RAW formats (-F HTK or RAW).
