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

#include "test_core_filestream.h"
#include "palstdlib.h"
#include "nbrefilestream.h"
#include "winutility.h"

/* Pointer to the file used by the tests. */
static FILE* testFile = NULL;
static char* fileName = NULL;
static PAL_Instance* m_Pal = NULL;
static uint32 fileSize = 0;
static uint8* recvBuff = NULL;


/* The suite setup function.
 * Check if file is exist, and create PAL instance
 * And get file size.
 */
static int SuiteSetup(void)
{
    //get test file name
    char* exeDir = GetExePath();
    char* name = "testfile\\testfile_1k.txt";
    fileName = (char*)nsl_malloc((nb_usize)(strlen(exeDir)+strlen(name)+1));
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

        PAL_FileGetSize(m_Pal, fileName, &fileSize);
        recvBuff = (uint8*)nsl_malloc(MAX_FILE);
        return 0;
    }
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Destroy PAL instance.
 */
static  int SuiteCleanup(void)
{
    if (0 != fclose(testFile)) {
        return -1;
    }
    else {
        testFile = NULL;
        PAL_Destroy(m_Pal);
        nsl_free(recvBuff);
        return 0;
    }
}

/* Filestream test of read().
 * set some wrong parameters and check
 * whether corresponding error code
 */
void Test_Read_Errorcode(void)
{
    PAL_File* file = NULL;
    PAL_FileOpen(m_Pal, fileName, PFM_Read, &file);
    uint32 readSize = 0;
    uint32 cashSize = 0;
    uint32 dataSize = fileSize;
    PAL_Error err = PAL_Ok;
    NBRE_FileStream fileStream(file, dataSize,cashSize);
    
    //check Error code when buff is NULL
    err = fileStream.Read(NULL,dataSize,&readSize);
    CU_ASSERT( PAL_ErrBadParam == err);

    //check Error code when readSize is Null
    err = fileStream.Read(recvBuff,dataSize,NULL);
    CU_ASSERT( PAL_ErrBadParam == err);
    PAL_FileClose(file);
    nsl_memset(recvBuff, 0, MAX_FILE);
}

/* Filestream test of read(). cashSize is set 0
 * Read data from the test file and check
 * whether the expected number of bytes were read.
 */
void Test_Read_NoCache(void)
{
    PAL_File* file = NULL;
    PAL_FileOpen(m_Pal, fileName, PFM_Read, &file);
    uint32 readSize = 0;
    uint32 cacheSize = 0;
    uint32 temp = readSize;
    uint32 dataSize = fileSize;
    PAL_Error err = PAL_Ok;

    NBRE_FileStream fileStream(file, dataSize,cacheSize);
    //NBRE_FileStream* fileStream = new NBRE_FileStream(file,fileSize,cashSize);

    if (recvBuff != NULL)
    {
        err = fileStream.Read(recvBuff,dataSize,&readSize);
        //err = fileStream->Read(recvBuff,fileSize,&readSize);
    }
    //Check if read file is success.
    CU_ASSERT( PAL_Ok == err);
    //read file content using other method, then compare.
    uint8* data = 0;
    data = (uint8*)nsl_malloc(dataSize);
    PAL_FileRead(file, (uint8*)data, dataSize, &temp);
    CU_ASSERT(0 == nsl_memcmp(recvBuff,data,cacheSize));
    nsl_memset(recvBuff, 0, MAX_FILE);
    nsl_free(data);
    
    //check read size greater than buffer
    PAL_FileSetPosition(file,PFSO_Start,0);
    dataSize = fileSize + 1;
    readSize = 0;
    NBRE_FileStream fileStream2(file, dataSize,cacheSize);
    err = fileStream2.Read(recvBuff,dataSize,&readSize);
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( fileSize == readSize);
    nsl_memset(recvBuff, 0, MAX_FILE);
    PAL_FileClose(file);
}
/* Filestream test of read(). cashSize is set none zero
 * Read data from the test file and check
 * whether the expected number of bytes were read.
 */
void Test_Read_SetCache(void)
{
    PAL_File* file = NULL;
    PAL_FileOpen(m_Pal, fileName, PFM_Read, &file);
    uint32 readSize = 0;
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;

    //cache size less than fileSize
    cacheSize = fileSize - 1;
    NBRE_FileStream fileStream1(file, fileSize, cacheSize);
    if (recvBuff != NULL)
    {
        err = fileStream1.Read(recvBuff,fileSize,&readSize);
    }
    //Check if read file is success.
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( fileSize == readSize)
    nsl_memset(recvBuff, 0, MAX_FILE);

    // cache size equal to fileSize
    PAL_FileSetPosition(file,PFSO_Start,0);
    cacheSize = fileSize;
    readSize = 0;
    NBRE_FileStream fileStream2(file, fileSize,cacheSize);
    
    if (recvBuff != NULL)
    {
        err = fileStream2.Read(recvBuff,fileSize,&readSize);
    }

    //Check if read file is success.
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( fileSize == readSize);
    nsl_memset(recvBuff, 0, MAX_FILE);


    //cache size greater than fileSize
    PAL_FileSetPosition(file,PFSO_Start,0);
    cacheSize = fileSize + 100;
    readSize = 0;
    NBRE_FileStream fileStream3(file, fileSize,cacheSize);
    if (recvBuff != NULL)
    {
        err = fileStream3.Read(recvBuff,fileSize,&readSize);
    }
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( fileSize == readSize);
    //check read content is right
    uint8* data = 0;
    uint32 temp = 0;
    data = (uint8*)nsl_malloc(fileSize);
    PAL_FileSetPosition(file,PFSO_Start,0);
    PAL_FileRead(file, (uint8*)data, fileSize, &temp);
    CU_ASSERT(0 == nsl_memcmp(recvBuff,data,fileSize));
    nsl_memset(recvBuff, 0, MAX_FILE);
    nsl_free (data);

    //just read part file content
    PAL_FileSetPosition(file,PFSO_Start,0);
    readSize = 0;
    NBRE_FileStream fileStream4(file, fileSize,cacheSize);
    if (recvBuff != NULL)
    {
        err = fileStream4.Read(recvBuff,fileSize - 1,&readSize);
    }
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( fileSize - 1 == readSize);
    nsl_memset(recvBuff, 0, MAX_FILE);


    /*carry on read next char from the last time the location of then end
    * only one character
    */
    readSize = 0;
    //uint32 position = 0;
    //PAL_FileGetPosition(file,&position);
    if (recvBuff != NULL)
    {
        err = fileStream4.Read(recvBuff,10,&readSize);
    }
    CU_ASSERT( PAL_Ok == err);
    CU_ASSERT( 1 == readSize);
    nsl_memset(recvBuff, 0, MAX_FILE);
    PAL_FileClose(file);
}

/* Simple test of seek().
 * precise positioning
 */
void Test_Seek_FromCurrent(void)
{
    PAL_File* file = NULL;
    PAL_FileOpen(m_Pal, fileName, PFM_Read, &file);
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;
    uint32 position = 0;

    //offset size is 0
    NBRE_FileStream fileStream(file, fileSize, cacheSize);
    err = fileStream.Seek(PFSO_Current,0);
    CU_ASSERT(PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(0 == position);

    //offset size less than fileSize
    NBRE_FileStream fileStream1(file, fileSize, cacheSize);
    err = fileStream1.Seek(PFSO_Current,10);
    CU_ASSERT(PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(10 == position);

    //offset size greater than fileSize
    err = fileStream1.Seek(PFSO_Current,fileSize + 1);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(fileSize == position);

    //offset size equal to fileSize
    err = fileStream1.Seek(PFSO_Current,-(int)fileSize);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(0 == position);
    
    //come back beginning of file
    err = fileStream1.Seek(PFSO_Current,-1);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(0 == position);

    //continuously call seek with same offset
    PAL_FileSetPosition(file,PFSO_Start,0);
    err = fileStream1.Seek(PFSO_Current,10);
    err = fileStream1.Seek(PFSO_Current,0);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(10 == position);
    err = fileStream1.Seek(PFSO_Current,10);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(20 == position);
    err = fileStream1.Seek(PFSO_Current,-30);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(0 == position);
    PAL_FileClose(file);
}

/* Simple test of seek().
 * precise positioning
 */
void Test_Seek_FromBegin(void)
{
    PAL_File* file = NULL;
    PAL_FileOpen(m_Pal, fileName, PFM_Read, &file);
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;
    uint32 position = 0;

    //offset size is 0
    NBRE_FileStream fileStream(file, fileSize, cacheSize);
    err = fileStream.Seek(PFSO_Start,0);
    CU_ASSERT(PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(0 == position);

    //offset size less than fileSize
    NBRE_FileStream fileStream1(file, fileSize, cacheSize);
    err = fileStream1.Seek(PFSO_Start,10);
    CU_ASSERT(PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(10 == position);
    
    //offset size greater than fileSize
    NBRE_FileStream fileStream2(file, fileSize, cacheSize);
    err = fileStream2.Seek(PFSO_Start,fileSize + 1);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(fileSize == position);
    
    //offset size equal to fileSize
    NBRE_FileStream fileStream3(file, fileSize, cacheSize);
    err = fileStream3.Seek(PFSO_Start,fileSize);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(fileSize == position);

    //continuously call seek with same offset, position is not changed.
    NBRE_FileStream fileStream4(file, fileSize, cacheSize);
    err = fileStream4.Seek(PFSO_Start,10);
    err = fileStream4.Seek(PFSO_Start,10);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(10 == position);

    // offset size less than zero
    NBRE_FileStream fileStream5(file, fileSize, cacheSize);
    err = fileStream5.Seek(PFSO_Start,-1);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    //CU_ASSERT(0 == position);
    PAL_FileClose(file);
}

/* Simple test of seek().
 * precise positioning
 */
void Test_Seek_FromEnd(void)
{
    PAL_File* file = NULL;
    PAL_FileOpen(m_Pal, fileName, PFM_Read, &file);
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;
    uint32 position = 0;

    //offset size is 0
    NBRE_FileStream fileStream(file, fileSize, cacheSize);
    err = fileStream.Seek(PFSO_End,0);
    CU_ASSERT(PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(fileSize == position);

    //offset size less than fileSize
    NBRE_FileStream fileStream1(file, fileSize, cacheSize);
    err = fileStream1.Seek(PFSO_End,-10);
    CU_ASSERT(PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(fileSize - 10 == position);

    //offset size greater than fileSize
    NBRE_FileStream fileStream2(file, fileSize, cacheSize);
    err = fileStream2.Seek(PFSO_End,(-(int)fileSize)- 1);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(0 == position);

    //offset size equal to fileSize
    NBRE_FileStream fileStream3(file, fileSize, cacheSize);
    err = fileStream3.Seek(PFSO_End,-(int)fileSize);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(0 == position);

    //continuously call seek with same offset, position is not changed.
    NBRE_FileStream fileStream4(file, fileSize, cacheSize);
    err = fileStream4.Seek(PFSO_End,-10);
    err = fileStream4.Seek(PFSO_End,-10);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(fileSize - 10 == position);

    // offset size greater than zero
    NBRE_FileStream fileStream5(file, fileSize, cacheSize);
    err = fileStream5.Seek(PFSO_End,1);
    CU_ASSERT( PAL_Ok == err);
    PAL_FileGetPosition(file, &position);
    CU_ASSERT(0 == position);
    PAL_FileClose(file);
}

/* Simple test of seek().
 * precise positioning
 */
void Test_Seek_ErrorCode(void)
{
    PAL_File* file = NULL;
    PAL_FileOpen(m_Pal, fileName, PFM_Read, &file);
    uint32 cacheSize = 0;
    PAL_Error err = PAL_Ok;

    NBRE_FileStream fileStream(NULL, fileSize, cacheSize);
    err = fileStream.Seek(PFSO_Start,10);
    CU_ASSERT(PAL_ErrFileFailed == err);
    PAL_FileClose(file);

}

/* Simple test of EOF().
 * query data from special file
 */
void Test_EOF(void)
{
    PAL_File* file = NULL;
    PAL_FileOpen(m_Pal, fileName, PFM_Read, &file);
    uint32 readSize = 0;
    uint32 cacheSize = fileSize;
    PAL_Error err = PAL_Ok;

    NBRE_FileStream fileStream(file, 0, cacheSize);
    if (recvBuff != NULL)
    {
        err = fileStream.Read(recvBuff,fileSize,&readSize);
    }
    CU_ASSERT( PAL_ErrEOF == err);
    nsl_memset(recvBuff, 0, MAX_FILE);
    PAL_FileClose(file);
}

/* Simple test of GetPos().
 * get current location.
 */
void Test_GetPos(void)
{
    PAL_File* file = NULL;
    PAL_FileOpen(m_Pal, fileName, PFM_Read, &file);
    uint32 readSize = 0;
    uint32 cacheSize = fileSize;
    PAL_Error err = PAL_Ok;
    uint32 position = 0;

    NBRE_FileStream fileStream(file, fileSize, cacheSize);
    err = fileStream.Read(recvBuff,fileSize,&readSize);
    position = fileStream.GetPos();
    CU_ASSERT( fileSize == position);

    fileStream.Seek(PFSO_Current, -(int)fileSize);
    position = fileStream.GetPos();
    CU_ASSERT( 0 == position);
    
    fileStream.Seek(PFSO_Current, 10);
    position = fileStream.GetPos();
    CU_ASSERT( 10 == position);

    PAL_FileClose(file);
}

////add test cases to suite
TestCoreFileStream::TestCoreFileStream()
    :NBRE_TestSuite("test_core_filestream", SuiteSetup, SuiteCleanup)
{
    AddTestCase(new NBRE_TestCase("test_read_nocache", Test_Read_NoCache));
    AddTestCase(new NBRE_TestCase("test_read_setcache", Test_Read_SetCache));
    AddTestCase(new NBRE_TestCase("test_read_errorcode", Test_Read_Errorcode));
    AddTestCase(new NBRE_TestCase("test_seek_fromcurrent", Test_Seek_FromCurrent));
    AddTestCase(new NBRE_TestCase("test_seek_frombegin", Test_Seek_FromBegin));
    AddTestCase(new NBRE_TestCase("test_seek_fromend", Test_Seek_FromEnd));
    AddTestCase(new NBRE_TestCase("test_seek_errorcode", Test_Seek_ErrorCode));
    AddTestCase(new NBRE_TestCase("test_EOF", Test_EOF));
    AddTestCase(new NBRE_TestCase("test_getpos", Test_GetPos));
}



