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

/*!--------------------------------------------------------------------------

    @file abfilesetinformation.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef ABFILESETINFORMATION_H
#define ABFILESETINFORMATION_H

#include "abexp.h"
#include "nberror.h"


/*!
    @addtogroup abfilesetinformation
    @{
*/


/*! @struct AB_FileSetInformation
Information about the results of a file set request
*/
typedef struct AB_FileSetInformation AB_FileSetInformation;


/*! Get the count of file sets downloaded from the server

@param information An AB_FileSetInformation object
@param fileSetRetrievedCount Pointer to buffer for number of file sets downloaded
@returns NB_Error
*/
AB_DEC NB_Error AB_FileSetInformationGetCount(
    AB_FileSetInformation* information,
    uint32* fileSetRetrievedCount
);


/*! Get a retrieved file set name and timestamp by index

This function returns the name and time stamp of the retrieved file set, specified by index.

@param information An AB_FileSetInformation object
@param fileSetIndex Index of file set to get status info of
@param fileSetName Pointer to buffer to store file set name in
@param fileSetNameSize Maximum size of file set name buffer
@param fileSetTimeStamp Pointer to buffer to store retrieved file set time stamp in
@param fileSetFileCount Pointer to buffer to store count of files contained in the file set
@returns NB_Error
*/
AB_DEC NB_Error AB_FileSetInformationGetStatus(
    AB_FileSetInformation* information,
    uint32 fileSetIndex,
    char* fileSetName,
    nb_size fileSetNameSize,
    uint64* fileSetTimeStamp,
    uint32* fileSetFileCount
);    


/*! Get retrieved file set file name and data by index

This function returns a pointer to the retrieved file set data and the size of the data.
The file set data pointer is valid for the life of the AB_FileSetInformation object.

@param information An AB_FileSetInformation object
@param fileSetIndex Index of file set
@parma fileIndex Index of file within file set 
@param fileName Pointer to buffer to store file name in
@param fileNameSize Maximum size of file name buffer
@param fileData Pointer to buffer to store pointer to file data in
@param fileDataSize Pointer to buffer to store file data size in
@returns NB_Error
*/
AB_DEC NB_Error AB_FileSetInformationGetFileData(
    AB_FileSetInformation* information,
    uint32 fileSetIndex,
    uint32 fileIndex,
    char* fileName,
    nb_size fileNameSize,
    byte** fileData,
    nb_size* fileDataSize
);    


/*! Destroy a previously created FileSetInformation object

@param information An AB_FileSetInformation object created with AB_FileSetHandlerGetInformation()
@returns NB_Error
*/
AB_DEC NB_Error AB_FileSetInformationDestroy(
    AB_FileSetInformation* information
);


#endif
