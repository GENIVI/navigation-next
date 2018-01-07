/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* =========================================================================

FILE: AEEMediaUtil.c

SERVICES: IMedia utility functions

DESCRIPTION
  This file contains IMedia related utility functions that apps can use
  to develop BREW multimedia applications.

PUBLIC CLASSES:  
   N/A

INITIALIZATION AND SEQUENCING REQUIREMENTS:  N/A


           Copyright ?2000-2002 QUALCOMM Incorporated.
                     All Rights Reserved.
                   QUALCOMM Proprietary/GTDR
  ========================================================================= */


/* =========================================================================
                     INCLUDES AND VARIABLE DEFINITIONS
   ========================================================================= */

#include "AEEMediaUtil.h"
#include "AEESource.h"     // ISource and IPeek definitions
#include "AEEFile.h"       // IFile, IFileMgr
#include "AEEMimeTypes.h"  // MIME definitions
#include "AEEStdLib.h"     // StdLib functions
#include "nmdef.h"

/* =========================================================================

                      PUBLIC DATA DECLARATIONS

   ========================================================================= */
#define FMT_DETECT_MAX_BYTES     32

#define AEEMediaUtil_PEEK_RETURN(p, lCode ) { MM_RELEASEIF(p); return (lCode); }

#ifdef MM_RELEASEIF
#undef MM_RELEASEIF
#endif
#define MM_RELEASEIF(p)          if (p) { IBASE_Release((IBase*)(p)); (p) = NULL; }

/*-------------------------------------------------------------------
            Type Declarations
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
            Function Prototypes
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
            Global Constant Definitions
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
            Global Data Definitions
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
            Static variable Definitions
-------------------------------------------------------------------*/


/* =========================================================================
                     FUNCTION DEFINITIONS
   ========================================================================= */

/*=========================================================================
Function: CREATEMEDIA

Description: 
   Given AEEMediaData, this function analyzes media data and creates 
   IMedia-based object (IMedia object).
   
Prototype:

   int AEEMediaUtil_CreateMedia(AEEMediaData * pmd, IMedia ** ppm);

Parameters:
   pmd [in]:      Media data info
   ppm [out]:     IMedia object pointer returned to caller

Return Value: 
   SUCCESS:          IMedia object successfully created
   EBADPARM:         Input parameter(s) is wrong
   EUNSUPPORTED:     Class not found
   MM_EBADMEDIADATA: Media data is not correct
   MM_ENOMEDIAMEMORY:No memory to create IMedia object
   ENEEDMORE:        Need more data to create IMedia object. 
                     For MMD_ISOURCE, call IPEEK_Peekable(). (See Comments).
   MM_EPEEK:         Error while peeking for the data. (See Comments)
   MM_ENOTENOUGHDATA Aborted due to insufficient data
   EFAILED:          General failure

Comments:
   If pmd->clsData is MMD_FILE_NAME, then 
   (1) file extension is checked to see if any IMedia class is registered 
       for the extension. 
   (2) If not, the file is opened and file contents are read. Using
       ISHELL_DetectType(), the MIME of the media data is determined and IMedia class
       is found from Shell registry.

   If pmd->clsData is MMD_BUFFER, then 
   (1) The buffer contents are analyzed using ISHELL_DetectType() to determine the MIME 
       of the media data is determined and IMedia class is found from Shell registry.

   If pmd->clsData is MMD_ISOURCE, then 
   (1) The caller needs to set pmd->pData to IPeek *.
   (2) This function peeks for the data to see if enough data is available. The buffer 
       contents are analyzed using ISHELL_DetectType(), the MIME of the media data is determined 
       and IMedia class is found from Shell registry.
   (3) If enough data is not there, then it calls IPEEK_Peek() requesting required
       number of bytes. If IPEEK_Peek() returns IPEEK_WAIT, then it returns
       ENEEDMORE. In response to this, caller needs to call IPEEK_Peekable().
       After the data is becomes available, caller should call this function again to
       create IMedia object.

   If IMedia class is found, then IMedia object is created and media data is
   set. This puts IMedia object in Ready state.

Side Effects: 
   None.   

See Also:
   ISHELL_DetectType()

========================================================================= */
int AEEMediaUtil_CreateMedia(IShell * ps, AEEMediaData * pmd, IMedia ** ppm)
{
   AEECLSID    cls;
   int         nRet;
   IMedia *    pMedia;

   if (!ps || !pmd || !pmd->pData || !ppm)
      return EBADPARM;

   *ppm = NULL;

   // Find the class ID. This function uses ISHELL_DetectType() API to get the IMedia
   // MIME and subsequently class ID...
   nRet = AEEMediaUtil_FindClass(ps, pmd, &cls);
   if (nRet != SUCCESS)
      return nRet;

   // Create IMedia-based object
   if (ISHELL_CreateInstance(ps, cls, (void **)&pMedia) || !pMedia)
      return MM_ENOMEDIAMEMORY;

   // Set the media data and put IMedia in Ready state.
   nRet = IMEDIA_SetMediaData(pMedia, pmd);
   if (nRet != SUCCESS)
   {
      IMEDIA_Release(pMedia);
      return nRet;
   }

   *ppm = pMedia;

   return SUCCESS;
}

/*==================================================================
   Assumption: pmd, pmd->pData and pCls are NOT NULL.
==================================================================*/
int AEEMediaUtil_FindClass(IShell * ps, AEEMediaData * pmd, AEECLSID * pCls)
{
   void *      pBuf = NULL;
   uint32      dwReqSize = 0;
   AEECLSID    cls;
   char        szMIME[32];
   const char *cpszMIME;
   int         nErr = SUCCESS;

   *pCls = 0;

   if (ENEEDMORE != ISHELL_DetectType(ps, NULL, &dwReqSize, NULL, NULL) || !dwReqSize)
         return EFAILED;

   // If it is file name, then first check the extension.
   // If there is no extension, then read the file data and
   // analyze it to determine if it is MIDI, QCP, PMD or MP3 format.
   if (pmd->clsData == MMD_FILE_NAME)
   {
      IFileMgr *  pfm;
      IFile *     pIFile;
      int         nRead;
      char *      psz = (char *)pmd->pData;
      char *      pszExt;
      
      // Check the extension...

      pszExt = STRRCHR(psz, '.');
      if (pszExt)
         pszExt++;

      if (pszExt && *pszExt)
      {
         // Check audio MIME base
         STRCPY(szMIME, AUDIO_MIME_BASE);
         STRCAT(szMIME, pszExt);
         cls = ISHELL_GetHandler(ps, AEECLSID_MEDIA, szMIME);
         if (cls)
         {
            *pCls = cls;
            return SUCCESS;
         }

         // Check video MIME base also
         STRCPY(szMIME, VIDEO_MIME_BASE);
         STRCAT(szMIME, pszExt);
         cls = ISHELL_GetHandler(ps, AEECLSID_MEDIA, szMIME);
         if (cls)
         {
            *pCls = cls;
            return SUCCESS;
         }
      }

      // Now, read the file contents and try to determine the format...
      
      if (ISHELL_CreateInstance(ps, AEECLSID_FILEMGR, (void **)&pfm))
         return ENOMEMORY;

      pIFile = IFILEMGR_OpenFile(pfm, (char *)pmd->pData, _OFM_READ);
      if (!pIFile)
      {
         MM_RELEASEIF(pfm);
         return ENOMEMORY;
      }

      pBuf = MALLOC(dwReqSize);
      if (!pBuf)
      {
         MM_RELEASEIF(pfm);
         return ENOMEMORY;
      }

      nRead = IFILE_Read(pIFile, pBuf, dwReqSize);

      MM_RELEASEIF(pIFile);
      MM_RELEASEIF(pfm);

      if (nRead < (int)dwReqSize)
      {
         FREE(pBuf);
         return MM_ENOTENOUGHTDATA;
      }

   } // if (pmd->clsData == MMD_FILE_NAME)
   else if (pmd->clsData == MMD_BUFFER)
   {
      if (pmd->dwSize >= (uint32)dwReqSize)
         pBuf = pmd->pData;
      else
         return MM_ENOTENOUGHTDATA;
   }
   else if (pmd->clsData >= MMD_ISOURCE) // Streaming is asynchronous -- return.
   {
      ISource *   pSource = (ISource *)pmd->pData;
      IPeek *     pPeek;
      char *      pPeekedBuf;
      int32       lAvailable;
      int32       lCode;

      // Check if ISource is IPeek
      if (SUCCESS != ISOURCE_QueryInterface(pSource, AEECLSID_PEEK, (void **)&pPeek) || !pPeek)
         return MM_EBADMEDIADATA;
      
      // Peek to see if some data is available
      pPeekedBuf = IPEEK_Peek(pPeek, 0, &lAvailable, &lCode);
      if (lCode == IPEEK_END || lCode == IPEEK_ERROR || lCode == IPEEK_FULL)
         AEEMediaUtil_PEEK_RETURN(pPeek, MM_EPEEK);

      // Peek till required data is fetched
      while (lAvailable < (long)dwReqSize)
      {
         pPeekedBuf = IPEEK_Peek(pPeek, dwReqSize - lAvailable, &lAvailable, &lCode);
         if (lCode == IPEEK_END || lCode == IPEEK_ERROR || lCode == IPEEK_FULL)
            AEEMediaUtil_PEEK_RETURN(pPeek, MM_EPEEK);

         // Indicate the caller to do IPEEK_Peekable() and call this function again...
         if (lCode == IPEEK_WAIT)
            AEEMediaUtil_PEEK_RETURN(pPeek, ENEEDMORE);
      }

      if (!pPeekedBuf)
         AEEMediaUtil_PEEK_RETURN(pPeek, MM_EPEEK);


      pBuf = MALLOC(dwReqSize);
      if (!pBuf)
         AEEMediaUtil_PEEK_RETURN(pPeek, ENOMEMORY);

      MEMCPY(pBuf, pPeekedBuf, dwReqSize);

      MM_RELEASEIF(pPeek);
   }

   nErr = ISHELL_DetectType(ps, pBuf, &dwReqSize, NULL, &cpszMIME);

   if (!nErr)
   {
      cls = ISHELL_GetHandler(ps, AEECLSID_MEDIA, cpszMIME);
      if (cls)
         *pCls = cls;
   }

   if (pBuf && pBuf != pmd->pData)
      FREE(pBuf);
   
   return nErr;
}

/* ============= End of File =============================================== */
