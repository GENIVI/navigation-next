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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "test_core_memorystream.h"
#include "palstdlib.h"
#include "nbrememorystream.h"
#include "winutility.h"
#include <iostream>

/* Pointer to the file used by the tests. */

static uint8* srcBuff = NULL;
static uint8* recvBuff = NULL;
static uint8* compareBuff = NULL;
static uint32 dataSize = 0;
static PAL_Instance* m_Pal = NULL;
static PAL_File* file = NULL;

/* The suite setup function.
 * Check if file is exist, and create PAL instance
 * And get file size.
 */
static int SuiteSetup(void)
{

    //get test file name
    FILE* testFile = NULL;
    uint32 readSize = 0;


    
    char* exeDir = GetExePath();
    char* name = "testfile\\testfile_1k.txt";
    char* fileName = (char*)nsl_malloc((nb_usize)(strlen(exeDir)+strlen(name)+1));
    nsl_strcpy(fileName, exeDir);
    nsl_strcat(fileName, name);

    if (NULL == (testFile = fopen(fileName, "rb+"))) {
        return -1;
    }

    else {
        fclose(testFile);
        if(m_Pal == NULL)
        {
            PAL_Config palConfig = {0};
            m_Pal = PAL_Create(&palConfig);
        }
        PAL_FileOpen(m_Pal, fileName, PFM_Read, &file);
        compareBuff = (uint8*)nsl_malloc(MAX_SIZE);
        nsl_memset(compareBuff, 0, MAX_SIZE);
        srcBuff = (uint8*)nsl_malloc(MAX_SIZE);
        nsl_memset(srcBuff, 0, MAX_SIZE);
        //read data to memory
        PAL_FileRead(file, (uint8*)srcBuff, MAX_SIZE, &readSize);
        dataSize = readSize;
        recvBuff = (uint8*)nsl_malloc(MAX_SIZE);
        return 0;
    }
    
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Destroy PAL instance.
 */
static int SuiteCleanup(void)
{

    nsl_free(recvBuff);
    nsl_free(srcBuff);
    nsl_free(compareBuff);
    PAL_FileClose(file);
    PAL_Destroy(m_Pal);
    return 0;    
}

/* Memorystream test of read().
 * set some wrong parameters and check
 * whether corresponding error code
 */
void Test_ReadMemory_Errorcode(void)
{
    uint32 readSize = 0;
    uint32 cacheSize = 0;
    uint32 dataSize = 0;
    PAL_Error err = PAL_Ok;
    NBRE_MemoryStream memoryStream(srcBuff, MAX_SIZE, TRUE, cacheSize);
  
    //check Error code when buff is NULL
    err = memoryStream.Read(NULL,dataSize,&readSize);
    CU_ASSERT( PAL_ErrBadParam == err);

    //check Error code when readSize is Null
    err = memoryStream.Read(recvBuff,dataSize,NULL);
    CU_ASSERT( PAL_ErrBadParam == err);
    nsl_memset(recvBuff, 0, MAX_SIZE);
}

/* Memorystream test of read(). cashSize is set 0
 * Read data from the test file and check
 * whether the expected number of bytes were read.
 */
void Test_ReadMemory_NoCache(void)
{
    uint32 readSize = 0;
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;

    NBRE_MemoryStream memoryStream(srcBuff, MAX_SIZE , TRUE, cacheSize);
    
    //read size is 0
    err = memoryStream.Read(recvBuff,0,&readSize);
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( 0 == readSize);
    
    //check read size less than buffer
    err = memoryStream.Read(recvBuff,10,&readSize);
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( 10 == readSize);

    //read file content using other method, then compare.
    nsl_memcpy(compareBuff,srcBuff,10);
    CU_ASSERT(0 == nsl_memcmp(recvBuff,compareBuff,10));

    //recovery default value.
    nsl_memset(recvBuff, 0, MAX_SIZE);
    nsl_memset(compareBuff, 0, MAX_SIZE);

    //check read size greater than buffer
    readSize = 0;
    //come back to beginning 
    memoryStream.Seek(PFSO_Current,-10);
    err = memoryStream.Read(recvBuff,MAX_SIZE + 1,&readSize);
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( MAX_SIZE == readSize);
    nsl_memcpy(compareBuff,srcBuff,MAX_SIZE + 1);
    CU_ASSERT(0 == nsl_memcmp(recvBuff,compareBuff,MAX_SIZE + 1));
    nsl_memset(recvBuff, 0, MAX_SIZE);
    nsl_memset(compareBuff, 0, MAX_SIZE);

}
/* Memorystream test of read(). cashSize is set none zero
 * Read data from the test file and check
 * whether the expected number of bytes were read.
 */
void Test_ReadMemory_SetCache(void)
{

    uint32 readSize = 0;
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;

    //cache size less than read size
    cacheSize = dataSize - 10;
    NBRE_MemoryStream memoryStream(srcBuff, MAX_SIZE, TRUE, cacheSize);
    if (recvBuff != NULL)
    {
        err = memoryStream.Read(recvBuff,dataSize,&readSize);
    }
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( dataSize == readSize)
    nsl_memcpy(compareBuff,srcBuff,dataSize);
    CU_ASSERT(0 == nsl_memcmp(recvBuff,compareBuff,dataSize));

    nsl_memset(recvBuff, 0, MAX_SIZE);
    nsl_memset(compareBuff, 0, MAX_SIZE);

    // cache size equal to fileSize
    readSize = 0;
    cacheSize = dataSize;
    NBRE_MemoryStream memoryStream2(srcBuff, MAX_SIZE, TRUE, cacheSize);
    
    if (recvBuff != NULL)
    {
        err = memoryStream2.Read(recvBuff,dataSize,&readSize);
    }
    //Check if read file is success.
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( dataSize == readSize)
    nsl_memcpy(compareBuff,srcBuff,dataSize);
    CU_ASSERT(0 == nsl_memcmp(recvBuff,compareBuff,dataSize));
    nsl_memset(recvBuff, 0, MAX_SIZE);
    nsl_memset(compareBuff, 0, MAX_SIZE);

    //cache size greater than fileSize
    cacheSize = dataSize + 100;
    readSize = 0;
    NBRE_MemoryStream memoryStream3(srcBuff, MAX_SIZE, TRUE, cacheSize);
    if (recvBuff != NULL)
    {
        err = memoryStream3.Read(recvBuff,dataSize,&readSize);
    }
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( dataSize == readSize)
    nsl_memcpy(compareBuff,srcBuff,dataSize);
    CU_ASSERT(0 == nsl_memcmp(recvBuff,compareBuff,dataSize));
    nsl_memset(recvBuff, 0, MAX_SIZE);
    nsl_memset(compareBuff, 0, MAX_SIZE);

    //just read part content
    cacheSize = dataSize + 100;
    readSize = 0;
    NBRE_MemoryStream memoryStream4(srcBuff, MAX_SIZE, TRUE, cacheSize);
    if (recvBuff != NULL)
    {
        err = memoryStream4.Read(recvBuff, dataSize - 1 ,&readSize);
    }
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( dataSize - 1 == readSize);
    nsl_memcpy(compareBuff,srcBuff,dataSize - 1);
    CU_ASSERT(0 == nsl_memcmp(recvBuff,compareBuff,dataSize - 1));
    nsl_memset(recvBuff, 0, MAX_SIZE);
    nsl_memset(compareBuff, 0, MAX_SIZE);


    /*carry on read next char from the last time the location of then end
    * only one character
    */
    uint32 position = memoryStream4.GetPos();
    uint32 temp = position;
    CU_ASSERT(position == readSize);
    readSize = 0;
    if (recvBuff != NULL)
    {
        err = memoryStream4.Read(recvBuff,10,&readSize);
    }
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream4.GetPos();
    CU_ASSERT(10 == position - temp);
    nsl_memset(recvBuff, 0, MAX_SIZE);

}

/* Simple test of seek().
 * precise positioning
 */
void Test_SeekMemory_FromCurrent(void)
{
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;
    uint32 position = 0;

    //offset size is 0
    NBRE_MemoryStream memoryStream(srcBuff, MAX_SIZE, TRUE, cacheSize);
    err = memoryStream.Seek(PFSO_Current,0);
    CU_ASSERT(PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(0 == position);


    err = memoryStream.Seek(PFSO_Current,10);
    CU_ASSERT(PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(10 == position);

    //offset size greater than fileSize
    err = memoryStream.Seek(PFSO_Current,MAX_SIZE + 1);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(MAX_SIZE == position);

    //offset size equal to fileSize
    err = memoryStream.Seek(PFSO_Current,-MAX_SIZE);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(0 == position);
    
    //come back beginning of file
    err = memoryStream.Seek(PFSO_Current,-1);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(0 == position);

    //continuously call seek with same offset
    err = memoryStream.Seek(PFSO_Current,10);
    err = memoryStream.Seek(PFSO_Current,0);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(10 == position);
    err = memoryStream.Seek(PFSO_Current,10);
    position = memoryStream.GetPos();
    CU_ASSERT(20 == position);
    err = memoryStream.Seek(PFSO_Current,-30);
    position = memoryStream.GetPos();
    CU_ASSERT(0 == position);
}

/* Simple test of seek().
 * precise positioning
 */
void Test_SeekMemory_FromBegin(void)
{
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;
    uint32 position = 0;

    //offset size is 0
    NBRE_MemoryStream memoryStream(srcBuff, MAX_SIZE, TRUE, cacheSize);
    err = memoryStream.Seek(PFSO_Start,0);
    CU_ASSERT(PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(0 == position);

    //offset size less than dataSize
    err = memoryStream.Seek(PFSO_Start,dataSize - 1);
    CU_ASSERT(PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(dataSize - 1 == position);
    
    //offset size greater than fileSize
    err = memoryStream.Seek(PFSO_Start,MAX_SIZE + 1);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(MAX_SIZE == position);
    
    //offset size equal to fileSize
    err = memoryStream.Seek(PFSO_Start,dataSize);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(dataSize == position);

    //continuously call seek with same offset, position is not changed.
    err = memoryStream.Seek(PFSO_Start,10);
    err = memoryStream.Seek(PFSO_Start,10);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(10 == position);

    // offset size less than zero
    err =memoryStream.Seek(PFSO_Start,-1);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    //CU_ASSERT(0 == position);
}

/* Simple test of seek().
 * precise positioning
 */
void Test_SeekMemory_FromEnd(void)
{
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;
    uint32 position = 0;

    //offset size is 0
    NBRE_MemoryStream memoryStream(srcBuff, MAX_SIZE, TRUE, cacheSize);
    err = memoryStream.Seek(PFSO_End,0);
    CU_ASSERT(PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(dataSize == position);

    //offset size less than fileSize
    err = memoryStream.Seek(PFSO_End,-10);
    CU_ASSERT(PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(MAX_SIZE - 10 == position);

    //offset size greater than fileSize
    err = memoryStream.Seek(PFSO_End,(-MAX_SIZE)- 1);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(0 == position);

    //offset size equal to fileSize

    err = memoryStream.Seek(PFSO_End,-MAX_SIZE);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(0 == position);

    //continuously call seek with same offset, position is not changed.
    err = memoryStream.Seek(PFSO_End,-10);
    err = memoryStream.Seek(PFSO_End,-10);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(MAX_SIZE - 10 == position);

    // offset size greater than zero
    err = memoryStream.Seek(PFSO_End,1);
    CU_ASSERT( PAL_Ok == err);
    position = memoryStream.GetPos();
    CU_ASSERT(0 == position);
}

/* Simple test of seek().
 * precise positioning
 */
void Test_SeekMemory_ErrorCode(void)
{
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;

    NBRE_MemoryStream memoryStream(srcBuff, MAX_SIZE + 1, TRUE, cacheSize);
    err = memoryStream.Read(recvBuff,dataSize,NULL);
    err = memoryStream.Seek(PFSO_Current,0);
    CU_ASSERT(PAL_Ok == err);

}

/* Simple test of EOF().
 * query data from special file
 */
void Test_ReadMemory_EOF(void)
{
    uint32 readSize = 0;
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;

    NBRE_MemoryStream memoryStream(srcBuff, 0, TRUE, cacheSize);
    if (recvBuff != NULL)
    {
        err = memoryStream.Read(recvBuff,dataSize,&readSize);
    }
    CU_ASSERT( PAL_ErrEOF == err);
    nsl_memset(recvBuff, 0, MAX_SIZE);
}

/* Simple test of GetPos().
 * get current location.
 */
void Test_ReadMemory_GetPos(void)
{
    uint32 readSize = 0;
    uint32 cacheSize = MAX_SIZE;
    PAL_Error err = PAL_Ok;
    uint32 position = 0;

    NBRE_MemoryStream memoryStream(srcBuff, MAX_SIZE, TRUE, cacheSize);
    //check starting location
    position = memoryStream.GetPos();
    CU_ASSERT( 0 == position);
    
    //check middle location
    err = memoryStream.Read(recvBuff, dataSize, &readSize);
    position = memoryStream.GetPos();
    CU_ASSERT (dataSize == position);
    
    //check end location
    err = memoryStream.Read(recvBuff, dataSize + MAX_SIZE, &readSize);
    position = memoryStream.GetPos();
    CU_ASSERT (MAX_SIZE == position);
    
    //come back to starting location
    //nsl_memset(srcBuff, 0, MAX_SIZE);
    memoryStream.Seek(PFSO_Current, - MAX_SIZE);
    position = memoryStream.GetPos();
    CU_ASSERT( 0 == position);    
}
void Test_SrcData_Copy(void)
{
    uint32 readSize = 0;
    uint32 cacheSize = MAX_SIZE;
    PAL_Error err = PAL_Ok;

    //don't copy data
    NBRE_MemoryStream memoryStream(srcBuff, MAX_SIZE, FALSE, cacheSize);
    err = memoryStream.Read(recvBuff, dataSize, &readSize);
    CU_ASSERT( PAL_Ok == err );
    
    NBRE_MemoryStream memoryStream1(srcBuff, MAX_SIZE, TRUE, cacheSize);
    err = memoryStream1.Read(recvBuff, dataSize, &readSize);
    CU_ASSERT( PAL_Ok == err );
}

//add test cases to suite
TestCoreMemoryStream::TestCoreMemoryStream()
    :NBRE_TestSuite("test_core_memorystream", SuiteSetup, SuiteCleanup)
{
    AddTestCase(new NBRE_TestCase("test_read_nocache", Test_ReadMemory_NoCache));
    AddTestCase(new NBRE_TestCase("test_read_setcache", Test_ReadMemory_SetCache));
    AddTestCase(new NBRE_TestCase("test_read_errorcode", Test_ReadMemory_Errorcode));
    AddTestCase(new NBRE_TestCase("test_seek_fromcurrent", Test_SeekMemory_FromCurrent));
    AddTestCase(new NBRE_TestCase("test_seek_frombegin", Test_SeekMemory_FromBegin));
    AddTestCase(new NBRE_TestCase("test_seek_fromend", Test_SeekMemory_FromEnd));
    AddTestCase(new NBRE_TestCase("test_seek_errorcode", Test_SeekMemory_ErrorCode));
    AddTestCase(new NBRE_TestCase("test_EOF", Test_ReadMemory_EOF));
    AddTestCase(new NBRE_TestCase("test_getpos", Test_ReadMemory_GetPos));
    AddTestCase(new NBRE_TestCase("test_srcdata_copy", Test_SrcData_Copy));
}



