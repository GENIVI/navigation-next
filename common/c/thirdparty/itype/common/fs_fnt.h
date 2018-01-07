
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* Copyright: 1987-1990 by Apple Computer, Inc., all rights reserved. */

/* fs_fnt.h */

#ifndef FS_FNT_H
#define FS_FNT_H

#ifdef FS_EDGE_HINTS
#include "../adf/adfalgnzonesmaz.h"
#endif

#define fnt_pixelSize  64    /* in F26DOT6, this is ONE */
#define fnt_pixelShift 6

#define MAXBYTE_INSTRUCTIONS 256

#define ONEVECTOR                    SHORTFRACT_ONE
#define ONESIXTEENTHVECTOR            ((ONEVECTOR) >> 4)
#define VECTORMUL(a,b)                VarMul(a,(FS_LONG)b,14)

/* a and b are ALWAYS 2.14 short fracts, so product will fit in 32 bits */
#define VECTORDOT(a,b)                (FS_SHORT)(((FS_LONG)a * b + (1<<13)) >> 14)

#define VECTORMULDIV(a,b,c)            LongMulDiv((FS_LONG)a,(FS_LONG)b,(FS_LONG)c)


/* the interpreters idea of a glyph */
typedef struct {
    F26DOT6     *x;        /* The Points the Interpreter modifies */
    F26DOT6     *y;        /* The Points the Interpreter modifies */
    F26DOT6     *ox;       /* Old Points */
    F26DOT6     *oy;       /* Old Points */
    F26DOT6     *oox;      /* Old Unscaled Points, really ints */
    F26DOT6     *ooy;      /* Old Unscaled Points, really ints */
    FS_BYTE     *onCurve;  /* indicates if a point is on or off the curve */
    FS_USHORT   *sp;       /* Start points */
    FS_USHORT   *ep;       /* End points */
    FS_BYTE     *f;        /* Internal flags, one byte for every point */
    FS_SHORT    nc;        /* Number of contours */
} fnt_ElementType;

/* declare to remove compiler warnings in the following prototypes */
struct fnt_LocalGraphicStateType;
typedef FS_VOID (*FntFunc)(struct fnt_LocalGraphicStateType*);
typedef FS_VOID (*FntMoveFunc)(struct fnt_LocalGraphicStateType*, fnt_ElementType*, FS_LONG, F26DOT6);
/* typedef F26DOT6 (*FntRoundFunc)(F26DOT6 xin, F26DOT6 engine, struct fnt_LocalGraphicStateType* gs); */
enum RoundFuncIndex {
    RFI_fnt_SuperRound,
    RFI_fnt_Super45Round,
    RFI_fnt_RoundToGrid,
    RFI_fnt_RoundToHalfGrid,
    RFI_fnt_RoundToDoubleGrid,
    RFI_fnt_RoundOff,
    RFI_fnt_RoundDownToGrid,
    RFI_fnt_RoundUpToGrid
};
typedef enum RoundFuncIndex FntRoundFunc;
F26DOT6 RoundFuncCall(FntRoundFunc f, F26DOT6 xin, F26DOT6 engine, struct fnt_LocalGraphicStateType* gs);
typedef F26DOT6 (*FntProjFunc)(struct fnt_LocalGraphicStateType*, F26DOT6, F26DOT6);


typedef struct {
    /* PARAMETERS CHANGEABLE BY TT INSTRUCTIONS */
    F26DOT6         wTCI;                   /* width table cut in */
    F26DOT6         sWCI;                   /* single width cut in */
    F26DOT6         scaledSW;               /* scaled single width */
    FS_LONG         scanControl;            /* controls kind and when of dropout control */
    FS_LONG         instructControl;        /* controls gridfitting and default setting */
    F26DOT6         minimumDistance;        /* moved from local gs  7/1/90  */
    FntRoundFunc    RoundValue;             /*                                */
    F26DOT6         periodMask;             /* ~(gs->period-1)                 */
    FRACT           period45;               /*                                */
    FS_SHORT        period;                 /* for power of 2 periods         */
    FS_SHORT        phase;                  /*                                */
    FS_SHORT        threshold;              /* moved from local gs  7/1/90  */
    FS_SHORT        deltaBase;
    FS_SHORT        deltaShift;
    FS_SHORT        angleWeight;
    FS_SHORT        sW;                     /* single width, expressed in the same units as the character */
    FS_BYTE         autoFlip;               /* The auto flip Boolean */
    FS_BYTE         pad;    
} fnt_ParameterBlock;

#define ROTATEDGLYPH    0x100
#define STRETCHEDGLYPH  0x200
#define NOGRIDFITFLAG    1
#define DEFAULTFLAG      2

typedef enum {
    PREPROGRAM,
    FONTPROGRAM,
    MAXPREPROGRAMS
} fnt_ProgramIndex;

typedef struct {
    FS_FIXED N;
    FS_FIXED D;
    FS_FIXED scl;
    int shift;
    } scale_rec;

/* typedef F26DOT6 (*ScaleFunc)(scale_rec*, F26DOT6); */
enum ScaleFuncIndex {
    SFI_fnt_FRound,
    SFI_fnt_SRound,
    SFI_fnt_FixRound
};
typedef enum ScaleFuncIndex ScaleFunc;
F26DOT6 ScaleFuncCall(ScaleFunc f, scale_rec*r, F26DOT6 v);

#ifdef FS_HINTS
#ifdef FS_EDGE_HINTS
typedef struct
{
    MAZOutlineContour *contours;
    MAZOutlinePoint   *points;
    MAZOutlineSegment *hSegs;
    MAZOutlineSegment *vSegs;
    MAZOutlineContour **contourList;
    MAZOutlineSegPair *segPairs;
    ADF_U32 gridFitType;
    ADF_U32 noCenter;
    ADF_I32 numHhints;
    ADF_I32 Hcollapse[10];
    ADF_I32 numVhints;
    ADF_I32 Vcollapse[10];
    ADF_I32 numXdeltas;
    ADF_I32 Xdeltashift[10];
    ADF_I32 Xdeltapoint[10];
    ADF_I32 numYdeltas;
    ADF_I32 Ydeltashift[10];
    ADF_I32 Ydeltapoint[10];
    ADF_I32 MAZmethodHorz;
    ADF_I32 MAZmethodVert;
    ADF_I32 MAZdeltaHorz;
    ADF_I32 MAZdeltaVert;
    ADF_I32 foundacollisionhorz;
    ADF_I32 foundacollisionvert;
    ADF_I32 cvtstart;
    ADF_I32 numylines;
    ADF_I32 isrighttoleft;
    ADF_I32 iscomposite;
    ADF_I32 isitalic;
    ADF_I32 maxWidth;
} MAZ_DATA;
#endif /* FS_EDGE_HINTS */

typedef struct
{
    FS_LONG   *RTGAHPieces;
    FS_LONG   *RTGAHStrokes;
    FS_ULONG   fontflags;
    FS_ULONG   stateflags;
    FS_LONG   *RTGAHStrokesPlist;
    FS_LONG   *RTGAHEdges;
    FS_LONG   *RTGAHXmin;
    FS_LONG   *RTGAHYmin;
    FS_LONG   *RTGAHXmax;
    FS_LONG   *RTGAHYmax;
    int        script;
    FS_ULONG   unicode;
    F26DOT6    hi,lo;
    FS_USHORT  rtgah_suitable;
    FS_USHORT  fnt_type;
} RTGAH_DATA;
#endif /* FS_HINTS */

#ifdef FS_CFFR
typedef struct {
    int numcontours;
    int numtypes;
    int numpoints;
    int args[48];
    int numargs;
    int prevx;
    int prevy;
    int hasadvwidth;
    int loopstartx;
    int loopstarty;
    int numhstems;
    int numvstems;
#if 0 /* future use */
    int *hstems;
    int *vstems;
    int stems[96];
    int numbluevalues;
    F26DOT6 scaledbluevalues[14];
    int numotherblues;
    F26DOT6 scaledotherblues[10];
    FS_FIXED bluescale;
    int scaledblueshift;
    int scaledbluefuzz;
    F26DOT6 scaledstdHW;
    F26DOT6 scaledstdVW;
    int numsnapstemH;
    F26DOT6 scaledsnapstemH[12];
    int numsnapstemV;
    F26DOT6 scaledsnapstemV[12];
#endif
    int dx;
    FS_LONG transient[32];
} CFF_DATA;
#endif
/****************************************************************/
typedef struct fnt_GlobalGraphicStateType {
    TTF_MAXP           *maxp;                       /* pointer to fonts "maxp" table */
    F26DOT6            *stackBase;                  /* the stack area */
    F26DOT6            *store;                      /* the storage area */
    F26DOT6            *controlValueTable;          /* the control value table */
    FS_USHORT           cvtCount;
    FS_USHORT           pixelsPerEm;                /* number of pixels per em as an integer */
    FS_USHORT           pointSize;                  /* the requested point size as an integer */
    FS_FIXED            fpem;                       /* FRACTional pixels per em    <3> */
    F26DOT6             engine[4];                  /* Engine Characteristics */
    
    /* variables settle by instructions and the defaults */
    fnt_ParameterBlock  defaultParBlock;
    fnt_ParameterBlock  localParBlock;

    /* variables NOT settable by instructions */
    FntFunc            *function;                  /* pointer to instruction definition area */
    fnt_funcDef        *funcDef;                   /* function Definitions identifiers */
    fnt_instrDef       *instrDef;                  /* instruction Definitions identifiers */

    FS_FIXED            metricScalarX;
    FS_FIXED            interpScalarX;
    scale_rec           scaleX;
    ScaleFunc           scaleFuncX;

    FS_FIXED            metricScalarY;
    FS_FIXED            interpScalarY;
    scale_rec           scaleY;
    ScaleFunc           scaleFuncY;

    FS_FIXED            cvtStretchX;
    FS_FIXED            cvtStretchY;
    scale_rec           scaleCVT;
    ScaleFunc           scaleFuncCVT;


    FS_BYTE             identityTransformation;      /* 1/0  (does not mean identity from a global sense) */
    FS_BYTE             non90DegreeTransformation;   /* bit 0 is 1 if non-90 degree, bit 1 is 1 if x scale doesn't equal y scale */
    FS_FIXED            xStretch;                    /* Tweaking for glyphs under transformational stress <4> */
    FS_FIXED            yStretch;
    
    FS_BYTE             init;                        /* executing preprogram ?? */
    FS_BYTE             pgmIndex;                    /* which preprogram is current */
    FS_LONG             instrDefCount;               /* number of currently defined IDefs */
    FS_USHORT           glyphProgram;
    FS_BYTE            *pgmList[MAXPREPROGRAMS];     /* each program ptr is in here */
#if defined(FS_STIK) || defined(FS_HINTS)            /* holds bold pct and stroke pct for stroke autohinters */
    SFNT               *sfnt;                        /* holds ref_lines for RTGAH */
#endif
#ifdef FS_HINTS
#ifdef FS_EDGE_HINTS
    MAZ_DATA            maz_data;
#endif
    RTGAH_DATA          rtgah_data;
#endif /* FS_HINTS */
#ifdef  FS_CFFR
    CFF_DATA cff_data;
#endif
    
} fnt_GlobalGraphicStateType;


typedef   F26DOT6    (*GlobalGSScaleFunc)(struct fnt_GlobalGraphicStateType*, F26DOT6);

/****************************************************************/
typedef struct fnt_LocalGraphicStateType {
    fnt_ElementType             *CE0, *CE1, *CE2;     /* The character element pointers */
    SHORT_UNIT_VECTOR            proj;                /* Projection Vector */
    SHORT_UNIT_VECTOR            freedom;             /* Freedom Vector */
    SHORT_UNIT_VECTOR            oldProj;             /* Old Projection Vector */
    F26DOT6                     *stackPointer;
    FS_BYTE                     *insPtr;              /* Pointer to the instruction we are about to execute */
    fnt_ElementType             *elements;            /* the coords we're instructing */
    fnt_GlobalGraphicStateType  *globalGS;
    FS_LONG                      Pt0, Pt1, Pt2;       /* The internal reference points */
    FS_LONG                      loop;                /* The loop variable */    
    FS_SHORT                     roundToGrid;            
    FS_BYTE                      opCode;              /* The instruction we are executing */
    /* Above is exported to client in FontScaler.h */

    SHORTFRACT                    pfProj;             /* = pvx * fvx + pvy * fvy  ... ABS(pfProj)<=2 as needed*/
    FntMoveFunc                   MovePoint;
    FntProjFunc                   Project;
    FntProjFunc                   OldProject;
    F26DOT6 (*GetCVTEntry) (struct fnt_LocalGraphicStateType *gs, FS_LONG n);
    F26DOT6 (*GetSingleWidth) (struct fnt_LocalGraphicStateType *gs);

#ifdef KILL_SETJMP_LONGJMP
    FS_LONG                        error;
#else
    jmp_buf                        env;               /* always be at the end, since it is unknown size */
#endif /* KILL_SETJMP_LONGJMP */
#ifdef FS_MULTI_PROCESS
    FS_BYTE *server; /* pointer to the guts of iType used for offsets */
#endif

} fnt_LocalGraphicStateType;

/*
 * Executes the font instructions.
 * This is the external interface to the interpreter.
 *
 * Parameter Description
 *
 * elements points to the character elements. Element 0 is always
 * reserved and not used by the actual character.
 *
 * ptr points at the first instruction.
 * eptr points to right after the last instruction
 *
 * globalGS points at the global graphics state
 *
 * Note: The stuff globalGS is pointing at must remain intact
 *       between calls to this function.
 */

extern FS_LONG fnt_Execute(fnt_ElementType *elements, FS_BYTE *ptr, FS_BYTE *eptr, 
            fnt_GlobalGraphicStateType *globalGS, FS_VOID *serv);

/* Export internal rounding routines so globalGraphicsState->defaultParBlock.RoundValue
 * can be set in fsglue.c
 */
extern F26DOT6 fnt_RoundToDoubleGrid(F26DOT6 xin, F26DOT6 engine, fnt_LocalGraphicStateType *gs);  
extern F26DOT6 fnt_RoundDownToGrid(F26DOT6 xin, F26DOT6 engine, fnt_LocalGraphicStateType *gs);  
extern F26DOT6 fnt_RoundUpToGrid(F26DOT6 xin, F26DOT6 engine, fnt_LocalGraphicStateType *gs);  
extern F26DOT6 fnt_RoundToGrid(F26DOT6 xin, F26DOT6 engine, fnt_LocalGraphicStateType *gs);  
extern F26DOT6 fnt_RoundToHalfGrid(F26DOT6 xin, F26DOT6 engine, fnt_LocalGraphicStateType *gs); 
extern F26DOT6 fnt_RoundOff(F26DOT6 xin, F26DOT6 engine, fnt_LocalGraphicStateType *gs);  
extern F26DOT6 fnt_SuperRound(F26DOT6 xin, F26DOT6 engine, fnt_LocalGraphicStateType *gs);  
extern F26DOT6 fnt_Super45Round(F26DOT6 xin, F26DOT6 engine, fnt_LocalGraphicStateType *gs); 

FS_VOID BAZOutlineSmooth(fnt_LocalGraphicStateType *gs);

#endif  /* FS_FNT_H */
