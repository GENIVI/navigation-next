
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_pfr.h */

#ifndef FS_PFR_H
#define FS_PFR_H

#include "fs_port.h"
#include "fs_bitio.h"


/********************* IMPORTANT: Please Note:    ********************** 
 iType can be used to render outline fonts encoded in PFR format, provided
 that such rendering is to be used to achieve compliance with digital TV 
 standards developed by DAVIC (ISO/IEC 16500), DVB and/or Digital TV Group.
********************* IMPORTANT: Please Note:    **********************/ 

/* byte swapping macros  */
#define GETSWAP16(x) (FS_SHORT)((((FS_ULONG)(*(x)))<<8)+((FS_ULONG)(*(x+1))))
#define GETSWAP24(x) ((((FS_ULONG)(*x)<<16))+(((FS_ULONG)(*(x+1)))<<8)+((FS_ULONG)(*(x+2))))
  
  
typedef struct {
  FS_USHORT charCode;
  FS_USHORT charSetWidth;
  FS_BYTE asciiCodeValue;
  FS_USHORT gpsSize;
  FS_ULONG gpsOffset;
} PFR_CHARRECORD;

typedef struct {
  FS_USHORT charCode;
  FS_USHORT gpsSize;
  FS_ULONG gpsOffset;
} PFR_BMAPCHARRECORD;

typedef struct {
  FS_USHORT xppm;
  FS_USHORT yppm;
  FS_BYTE flags;
  FS_ULONG bctSize;
  FS_ULONG bctOffset;
  FS_USHORT nBmapChars;
} PFR_BMAPSIZERECORD;

typedef struct {
  FS_FIXED s00;
  FS_FIXED s01;
  FS_FIXED s10;
  FS_FIXED s11;
  FS_FIXED f00;
  FS_FIXED f01;
  FS_FIXED f10;
  FS_FIXED f11;
  FS_FIXED *blueValues;
  FS_FIXED blueFuzz;
  FS_FIXED blueScale;
  FS_FIXED stdVW;
  FS_FIXED stdHW;
  FS_FIXED *hSnapValues;
  FS_FIXED *vSnapValues;
} PFR_SCALEINFO;


typedef struct _pfr {
  FILECHAR *path;                  /* if a disk file .. the path */
  FS_BYTE  *memptr;                /* else ROM/RAM location */
  FS_FILE  *fp;                    /* fp = FS_open(path) for disk file */
  FILECHAR *name;
  FS_SHORT nCharacters;
  PFR_CHARRECORD *characters;
  FS_USHORT fontRefNumber;
  FS_USHORT outlineResolution;
  FS_USHORT metricsResolution;
  FS_SHORT xMin;
  FS_SHORT yMin;
  FS_SHORT xMax;
  FS_SHORT yMax;
  FS_BYTE flags;
  FS_USHORT standardSetWidth;
  FS_SHORT nBlueValues;
  FS_SHORT *blueValues;
  FS_BYTE blueFuzz;
  FS_BYTE blueScale;
  FS_SHORT stdVW;
  FS_SHORT stdHW;
  FS_SHORT nHSnapValues;
  FS_SHORT nVSnapValues;
  FS_SHORT *hSnapValues;
  FS_SHORT *vSnapValues;
  FS_USHORT gpsMaxSize;
  FS_ULONG gpsSectionSize;
  FS_ULONG gpsSectionOffset;
  FS_SHORT nPhysFonts;
  FS_BYTE maxBlueValues;
  FS_BYTE maxXorus;
  FS_BYTE maxYorus;
  FS_BYTE pfntInvertBitmap;
  FS_BYTE pfrBlackPixel;
  FS_BYTE maxStemSnapVsize;
  FS_BYTE maxStemSnapHsize;
  FS_USHORT maxChars;
  FS_SHORT ascent;
  FS_SHORT descent;
  FS_SHORT externalleading;
} PFR;

typedef struct {
  FS_SHORT numcontours;
  FS_SHORT numelements;
  FS_SHORT numpoints;
  FS_SHORT nXorus;
  FS_SHORT nYorus;
  FS_SHORT nXSstrokes;
  FS_SHORT nYSstrokes;
  FS_SHORT nXSedges;
  FS_SHORT nYSedges;
} PFR_GETSIZEINFO;

typedef struct {
  FS_BYTE *etype;
  FS_FIXED *x;
  FS_FIXED *y;
  FS_FIXED *Xorus;
  FS_FIXED *Yorus;
  FS_FIXED *XSstroke;      /* vertical strokes */
  FS_FIXED *YSstroke;      /* horizontal strokes */
  FS_FIXED *XSedge;        /* vertical edges */
  FS_FIXED *YSedge;        /* horizontal edges    */
  FS_FIXED *topXorus;
  FS_FIXED *topYorus;
  FS_FIXED *topXSstroke;    /* vertical strokes    */
  FS_FIXED *topYSstroke;    /* horizontal strokes */
  FS_FIXED *topXSedge;      /* vertical edges */
  FS_FIXED *topYSedge;      /* horizontal edges    */
  FS_BYTE *processed;
  FS_FIXED *newXorus;
  FS_FIXED *newXSstroke;
  FS_FIXED *newYorus;
  FS_FIXED *newYSstroke;
  FS_FIXED *newv;
  FS_FIXED *oldv;
} PFR_POINTERINFO;


#endif /* FS_PFR_H */
