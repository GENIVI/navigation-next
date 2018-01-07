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

    @defgroup TestFile_h Unit Tests for PAL File

    Unit Tests for PAL file

    This file contains all unit tests for the PAL file component
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

/*! @{ */

#include "testfile.h"
#include "main.h"
#include "palfile.h"
#include "PlatformUtil.h"

#ifdef WINCE
#include "winbase.h"
#endif

#if WINCE || WIN32
#define DIR_SEPARATOR "\\"
#else
#define DIR_SEPARATOR "/"
#endif

#if defined(ANDROID_NDK)
#define TEST_DIR "./mnt/sdcard/testdir"
#else
#define TEST_DIR "testdir"
#endif

#define TEST_FILE1    "f1"
#define TEST_FILE2    "f2"
#define TEST_FILE1_FULL_PATH   TEST_DIR DIR_SEPARATOR TEST_FILE1
#define TEST_FILE2_FULL_PATH   TEST_DIR DIR_SEPARATOR TEST_FILE2
#define TEST_FILE1_NEW_FULL_PATH   TEST_FILE1_FULL_PATH ".new"

#define TEST_SUBDIR    "subdir"
#define TEST_SUBDIR_FULL_PATH  (TEST_DIR DIR_SEPARATOR TEST_SUBDIR)

#define TEST_FILE_STR "nimnbipal"


// Local Functions ...............................................................................


static void TestFileExists(void);
static void TestPalCreate(void);
static void TestDirectoryCreate(void);
static void TestDirectoryRemove(void);
static void TestDirectoryRemoveRecursive(void);
static void TestFileWrite(void);
static void TestFileRead(void);
static void TestFileLoad(void);
static void TestFileGetSize(void);
static void TestFileGetAttributes(void);
static void TestFileTruncate(void);
static void TestFileRename(void);
static void TestFileRemove(void);
static void TestFileEnum(void);
static void TestFileEnumFilesOnly(void);
static void TestGetFreeSpace(void);
static void TestGetTotalSpace(void);
static void TestFileIsDirectory(void);
static void TestFilePathAppend(void);
static void TestFilePathRemove(void);
static void TestFileGetLastPath(void);
static void TestFileCreateDirectoryEx(void);


// Public Functions ..............................................................................


/*! Add all your test functions here

@return None
*/
void
TestFile_AddAllTests( CU_pSuite pTestSuite )
{
	// ! Add all your function names here !
    CU_add_test(pTestSuite, "TestPalCreate",                &TestPalCreate);
    CU_add_test(pTestSuite, "TestDirectoryCreate",          &TestDirectoryCreate);
    CU_add_test(pTestSuite, "TestFileWrite",                &TestFileWrite);
    CU_add_test(pTestSuite, "TestFileGetSize",              &TestFileGetSize);
    CU_add_test(pTestSuite, "TestFileGetAttributes",        &TestFileGetAttributes);
    CU_add_test(pTestSuite, "TestFileRead",                 &TestFileRead);
    CU_add_test(pTestSuite, "TestFileLoad",                 &TestFileLoad);
    CU_add_test(pTestSuite, "TestFileExists",               &TestFileExists);
    CU_add_test(pTestSuite, "TestFileTruncate",             &TestFileTruncate);
    CU_add_test(pTestSuite, "TestFileEnum",                 &TestFileEnum);
    CU_add_test(pTestSuite, "TestFileEnumFilesOnly",        &TestFileEnumFilesOnly);
    CU_add_test(pTestSuite, "TestFileRename",               &TestFileRename);
    CU_add_test(pTestSuite, "TestFileRemove",               &TestFileRemove);
    CU_add_test(pTestSuite, "TestFileIsDirectory",          &TestFileIsDirectory);
    CU_add_test(pTestSuite, "TestDirectoryRemove",          &TestDirectoryRemove);
    CU_add_test(pTestSuite, "TestDirectoryRemoveRecursive", &TestDirectoryRemoveRecursive);
    CU_add_test(pTestSuite, "TestGetFreeSpace",             &TestGetFreeSpace);
    CU_add_test(pTestSuite, "TestGetTotalSpace",            &TestGetTotalSpace);
    CU_add_test(pTestSuite, "TestFilePathAppend",           &TestFilePathAppend);
    CU_add_test(pTestSuite, "TestFilePathRemove",           &TestFilePathRemove);
    CU_add_test(pTestSuite, "TestFileGetLastPath",          &TestFileGetLastPath);
    CU_add_test(pTestSuite, "TestFileCreateDirectoryEx",    &TestFileCreateDirectoryEx);
};


/*! Add common initialization code here.

@return 0

@see TestFile_SuiteCleanup
*/
int
TestFile_SuiteSetup()
{
	return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestFile_SuiteSetup
*/
int 
TestFile_SuiteCleanup()
{
	return 0;
}


// Local Functions ...............................................................................


// All tests. Add all tests to the TestFile_AddAllTests function

/*! @brief Test simple network connect.

@return None. CUnit asserts are called on failure.

@see TestFile_Initialize
@see TestFile_Cleanup
*/
static void TestFileExists(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
	CU_ASSERT(PAL_FileExists(pal, "non_existing_file") == PAL_ErrFileNotExist);
	CU_ASSERT(PAL_FileExists(pal, TEST_FILE1_FULL_PATH) == PAL_Ok);
	PAL_Destroy(pal);
}

static void TestPalCreate(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
	CU_ASSERT(pal != NULL);
	PAL_Destroy(pal);
}

static void TestDirectoryCreate(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(PAL_FileCreateDirectory(pal, TEST_DIR) == PAL_Ok);
    CU_ASSERT(PAL_FileCreateDirectory(pal, TEST_SUBDIR_FULL_PATH) == PAL_Ok);
	PAL_Destroy(pal);
}

static void TestDirectoryRemove(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(PAL_FileRemoveDirectory(pal, TEST_SUBDIR_FULL_PATH, FALSE)== PAL_Ok);
    CU_ASSERT(PAL_FileRemoveDirectory(pal, TEST_DIR, FALSE) != PAL_Ok);
	CU_ASSERT_EQUAL(PAL_FileExists(pal, TEST_DIR), PAL_Ok);
	PAL_Destroy(pal);
}

static void TestDirectoryRemoveRecursive(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(PAL_FileRemoveDirectory(pal, TEST_DIR, TRUE) == PAL_Ok);
	CU_ASSERT_EQUAL(PAL_FileExists(pal, TEST_DIR), PAL_ErrFileNotExist);
	PAL_Destroy(pal);
}

static void TestFileWrite(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    PAL_File *palFile = NULL;
    uint32 bytesWritten = 0;
    char buf[sizeof(TEST_FILE_STR)];

    CU_ASSERT(PAL_FileOpen(pal, TEST_FILE1_FULL_PATH, PFM_Create, &palFile)
            == PAL_Ok);
    CU_ASSERT(palFile != NULL);

    nsl_strncpy(buf, TEST_FILE_STR, sizeof(buf));
    CU_ASSERT(PAL_FileWrite(palFile, (uint8 *)buf, strlen(buf)+1, &bytesWritten)
            == PAL_Ok);
    CU_ASSERT(bytesWritten == sizeof(TEST_FILE_STR));

    CU_ASSERT(PAL_FileClose(palFile) == PAL_Ok);

    /* create empty file */
    CU_ASSERT(PAL_FileOpen(pal, TEST_FILE2_FULL_PATH, PFM_Create, &palFile)
            == PAL_Ok);
    CU_ASSERT(palFile != NULL);
    CU_ASSERT(PAL_FileClose(palFile) == PAL_Ok);
	PAL_Destroy(pal);
}

static void TestFileRead(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    char buf[sizeof(TEST_FILE_STR)+1];
    uint32 bytesRead = 0;
	uint32 size = sizeof(TEST_FILE_STR);

    PAL_File *palFile = NULL;
    CU_ASSERT(PAL_FileOpen(pal, TEST_FILE1_FULL_PATH, PFM_Read, &palFile)
            == PAL_Ok);
    CU_ASSERT(palFile != NULL);

    CU_ASSERT(PAL_FileRead(palFile, (uint8 *)buf, sizeof(buf), &bytesRead)
            == PAL_Ok);
    CU_ASSERT(bytesRead == size);
    CU_ASSERT_STRING_EQUAL(buf, TEST_FILE_STR);

    CU_ASSERT(PAL_FileClose(palFile) == PAL_Ok);

    palFile = NULL;
    CU_ASSERT(PAL_FileOpen(pal, TEST_FILE2_FULL_PATH, PFM_Read, &palFile)
            == PAL_Ok);
    CU_ASSERT(palFile != NULL);

    CU_ASSERT(PAL_FileRead(palFile, (uint8 *)buf, sizeof(buf), &bytesRead)
            == PAL_Ok);
    CU_ASSERT(bytesRead == 0);

    CU_ASSERT(PAL_FileClose(palFile) == PAL_Ok);
	PAL_Destroy(pal);
}

static void TestFileLoad(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    unsigned char *data = NULL;
    uint32 dataSize = 0;

    CU_ASSERT(PAL_FileLoadFile(pal, TEST_FILE1_FULL_PATH, &data, &dataSize)
            == PAL_Ok);
    CU_ASSERT(dataSize == sizeof(TEST_FILE_STR));
    CU_ASSERT_STRING_EQUAL(data, TEST_FILE_STR);

    nsl_free(data);
	PAL_Destroy(pal);
}

static void TestFileGetSize(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    uint32 fileSize = 0;

    CU_ASSERT(PAL_FileGetSize(pal, TEST_FILE1_FULL_PATH, &fileSize) == PAL_Ok);
    CU_ASSERT(fileSize == sizeof(TEST_FILE_STR));
	PAL_Destroy(pal);
}

static void TestFileGetAttributes(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    uint32 attr= 0;

    CU_ASSERT(PAL_FileGetAttributes(pal, TEST_FILE1_FULL_PATH, &attr)
            == PAL_Ok);
    CU_ASSERT( attr == PAL_FILE_ATTRIBUTE_NORMAL );
    PAL_Destroy(pal);
}

static void TestFileTruncate(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    uint32 fileSize = 0;

    PAL_File *palFile = NULL;
    CU_ASSERT(PAL_FileOpen(pal, TEST_FILE1_FULL_PATH, PFM_ReadWrite, &palFile) == PAL_Ok);
    CU_ASSERT(palFile != NULL);
    CU_ASSERT(PAL_FileTruncate(palFile, 0) == PAL_Ok);
    CU_ASSERT(PAL_FileClose(palFile) == PAL_Ok);
    CU_ASSERT(PAL_FileGetSize(pal, TEST_FILE1_FULL_PATH, &fileSize) == PAL_Ok);
    CU_ASSERT(fileSize == 0);
	PAL_Destroy(pal);
}

static void TestFileRename(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(PAL_FileRename(pal, TEST_FILE1_FULL_PATH,
                TEST_FILE1_NEW_FULL_PATH) == PAL_Ok);
    CU_ASSERT(PAL_FileRename(pal, TEST_FILE1_NEW_FULL_PATH,
                TEST_FILE1_FULL_PATH) == PAL_Ok);
	PAL_Destroy(pal);
}

static void TestFileRemove(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(PAL_FileRemove(pal, TEST_FILE1_FULL_PATH) == PAL_Ok);
	PAL_Destroy(pal);
}

static void TestFileEnum(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    PAL_FileEnum *fileEnum = NULL;
	PAL_FileEnumInfo fileInfo;
	uint32 fileEnumCount = 0;
    CU_ASSERT(PAL_FileEnumerateCreate(pal, TEST_DIR, FALSE, &fileEnum) == PAL_Ok);
    CU_ASSERT(fileEnum != NULL);

	while (PAL_FileEnumerateNext(fileEnum, &fileInfo) == PAL_Ok)
	{
		fileEnumCount++;
	}

    CU_ASSERT(fileEnumCount == 3);
    CU_ASSERT(PAL_FileEnumerateDestroy(fileEnum) == PAL_Ok);
	PAL_Destroy(pal);
}

static void TestFileEnumFilesOnly(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    PAL_FileEnum *fileEnum = NULL;
	PAL_FileEnumInfo fileInfo;
	uint32 fileEnumCount = 0;
    CU_ASSERT(PAL_FileEnumerateCreate(pal, TEST_DIR, TRUE, &fileEnum) == PAL_Ok);
    CU_ASSERT(fileEnum != NULL);

	while (PAL_FileEnumerateNext(fileEnum, &fileInfo) == PAL_Ok)
	{
		fileEnumCount++;
	}

    CU_ASSERT(fileEnumCount == 2);
    CU_ASSERT(PAL_FileEnumerateDestroy(fileEnum) == PAL_Ok);
	PAL_Destroy(pal);
}

static void TestGetFreeSpace(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    uint32 freeSpace = PAL_FileGetFreeSpace(pal);
    CU_ASSERT(freeSpace > 0);

#ifdef UNIT_TEST_VERBOSE_HIGH
    PRINTF("<free space = %u> ", freeSpace);
#endif

	PAL_Destroy(pal);
}

static void TestGetTotalSpace(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    uint32 freeSpace = PAL_FileGetFreeSpace(pal);
    uint32 totalSpace = PAL_FileGetTotalSpace(pal);
    CU_ASSERT(freeSpace > 0);
    CU_ASSERT(totalSpace > 0);
    CU_ASSERT(totalSpace >= freeSpace);

#ifdef UNIT_TEST_VERBOSE_HIGH
    PRINTF("<total space = %u> ", totalSpace);
#endif

	PAL_Destroy(pal);
}

static void TestFileIsDirectory(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(PAL_FileIsDirectory(pal, TEST_DIR));
    CU_ASSERT_FALSE(PAL_FileIsDirectory(pal, TEST_FILE1_FULL_PATH));
	PAL_Destroy(pal);
}

/*! Test path append function */
void 
TestFilePathAppend(void)
{
	PAL_Instance* pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        #define LENGTH 256

        #define ROOT    "c:"
        #define PATH1   "PAL_UNITTEST_TEST"
        #define PATH2   "BLAH"
        #define PATH3   "anotherPath"
        #define FILENAME "SomeFile.txt"

        char path[LENGTH] = "c:";
        char expectedString[LENGTH] = {ROOT DIR_SEPARATOR PATH1 DIR_SEPARATOR PATH2 DIR_SEPARATOR PATH3 DIR_SEPARATOR FILENAME};

        PAL_Error result = PAL_FileAppendPath(pal, path, LENGTH, PATH1);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        result = PAL_FileAppendPath(pal, path, LENGTH, PATH2);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        result = PAL_FileAppendPath(pal, path, LENGTH, PATH3);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        // Now add the file at the end
        result = PAL_FileAppendPath(pal, path, LENGTH, FILENAME);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        CU_ASSERT_STRING_EQUAL(path, expectedString);

	    PAL_Destroy(pal);
    }
}

/*! Test the PAL_FileRemovePath() function. */
void 
TestFilePathRemove(void)
{
	PAL_Instance* pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        #define LENGTH 256

        #define ROOT    "c:"
        #define PATH1   "PAL_UNITTEST_TEST"
        #define PATH2   "BLAH"
        #define PATH3   "anotherPath"
        #define FILENAME "SomeFile.txt"

        char path[LENGTH] = {ROOT DIR_SEPARATOR PATH1 DIR_SEPARATOR PATH2 DIR_SEPARATOR PATH3 DIR_SEPARATOR FILENAME};

        // Remove filename from path
        PAL_Error result = PAL_FileRemovePath(pal, path);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        // Verify new path
        CU_ASSERT_STRING_EQUAL(path, ROOT DIR_SEPARATOR PATH1 DIR_SEPARATOR PATH2 DIR_SEPARATOR PATH3);

        /*
            Remove one subdirectory after another. This also works for files.
        */
        result = PAL_FileRemovePath(pal, path);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        // Verify new path
        CU_ASSERT_STRING_EQUAL(path, ROOT DIR_SEPARATOR PATH1 DIR_SEPARATOR PATH2);

        result = PAL_FileRemovePath(pal, path);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        // Verify new path
        CU_ASSERT_STRING_EQUAL(path, ROOT DIR_SEPARATOR PATH1);

        result = PAL_FileRemovePath(pal, path);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        // Verify root. The root keeps the delimiter
        CU_ASSERT_STRING_EQUAL(path, ROOT DIR_SEPARATOR);

	    PAL_Destroy(pal);
    }
}

/*! Test the PAL_FileGetLastPath() function. */
void
TestFileGetLastPath(void)
{
    PAL_Instance* pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        #define LENGTH 256

        #define ROOT    "c:"
        #define PATH1   "PAL_UNITTEST_TEST"
        #define PATH2   "BLAH"
        #define PATH3   "anotherPath"
        #define FILENAME "SomeFile.txt"

        PAL_Error result = PAL_Ok;
        char* lastPath = NULL;
        char pathWithFilename[LENGTH] = {ROOT DIR_SEPARATOR PATH1 DIR_SEPARATOR PATH2 DIR_SEPARATOR PATH3 DIR_SEPARATOR FILENAME};
        char pathWithDirectory[LENGTH] = {ROOT DIR_SEPARATOR PATH1 DIR_SEPARATOR PATH2 DIR_SEPARATOR};
        char wrongPath[LENGTH] = {ROOT DIR_SEPARATOR PATH1 DIR_SEPARATOR DIR_SEPARATOR};

        // Get filename from path
        result = PAL_FileGetLastPath(pal, pathWithFilename, &lastPath);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        if (result == PAL_Ok)
        {
            // Verify filename
            CU_ASSERT_STRING_EQUAL(lastPath, FILENAME);
            nsl_free(lastPath);
            lastPath = NULL;
        }

        // Get directory from path
        result = PAL_FileGetLastPath(pal, pathWithDirectory, &lastPath);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        if (result == PAL_Ok)
        {
            // Verify directory
            CU_ASSERT_STRING_EQUAL(lastPath, PATH2);
            nsl_free(lastPath);
            lastPath = NULL;
        }

        // Test with wrong path
        result = PAL_FileGetLastPath(pal, wrongPath, &lastPath);
        CU_ASSERT_EQUAL(result, PAL_ErrBadParam);

	    PAL_Destroy(pal);
    }
}

/*! Test PAL_FileCreateDirectoryEx() function */
void 
TestFileCreateDirectoryEx(void)
{
	PAL_Instance* pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        #define LENGTH 256

        #define PATH1   "PAL_UNITTEST_TEST"
        #define PATH2   "BLAH"
        #define PATH3   "anotherPath"

        char path[LENGTH] = {PATH1 DIR_SEPARATOR PATH2 DIR_SEPARATOR PATH3};

        // Create all sub-directories
        PAL_Error result = PAL_FileCreateDirectoryEx(pal, path);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        // Validate that all subdirectories exist
        result = PAL_FileExists(pal, PATH1);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        result = PAL_FileExists(pal, PATH1 DIR_SEPARATOR PATH2);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        result = PAL_FileExists(pal, path);
        CU_ASSERT_EQUAL(result, PAL_Ok);

        // Cleanup everything again
        result = PAL_FileRemoveDirectory(pal, PATH1, TRUE);
        CU_ASSERT_EQUAL(result, PAL_Ok);

	    PAL_Destroy(pal);
    }
}


/*! @} */


