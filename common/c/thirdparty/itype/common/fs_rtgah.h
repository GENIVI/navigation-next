
/* Copyright (C) 2009 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_rtgah.h */

#ifndef FS_RTGAH_H
#define FS_RTGAH_H

#ifdef __cplusplus
extern "C" {
#endif




/* be careful changing these... */
#define CJK_TROUGH 32
#define CAP_REF_LINE_TOL 32
#define LC_REF_LINE_TOL 48
#define BASE_REF_LINE_TOL 48
#define MAX_PIECE_SLOP 3
#define ANG_TOL 22
    /* if (ang_tol*dy < dx) angle is small */
    /* atan(1/14) =  4.086 degrees */
    /* atan(1/13) =  4.399 degrees */
    /* atan(1/12) =  4.764 degrees */
    /* atan(1/11) =  5.194 degrees */
    /* atan(1/10) =  5.702 degrees */
    /* atan(1/9)  =  6.342 degrees */
    /* atan(1/8)  =  7.125 degrees */
    /* atan(1/7)  =  8.130 degrees */
    /* atan(1/6)  =  9.462 degrees */
    /* atan(1/5)  = 11.310 degrees */

#define LEN_TOL 16  /* 1/4 pixel */

typedef struct _PLIST {
    FS_USHORT pt;
    struct _PLIST *next;
    } PLIST;

typedef struct {
    FS_USHORT p0;
    FS_USHORT p1;
    FS_USHORT c;        /* useable only before "combine pieces" */
    FS_SHORT d;    
    PLIST *pts;
    } PIECE;

typedef struct {
    FS_USHORT p0,p1,p2,p3;
    PLIST *e1;  /* other points belonging to first edge */
    PLIST *e2;  /* other points belonging to second edge */

    F26DOT6 raw_center;
    F26DOT6 adj_center;
    F26DOT6 width;
    F26DOT6 min,max; /* wrt 'other' coord */
    } STROKE;

typedef struct {
FS_USHORT pt;
FS_USHORT unused;
F26DOT6 x;
F26DOT6 y0;
F26DOT6 y1;
    } EDGE;     /* 16 bytes */ 

typedef struct {
    FS_USHORT c;
    FS_USHORT xmin;
    FS_USHORT xmax;
    FS_USHORT ymin;
    FS_USHORT ymax;
    F26DOT6 oxc;
    F26DOT6 oyc;
    F26DOT6 xc;
    F26DOT6 yc;
    } BOOJUM;   /* for multi_dot() */


#define MAX_PIECES 128
#define MAX_STROKES 64
#define MAX_CONTOURS 32

/* !!! these two must agree with the defines in <fnt.c> */
#define XMOVED 0x01
#define YMOVED 0x02

#define ONE_26_6  64
#define TWO_26_6  128
#define FLOOR_26_6(x)   ((x) & ~63)             /* 26.6 result */
#define CEILING_26_6(x) (((x)+63)& ~63)         /* 26.6 result */
#define ROUND_26_6(x)   FLOOR_26_6(32+(x))      /* 26.6 result */

#define NEXT(x) (((x)==ep[c]) ? sp[c] : (x)+1)
#define PREV(x) (((x)==sp[c]) ? ep[c] : (x)-1)

#ifdef __cplusplus
    }
#endif

#endif /* FS_RTGAH_H */
