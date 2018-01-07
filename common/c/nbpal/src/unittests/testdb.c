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

#include "testdb.h"
#include "main.h"
#include "paldb.h"
#include "palfile.h"
#include "paldberror.h"
#include "PlatformUtil.h"

static const char* g_testdata = "testdata";

static const char* g_lpstrTestDBName = "testdb.dat";
static const char* g_lpstrSQL_DropTable = "DROP TABLE IF EXISTS 'TestTable';";
static const char* g_lpstrSQL_ClearTable = "DELETE FROM 'TestTable' WHERE 1;";
static const char* g_lpstrSQL_CreateTable = "CREATE TABLE 'TestTable' ('blob' BLOB, 'string' VARCHAR, 'integer' INTEGER, 'double' DOUBLE);";
static const char* g_lpstrSQL_CreateTable2 = "CREATE TABLE 'TestTable' ('blob' BLOB, 'string' VARCHAR, 'integer' INTEGER, 'long' LONG, 'double' DOUBLE);";
static const char* g_lpstrSQL_InsertData = "INSERT INTO TestTable VALUES(@blob, @string, @integer, @double);";
static const char* g_lpstrSQL_GetDouble = "SELECT double FROM TestTable WHERE double = @double;";
static const char* g_lpstrSQL_GetInt = "SELECT integer FROM TestTable WHERE integer = @integer;";
static const char* g_lpstrSQL_GetString = "SELECT string FROM TestTable WHERE string = @string;";
static const char* g_lpstrSQL_GetBlob = "SELECT blob FROM TestTable WHERE blob = @blob;";
static const char* g_lpstrSQL_GetReader = "SELECT * FROM TestTable WHERE rowid = 1;";

// System test functions
static void TestPalDbOpenWithoutPal(void);
static void TestPalDbOpenWithoutName(void);
static void TestPalDbOpenWithoutConnect(void);
static void TestPalDbOpen(void);
static void TestPalDbCloseWithoutConnect(void);
static void TestPalDbClose(void);
static void TestPalDbGetLastErrorWithoutConnect(void);
static void TestPalDbGetLastErrorWithoutError(void);
static void TestPalDbGetLastError(void);
static void TestPalDbExecuteNonQueryWithoutConnect(void);
static void TestPalDbExecuteNonQueryWithoutCommand(void);
static void TestPalDbExecuteNonQuery(void);
static void TestPalDbExecuteNonQueryCommandWithoutCommand(void);
static void TestPalDbExecuteNonQueryCommand(void);
static void TestPalDbInitCommandWithoutConnect(void);
static void TestPalDbInitCommandWithoutSqlCommand(void);
static void TestPalDbInitCommandWithoutDbCommand(void);
static void TestPalDbInitCommand(void);
static void TestPalDbCommandStepWithoutCommand(void);
static void TestPalDbCommandStepWithoutReader(void);
static void TestPalDbCommandStep(void);
static void TestPalDbCommandFinalizeWithoutCommand(void);
static void TestPalDbCommandFinalizeWithoutReader(void);
static void TestPalDbCommandFinalize(void);
static void TestPalDbBindDoubleParamWithoutCommand(void);
static void TestPalDbBindDoubleParamWithoutParamName(void);
static void TestPalDbBindDoubleParam(void);
static void TestPalDbBindIntParamWithoutCommand(void);
static void TestPalDbBindIntParamWithoutParamName(void);
static void TestPalDbBindIntParam(void);
static void TestPalDbBindLongParamWithoutCommand(void);
static void TestPalDbBindLongParamWithoutParamName(void);
static void TestPalDbBindLongParam(void);
static void TestPalDbBindStringParamWithoutCommand(void);
static void TestPalDbBindStringParamWithoutParamName(void);
static void TestPalDbBindStringParam(void);
static void TestPalDbBindBlobParamWithoutCommand(void);
static void TestPalDbBindBlobParamWithoutParamName(void);
static void TestPalDbBindBlobParam(void);
static void TestPalDbExecuteDoubleWithoutCommand(void);
static void TestPalDbExecuteDoubleWithoutResult(void);
static void TestPalDbExecuteDouble(void);
static void TestPalDbExecuteIntWithoutCommand(void);
static void TestPalDbExecuteIntWithoutResult(void);
static void TestPalDbExecuteInt(void);
static void TestPalDbExecuteStringWithoutCommand(void);
static void TestPalDbExecuteStringWithoutResult(void);
static void TestPalDbExecuteString(void);
static void TestPalDbExecuteBlobExWithoutCommand(void);
static void TestPalDbExecuteBlobExWithoutResult(void);
static void TestPalDbExecuteBlobExWithoutLength(void);
static void TestPalDbExecuteBlobEx(void);
static void TestPalDbExecuteReaderWithoutCommand(void);
static void TestPalDbExecuteReaderWithoutReader(void);
static void TestPalDbExecuteReader(void);
static void TestPalDbResetReaderWithoutReader(void);
static void TestPalDbResetReader(void);
static void TestPalDbReaderReadWithoutReader(void);
static void TestPalDbReaderRead(void);
static void TestPalDbReaderGetIntWithoutReader(void);
static void TestPalDbReaderGetIntWithoutColumnName(void);
static void TestPalDbReaderGetIntWithoutResult(void);
static void TestPalDbReaderGetInt(void);
static void TestPalDbReaderGetDoubleWithoutReader(void);
static void TestPalDbReaderGetDoubleWithoutColumnName(void);
static void TestPalDbReaderGetDoubleWithoutResult(void);
static void TestPalDbReaderGetDouble(void);
static void TestPalDbReaderGetStringWithoutReader(void);
static void TestPalDbReaderGetStringWithoutColumnName(void);
static void TestPalDbReaderGetStringWithoutResult(void);
static void TestPalDbReaderGetString(void);
static void TestPalDbReaderGetBlobWithoutReader(void);
static void TestPalDbReaderGetBlobWithoutColumnName(void);
static void TestPalDbReaderGetBlobWithoutResult(void);
static void TestPalDbReaderGetBlobWithoutLength(void);
static void TestPalDbReaderGetBlob(void);

static void TestPalDatabase(void);

// local functions
static PAL_DB_Error CreateDbConnect(PAL_Instance** pPal, PAL_DBConnect** pConnect);
static PAL_DB_Error DbConnectDestroy(PAL_Instance** pPal, PAL_DBConnect** pConnect);
static PAL_DB_Error InsertDataInTable(PAL_DBConnect* pConnect);
static nb_boolean PrepareTableInDb(PAL_Instance **pal, PAL_DBConnect** pConnect, PAL_DBCommand** pCommand);

/*! Add all your test functions here

@return None
*/
void
TestDB_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestPalDbOpenWithoutPal", &TestPalDbOpenWithoutPal);
    CU_add_test(pTestSuite, "TestPalDbOpenWithoutName", &TestPalDbOpenWithoutName);
    CU_add_test(pTestSuite, "TestPalDbOpenWithoutConnect", &TestPalDbOpenWithoutConnect);
    CU_add_test(pTestSuite, "TestPalDbOpen", &TestPalDbOpen);
    CU_add_test(pTestSuite, "TestPalDbCloseWithoutConnect", &TestPalDbCloseWithoutConnect);
    CU_add_test(pTestSuite, "TestPalDbClose", &TestPalDbClose);
    CU_add_test(pTestSuite, "TestPalDbGetLastErrorWithoutConnect", &TestPalDbGetLastErrorWithoutConnect);
    CU_add_test(pTestSuite, "TestPalDbGetLastErrorWithoutError", &TestPalDbGetLastErrorWithoutError);
    CU_add_test(pTestSuite, "TestPalDbGetLastError", &TestPalDbGetLastError);
    CU_add_test(pTestSuite, "TestPalDbExecuteNonQueryWithoutConnect", &TestPalDbExecuteNonQueryWithoutConnect);
    CU_add_test(pTestSuite, "TestPalDbExecuteNonQueryWithoutCommand", &TestPalDbExecuteNonQueryWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbExecuteNonQuery", &TestPalDbExecuteNonQuery);
    CU_add_test(pTestSuite, "TestPalDbExecuteNonQueryCommandWithoutCommand", &TestPalDbExecuteNonQueryCommandWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbExecuteNonQueryCommand", &TestPalDbExecuteNonQueryCommand);
    CU_add_test(pTestSuite, "TestPalDbInitCommandWithoutConnect", &TestPalDbInitCommandWithoutConnect);
    CU_add_test(pTestSuite, "TestPalDbInitCommandWithoutSqlCommand", &TestPalDbInitCommandWithoutSqlCommand);
    CU_add_test(pTestSuite, "TestPalDbInitCommandWithoutDbCommand", &TestPalDbInitCommandWithoutDbCommand);
    CU_add_test(pTestSuite, "TestPalDbInitCommand", &TestPalDbInitCommand);
    CU_add_test(pTestSuite, "TestPalDbCommandStepWithoutCommand", &TestPalDbCommandStepWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbCommandStepWithoutReader", &TestPalDbCommandStepWithoutReader);
    CU_add_test(pTestSuite, "TestPalDbCommandStep", &TestPalDbCommandStep);
    CU_add_test(pTestSuite, "TestPalDbCommandFinalizeWithoutCommand", &TestPalDbCommandFinalizeWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbCommandFinalizeWithoutReader", &TestPalDbCommandFinalizeWithoutReader);
    CU_add_test(pTestSuite, "TestPalDbCommandFinalize", &TestPalDbCommandFinalize);
    CU_add_test(pTestSuite, "TestPalDbBindDoubleParamWithoutCommand", &TestPalDbBindDoubleParamWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbBindDoubleParamWithoutParamName", &TestPalDbBindDoubleParamWithoutParamName);
    CU_add_test(pTestSuite, "TestPalDbBindDoubleParam", &TestPalDbBindDoubleParam);
    CU_add_test(pTestSuite, "TestPalDbBindIntParamWithoutCommand", &TestPalDbBindIntParamWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbBindIntParamWithoutParamName", &TestPalDbBindIntParamWithoutParamName);
    CU_add_test(pTestSuite, "TestPalDbBindIntParam", &TestPalDbBindIntParam);
    CU_add_test(pTestSuite, "TestPalDbBindLongParamWithoutCommand", &TestPalDbBindLongParamWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbBindLongParamWithoutParamName", &TestPalDbBindLongParamWithoutParamName);
    CU_add_test(pTestSuite, "TestPalDbBindLongParam", &TestPalDbBindLongParam);
    CU_add_test(pTestSuite, "TestPalDbBindStringParamWithoutCommand", &TestPalDbBindStringParamWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbBindStringParamWithoutParamName", &TestPalDbBindStringParamWithoutParamName);
    CU_add_test(pTestSuite, "TestPalDbBindStringParam", &TestPalDbBindStringParam);
    CU_add_test(pTestSuite, "TestPalDbBindBlobParamWithoutCommand", &TestPalDbBindBlobParamWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbBindBlobParamWithoutParamName", &TestPalDbBindBlobParamWithoutParamName);
    CU_add_test(pTestSuite, "TestPalDbBindBlobParam", &TestPalDbBindBlobParam);
    CU_add_test(pTestSuite, "TestPalDbExecuteDoubleWithoutCommand", &TestPalDbExecuteDoubleWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbExecuteDoubleWithoutResult", &TestPalDbExecuteDoubleWithoutResult);
    CU_add_test(pTestSuite, "TestPalDbExecuteDouble", &TestPalDbExecuteDouble);
    CU_add_test(pTestSuite, "TestPalDbExecuteIntWithoutCommand", &TestPalDbExecuteIntWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbExecuteIntWithoutResult", &TestPalDbExecuteIntWithoutResult);
    CU_add_test(pTestSuite, "TestPalDbExecuteInt", &TestPalDbExecuteInt);
    CU_add_test(pTestSuite, "TestPalDbExecuteStringWithoutCommand", &TestPalDbExecuteStringWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbExecuteStringWithoutResult", &TestPalDbExecuteStringWithoutResult);
    CU_add_test(pTestSuite, "TestPalDbExecuteString", &TestPalDbExecuteString);
    CU_add_test(pTestSuite, "TestPalDbExecuteBlobExWithoutCommand", &TestPalDbExecuteBlobExWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbExecuteBlobExWithoutResult", &TestPalDbExecuteBlobExWithoutResult);
    CU_add_test(pTestSuite, "TestPalDbExecuteBlobExWithoutLength", &TestPalDbExecuteBlobExWithoutLength);
    CU_add_test(pTestSuite, "TestPalDbExecuteBlobEx", &TestPalDbExecuteBlobEx);
    CU_add_test(pTestSuite, "TestPalDbExecuteReaderWithoutCommand", &TestPalDbExecuteReaderWithoutCommand);
    CU_add_test(pTestSuite, "TestPalDbExecuteReaderWithoutReader", &TestPalDbExecuteReaderWithoutReader);
    CU_add_test(pTestSuite, "TestPalDbExecuteReader", &TestPalDbExecuteReader);
    CU_add_test(pTestSuite, "TestPalDbResetReaderWithoutReader", &TestPalDbResetReaderWithoutReader);
    CU_add_test(pTestSuite, "TestPalDbResetReader", &TestPalDbResetReader);
    CU_add_test(pTestSuite, "TestPalDbReaderReadWithoutReader", &TestPalDbReaderReadWithoutReader);
    CU_add_test(pTestSuite, "TestPalDbReaderRead", &TestPalDbReaderRead);
    CU_add_test(pTestSuite, "TestPalDbReaderGetIntWithoutReader", &TestPalDbReaderGetIntWithoutReader);
    CU_add_test(pTestSuite, "TestPalDbReaderGetIntWithoutColumnName", &TestPalDbReaderGetIntWithoutColumnName);
    CU_add_test(pTestSuite, "TestPalDbReaderGetIntWithoutResult", &TestPalDbReaderGetIntWithoutResult);
    CU_add_test(pTestSuite, "TestPalDbReaderGetInt", &TestPalDbReaderGetInt);
    CU_add_test(pTestSuite, "TestPalDbReaderGetDoubleWithoutReader", &TestPalDbReaderGetDoubleWithoutReader);
    CU_add_test(pTestSuite, "TestPalDbReaderGetDoubleWithoutColumnName", &TestPalDbReaderGetDoubleWithoutColumnName);
    CU_add_test(pTestSuite, "TestPalDbReaderGetDoubleWithoutResult", &TestPalDbReaderGetDoubleWithoutResult);
    CU_add_test(pTestSuite, "TestPalDbReaderGetDouble", &TestPalDbReaderGetDouble);
    CU_add_test(pTestSuite, "TestPalDbReaderGetStringWithoutReader", &TestPalDbReaderGetStringWithoutReader);
    CU_add_test(pTestSuite, "TestPalDbReaderGetStringWithoutColumnName", &TestPalDbReaderGetStringWithoutColumnName);
    CU_add_test(pTestSuite, "TestPalDbReaderGetStringWithoutResult", &TestPalDbReaderGetStringWithoutResult);
    CU_add_test(pTestSuite, "TestPalDbReaderGetString", &TestPalDbReaderGetString);
    CU_add_test(pTestSuite, "TestPalDbReaderGetBlobWithoutReader", &TestPalDbReaderGetBlobWithoutReader);
    CU_add_test(pTestSuite, "TestPalDbReaderGetBlobWithoutColumnName", &TestPalDbReaderGetBlobWithoutColumnName);
    CU_add_test(pTestSuite, "TestPalDbReaderGetBlobWithoutResult", &TestPalDbReaderGetBlobWithoutResult);
    CU_add_test(pTestSuite, "TestPalDbReaderGetBlobWithoutLength", &TestPalDbReaderGetBlobWithoutLength);
    CU_add_test(pTestSuite, "TestPalDbReaderGetBlob", &TestPalDbReaderGetBlob);
    CU_add_test(pTestSuite, "TestPalDataBase", &TestPalDatabase);
};

/*! Add common initialization code here.

@return 0

@see TestFile_SuiteCleanup
*/
int
TestDB_SuiteSetup()
{
    return 0;
}

/*! Add common cleanup code here.

@return 0

@see TestFile_SuiteSetup
*/
int
TestDB_SuiteCleanup()
{
    return 0;
}

PAL_DB_Error
CreateDbConnect(PAL_Instance** pPal, PAL_DBConnect** pConnect)
{
    PAL_Instance  *pal = NULL;
    PAL_DBConnect *connect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    if(pPal == NULL || pConnect == NULL)
    {
        return PAL_DB_ERROR;
    }

    pal = PAL_CreateInstance();
    if (pal == NULL)
    {
        return PAL_DB_ERROR;
    }

    result = PAL_DBOpen(pal, PAL_DB_Sqlite, g_lpstrTestDBName, &connect);
    if(result != PAL_DB_OK || connect == NULL)
    {
        PAL_Destroy(pal);
        return PAL_DB_ERROR;
    }

    *pPal = pal;
    *pConnect = connect;

    return PAL_DB_OK;
}

PAL_DB_Error
DbConnectDestroy(PAL_Instance** pPal, PAL_DBConnect** pConnect)
{
    if ((pPal == NULL)||(pConnect == NULL))
    {
        return PAL_DB_ERROR;
    }

    if (*pPal != NULL)
    {
        PAL_Destroy(*pPal);
    }

    if (*pConnect != NULL)
    {
        PAL_DBClose(*pConnect);
    }

    return PAL_DB_OK;
}

PAL_DB_Error
InsertDataInTable(PAL_DBConnect* pConnect)
{
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    if(pConnect == NULL)
    {
        return PAL_DB_ERROR;
    }

    // clear old table
    result = PAL_DBInitCommand(pConnect, g_lpstrSQL_DropTable, &pCommand);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    result = PAL_DBExecuteNonQueryCommand(pCommand);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    result = PAL_DBCommandFinalize(pCommand, NULL);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    // create table
    pCommand = NULL;
    result = PAL_DBInitCommand(pConnect, g_lpstrSQL_CreateTable, &pCommand);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    result = PAL_DBExecuteNonQueryCommand(pCommand);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    result = PAL_DBCommandFinalize(pCommand, NULL);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    // insert data
    pCommand = NULL;
    result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    result = PAL_DBExecuteNonQueryCommand(pCommand);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    result = PAL_DBCommandFinalize(pCommand, NULL);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    return result;
}

nb_boolean PrepareTableInDb(PAL_Instance **pal, PAL_DBConnect** pConnect, PAL_DBCommand** pCommand)
{
    PAL_DB_Error result = PAL_DB_OK;

    if ((pal == NULL)||(pConnect == NULL)||(pCommand == NULL))
    {
        return FALSE;
    }

    result = CreateDbConnect(pal, pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if (result != PAL_DB_OK)
    {
        return FALSE;
    }

    result = InsertDataInTable(*pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    result = PAL_DBInitCommand(*pConnect, g_lpstrSQL_ClearTable, pCommand);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
    CU_ASSERT_PTR_NOT_NULL(*pCommand);

    result = PAL_DBExecuteNonQueryCommand(*pCommand);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    result = PAL_DBCommandFinalize(*pCommand, NULL);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if (result == PAL_DB_OK)
    {
        return TRUE;
    }

    return FALSE;
}

void
TestPalDbOpenWithoutPal(void)
{
    PAL_DB_Error result = PAL_DB_OK;
    PAL_DBConnect* pConnect = NULL;

    result = PAL_DBOpen(NULL, PAL_DB_Sqlite, g_lpstrTestDBName, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        PAL_DBClose(pConnect);
    }
}

void
TestPalDbOpenWithoutName(void)
{
    PAL_DB_Error result = PAL_DB_OK;
    PAL_Instance *pal = PAL_CreateInstance();
    PAL_DBConnect* pConnect = NULL;

    CU_ASSERT_PTR_NOT_NULL(pal);

    if(pal)
    {
        result = PAL_DBOpen(pal, PAL_DB_Sqlite, NULL, &pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    if(pConnect)
    {
        PAL_DBClose(pConnect);
    }

    if(pal)
    {
        PAL_Destroy(pal);
    }
}

void
TestPalDbOpenWithoutConnect(void)
{
    PAL_DB_Error result = PAL_DB_OK;
    PAL_Instance *pal = PAL_CreateInstance();

    CU_ASSERT_PTR_NOT_NULL(pal);

    if(pal)
    {
        result = PAL_DBOpen(pal, PAL_DB_Sqlite, g_lpstrTestDBName, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    if(pal)
    {
        PAL_Destroy(pal);
    }
}

void
TestPalDbOpen(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbCloseWithoutConnect(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    result = PAL_DBClose(NULL);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbClose(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBClose(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    if(pal)
    {
        PAL_Destroy(pal);
    }
}

void
TestPalDbGetLastErrorWithoutConnect(void)
{
    PAL_DB_Error result = PAL_DB_OK;
    int lastError = SQLITE_OK;

    result = PAL_DBGetLastError(NULL, &lastError);
    CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
}

void
TestPalDbGetLastErrorWithoutError(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBGetLastError(pConnect, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbGetLastError(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;
    int lastError = SQLITE_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBGetLastError(pConnect, &lastError);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteNonQueryWithoutConnect(void)
{
    PAL_DB_Error result = PAL_DB_OK;

    result = PAL_DBExecuteNonQuery(NULL, g_lpstrSQL_DropTable);
    CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
}

void
TestPalDbExecuteNonQueryWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBExecuteNonQuery(pConnect, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteNonQuery(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBExecuteNonQuery(pConnect, g_lpstrSQL_DropTable);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteNonQueryCommandWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBExecuteNonQueryCommand(NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteNonQueryCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_DropTable, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbInitCommandWithoutConnect(void)
{
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;


    result = PAL_DBInitCommand(NULL, g_lpstrSQL_DropTable, &pCommand);
    CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

    if(pCommand)
    {
        PAL_DBCommandFinalize(pCommand, NULL);
    }
}

void
TestPalDbInitCommandWithoutSqlCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBInitCommand(pConnect, NULL, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

        if(pCommand)
        {
            PAL_DBCommandFinalize(pCommand, NULL);
        }
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbInitCommandWithoutDbCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_DropTable, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbInitCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_DropTable, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbCommandStepWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_DropTable, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBCommandStep(NULL, pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

            PAL_DBCommandFinalize(pCommand, pReader);
        }
        else
        {
            PAL_DBCommandFinalize(pCommand, NULL);
        }
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbCommandStepWithoutReader(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_DropTable, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBCommandStep(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbCommandStep(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_DropTable, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBCommandStep(pCommand, pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

            PAL_DBCommandFinalize(pCommand, pReader);
        }
        else
        {
            PAL_DBCommandFinalize(pCommand, NULL);
        }
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbCommandFinalizeWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_DropTable, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBCommandFinalize(pCommand, pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }
        else
        {
            result = PAL_DBCommandFinalize(pCommand, NULL);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbCommandFinalizeWithoutReader(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_DropTable, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbCommandFinalize(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_DropTable, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBCommandFinalize(pCommand, pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }
        else
        {
            result = PAL_DBCommandFinalize(pCommand, NULL);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindDoubleParamWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const double* testDouble = (double*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBBindDoubleParam(NULL, "@double", *testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindDoubleParamWithoutParamName(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const double* testDouble = (double*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetDouble, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, NULL, *testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_RANGE);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindDoubleParam(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const double* testDouble = (double*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetDouble, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, "@double", *testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindIntParamWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int* testInt = (int*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBBindIntParam(NULL, "@integer", *testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindIntParamWithoutParamName(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int* testInt = (int*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, NULL, *testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_RANGE);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindIntParam(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int* testInt = (int*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, "@integer", *testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindLongParamWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int64* testLong = (int64*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBBindLongParam(NULL, "@long", *testLong);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindLongParamWithoutParamName(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int64* testLong = (int64*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetDouble, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindLongParam(pCommand, NULL, *testLong);
        CU_ASSERT_EQUAL(result, PAL_DB_RANGE);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindLongParam(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int64* testLong = (int64*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindLongParam(pCommand, "@long", *testLong);
        CU_ASSERT_EQUAL(result, PAL_DB_RANGE); // Long is not name of column in table so result = PAL_DB_RANGE

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindStringParamWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBBindStringParam(NULL, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindStringParamWithoutParamName(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetString, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, NULL, g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_RANGE);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindStringParam(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetString, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindBlobParamWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBBindBlobParam(NULL, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindBlobParamWithoutParamName(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetBlob, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, NULL, (void*)g_testdata, (int)strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_RANGE);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbBindBlobParam(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetBlob, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        PAL_DBCommandFinalize(pCommand, NULL);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteDoubleWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const double *testDouble = (double*)g_testdata;
    double resultDouble;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetDouble, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, "@double", *testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetDouble, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, "@double", *testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteDouble(NULL, &resultDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteDoubleWithoutResult(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const double *testDouble = (double*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, "@double", *testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetDouble, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, "@double", *testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteDouble(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteDouble(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const double testDouble = 100.456;
    double resultDouble = 0;

    if(PrepareTableInDb(&pal, &pConnect, &pCommand))
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, "@double", testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetDouble, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, "@double", testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteDouble(pCommand, &resultDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        CU_ASSERT_EQUAL(resultDouble, testDouble);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteIntWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int *testInt = (int*)g_testdata;
    int resultInt = 0;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetInt, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, "@integer", *testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetInt, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, "@integer", *testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteInt(NULL, &resultInt);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteIntWithoutResult(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int *testInt = (int*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, "@integer", *testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetInt, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, "@integer", *testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteInt(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteInt(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int testInt = 45646;
    int resultInt = 0;

    if(PrepareTableInDb(&pal, &pConnect, &pCommand))
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, "@integer", testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetInt, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, "@integer", testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteInt(pCommand, &resultInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        CU_ASSERT_EQUAL(resultInt, testInt);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteStringWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    char* resultString = NULL;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetString, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetString, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteString(NULL, &resultString);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteStringWithoutResult(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    if(PrepareTableInDb(&pal, &pConnect, &pCommand))
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetString, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteString(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteString(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    char* resultString = NULL;

    if(PrepareTableInDb(&pal, &pConnect, &pCommand))
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetString, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteString(pCommand, &resultString);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        CU_ASSERT_EQUAL(nsl_strcmp(resultString, g_testdata), 0);
        nsl_free(resultString);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteBlobExWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    char* resultString = NULL;
    int len;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetBlob, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetBlob, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteBlobEx(NULL, (byte**)&resultString, &len);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
        //nsl_free(resultString);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteBlobExWithoutResult(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    int len = 0;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetBlob, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteBlobEx(pCommand, NULL, &len);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteBlobExWithoutLength(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    char* resultString = NULL;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteBlobEx(pCommand, (byte**)&resultString, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
        nsl_free(resultString);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteBlobEx(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    byte* resultString = NULL;
    int len = 0;

    if (PrepareTableInDb(&pal, &pConnect, &pCommand))
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetBlob, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteBlobEx(pCommand, &resultString, &len);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_EQUAL(len, (int)strlen(g_testdata));
        CU_ASSERT_EQUAL(nsl_memcmp((char*)resultString, g_testdata, len), 0);
        nsl_free(resultString);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteReaderWithoutCommand(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteReader(NULL, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

        if(pReader)
        {
            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);

            result = PAL_DBCommandFinalize(NULL, pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteReaderWithoutReader(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbExecuteReader(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        result = PAL_DBResetReader(pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbResetReaderWithoutReader(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBResetReader(NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbResetReader(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderReadWithoutReader(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = PAL_DBReaderRead(NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderRead(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;


    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetIntWithoutReader(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int* testInt = (int*)g_testdata;
    int resultInt;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetInt, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, "@integer", *testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetInt(NULL, "integer", &resultInt);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetIntWithoutColumnName(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int* testInt = (int*)g_testdata;
    int resultInt;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetInt, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, "@integer", *testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetInt(pReader, NULL, &resultInt);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetIntWithoutResult(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int* testInt = (int*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, "@integer", *testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetInt(pReader, "integer", NULL);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetInt(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const int* testInt = (int*)g_testdata;
    int resultInt = 0;

    if(PrepareTableInDb(&pal, &pConnect, &pCommand))
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindIntParam(pCommand, "@integer", *testInt);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetInt(pReader, "integer", &resultInt);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
            CU_ASSERT_EQUAL(resultInt, *testInt);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetDoubleWithoutReader(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const double* testDouble = (double*)g_testdata;
    double resultDouble;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetDouble, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, "@double", *testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetdouble(NULL, "double", &resultDouble);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetDoubleWithoutColumnName(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const double* testDouble = (double*)g_testdata;
    double resultDouble;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, "@double", *testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetdouble(pReader, NULL, &resultDouble);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetDoubleWithoutResult(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const double* testDouble = (double*)g_testdata;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, "@double", *testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetdouble(pReader, "double", NULL);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetDouble(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    const double* testDouble = (double*)g_testdata;
    double resultDouble;

    if(PrepareTableInDb(&pal, &pConnect, &pCommand))
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindDoubleParam(pCommand, "@double", *testDouble);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetdouble(pReader, "double", &resultDouble);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
            CU_ASSERT_EQUAL(resultDouble, *testDouble);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetStringWithoutReader(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    char* resultString = NULL;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetString, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetstring(NULL, "string", &resultString);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
            nsl_free(resultString);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetStringWithoutColumnName(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    char* resultString = NULL;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetstring(pReader, NULL, &resultString);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
            nsl_free(resultString);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetStringWithoutResult(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetstring(pReader, "string", NULL);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetString(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    char* resultString = NULL;

    if(PrepareTableInDb(&pal, &pConnect, &pCommand))
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindStringParam(pCommand, "@string", g_testdata);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetstring(pReader, "string", &resultString);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
            CU_ASSERT_EQUAL(nsl_strcmp(resultString, g_testdata), 0);
            nsl_free(resultString);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetBlobWithoutReader(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    char* resultString = NULL;
    int len;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetBlob, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetblob(NULL, "blob", (byte**)&resultString, &len);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
            nsl_free(resultString);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetBlobWithoutColumnName(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    char* resultString = NULL;
    int len = 0;

    result = CreateDbConnect(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);

    if(pConnect)
    {
        result = InsertDataInTable(pConnect);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetblob(pReader, NULL, (byte**)&resultString, &len);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
            nsl_free(resultString);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetBlobWithoutResult(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    int len = 0;

    if(PrepareTableInDb(&pal, &pConnect, &pCommand))
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetblob(pReader, "blob", NULL, &len);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetBlobWithoutLength(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    char* resultString = NULL;

    if(PrepareTableInDb(&pal, &pConnect, &pCommand))
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetblob(pReader, "blob", (byte**)&resultString, NULL);
            CU_ASSERT_EQUAL(result, PAL_DB_ERROR);
            nsl_free(resultString);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDbReaderGetBlob(void)
{
    PAL_Instance* pal = NULL;
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;
    PAL_DB_Error result = PAL_DB_OK;

    char* resultString = NULL;
    int len = 0;

    if(PrepareTableInDb(&pal, &pConnect, &pCommand))
    {
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata));
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBExecuteNonQueryCommand(pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        result = PAL_DBCommandFinalize(pCommand, NULL);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);

        pCommand = NULL;
        result = PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pCommand);

        result = PAL_DBExecuteReader(pCommand, &pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
        CU_ASSERT_PTR_NOT_NULL(pReader);

        if(pReader)
        {
            result = PAL_DBReaderRead(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_ROW);

            result = PAL_DBReaderGetblob(pReader, "blob", (byte**)&resultString, &len);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
            CU_ASSERT_EQUAL(len, (int)strlen(g_testdata));
            CU_ASSERT_EQUAL(memcmp(resultString, g_testdata, len), 0);
            nsl_free(resultString);

            result = PAL_DBResetReader(pReader);
            CU_ASSERT_EQUAL(result, PAL_DB_OK);
        }

        result = PAL_DBCommandFinalize(pCommand, pReader);
        CU_ASSERT_EQUAL(result, PAL_DB_OK);
    }

    result = DbConnectDestroy(&pal, &pConnect);
    CU_ASSERT_EQUAL(result, PAL_DB_OK);
}

void
TestPalDatabase(void)
{
    PAL_DBConnect* pConnect = NULL;
    PAL_DBCommand* pCommand = NULL;
    PAL_DBReader* pReader = NULL;

    const double* testDouble = (double*)g_testdata;
    const int* testInt = (int*)g_testdata;
    double resultDouble;
    int resultInt;
    char* resultString = NULL;
    int len;

    PAL_Instance *pal = PAL_CreateInstance();

    CU_ASSERT(PAL_DBOpen(pal, PAL_DB_Sqlite, g_lpstrTestDBName, &pConnect) == PAL_DB_OK);

    // clear old table
    CU_ASSERT(PAL_DBInitCommand(pConnect, g_lpstrSQL_DropTable, &pCommand) == PAL_DB_OK);
    CU_ASSERT(PAL_DBExecuteNonQueryCommand(pCommand) == PAL_DB_OK);
    CU_ASSERT(PAL_DBCommandFinalize(pCommand, NULL) == PAL_DB_OK);

    // create table
    pCommand = NULL;
    CU_ASSERT(PAL_DBInitCommand(pConnect, g_lpstrSQL_CreateTable, &pCommand) == PAL_DB_OK);
    CU_ASSERT(PAL_DBExecuteNonQueryCommand(pCommand) == PAL_DB_OK);
    CU_ASSERT(PAL_DBCommandFinalize(pCommand, NULL) == PAL_DB_OK);

    // insert data
    pCommand = NULL;
    CU_ASSERT(PAL_DBInitCommand(pConnect, g_lpstrSQL_InsertData, &pCommand) == PAL_DB_OK);
    CU_ASSERT(PAL_DBBindDoubleParam(pCommand, "@double", *testDouble) == PAL_DB_OK);
    CU_ASSERT(PAL_DBBindIntParam(pCommand, "@integer", *testInt) == PAL_DB_OK);
    CU_ASSERT(PAL_DBBindStringParam(pCommand, "@string", g_testdata) == PAL_DB_OK);
    CU_ASSERT(PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata)) == PAL_DB_OK);
    CU_ASSERT(PAL_DBExecuteNonQueryCommand(pCommand) == PAL_DB_OK);
    CU_ASSERT(PAL_DBCommandFinalize(pCommand, NULL) == PAL_DB_OK);

    pCommand = NULL;
    CU_ASSERT(PAL_DBInitCommand(pConnect, g_lpstrSQL_GetDouble, &pCommand) == PAL_DB_OK);
    CU_ASSERT(PAL_DBBindDoubleParam(pCommand, "@double", *testDouble) == PAL_DB_OK);
    CU_ASSERT(PAL_DBExecuteDouble(pCommand, &resultDouble) == PAL_DB_OK);
    CU_ASSERT(resultDouble == *testDouble);
    CU_ASSERT(PAL_DBCommandFinalize(pCommand, NULL) == PAL_DB_OK);

    pCommand = NULL;
    CU_ASSERT(PAL_DBInitCommand(pConnect, g_lpstrSQL_GetInt, &pCommand) == PAL_DB_OK);
    CU_ASSERT(PAL_DBBindIntParam(pCommand, "@integer", *testInt) == PAL_DB_OK);
    CU_ASSERT(PAL_DBExecuteInt(pCommand, &resultInt) == PAL_DB_OK);
    CU_ASSERT(resultInt == *testInt);
    CU_ASSERT(PAL_DBCommandFinalize(pCommand, NULL) == PAL_DB_OK);

    pCommand = NULL;
    CU_ASSERT(PAL_DBInitCommand(pConnect, g_lpstrSQL_GetString, &pCommand) == PAL_DB_OK);
    CU_ASSERT(PAL_DBBindStringParam(pCommand, "@string", g_testdata) == PAL_DB_OK);
    CU_ASSERT(PAL_DBExecuteString(pCommand, &resultString) == PAL_DB_OK);
    CU_ASSERT(strcmp(resultString, g_testdata) == 0);
    nsl_free(resultString);
    resultString = NULL;
    CU_ASSERT(PAL_DBCommandFinalize(pCommand, NULL) == PAL_DB_OK);

    pCommand = NULL;
    CU_ASSERT(PAL_DBInitCommand(pConnect, g_lpstrSQL_GetBlob, &pCommand) == PAL_DB_OK);
    CU_ASSERT(PAL_DBBindBlobParam(pCommand, "@blob", (void*)g_testdata, strlen(g_testdata)) == PAL_DB_OK);
    CU_ASSERT(PAL_DBExecuteBlobEx(pCommand, (byte**)&resultString, &len) == PAL_DB_OK);
    CU_ASSERT(len == (int)strlen(g_testdata));
    CU_ASSERT(memcmp(resultString, g_testdata, len) == 0);
    nsl_free(resultString);
    resultString = NULL;
    CU_ASSERT(PAL_DBCommandFinalize(pCommand, NULL) == PAL_DB_OK);

    pCommand = NULL;
    CU_ASSERT(PAL_DBInitCommand(pConnect, g_lpstrSQL_GetReader, &pCommand) == PAL_DB_OK);
    CU_ASSERT(PAL_DBExecuteReader(pCommand, &pReader) == PAL_DB_OK);
    CU_ASSERT(PAL_DBReaderRead(pReader) == PAL_DB_ROW);
    CU_ASSERT(PAL_DBReaderGetInt(pReader, "integer", &resultInt) == PAL_DB_OK);
    CU_ASSERT(resultInt == *testInt);
    CU_ASSERT(PAL_DBReaderGetdouble(pReader, "double", &resultDouble) == PAL_DB_OK);
    CU_ASSERT(resultDouble == *testDouble);
    CU_ASSERT(PAL_DBReaderGetstring(pReader, "string", &resultString) == PAL_DB_OK);
    CU_ASSERT(strcmp(resultString, g_testdata) == 0);
    nsl_free(resultString);
    resultString = NULL;
    CU_ASSERT(PAL_DBReaderGetblob(pReader, "blob", (byte**)&resultString, &len) == PAL_DB_OK);
    CU_ASSERT(len == (int)strlen(g_testdata));
    CU_ASSERT(memcmp(resultString, g_testdata, len) == 0);
    nsl_free(resultString);

    CU_ASSERT(PAL_DBResetReader(pReader) == PAL_DB_OK);

    CU_ASSERT(PAL_DBCommandFinalize(pCommand, pReader) == PAL_DB_OK);

    CU_ASSERT(PAL_DBClose(pConnect) == PAL_DB_OK);
    PAL_Destroy(pal);
}
/*! @} */