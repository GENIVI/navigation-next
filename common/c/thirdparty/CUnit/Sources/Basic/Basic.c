/*
 *  CUnit - A Unit testing framework library for C.
 *  Copyright (C) 2004, 2005  Anil Kumar, Jerry St.Clair
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *  Implementation for basic test runner interface.
 *
 *  Created By  : Jerry St.Clair  (11-Aug-2004)
 *  Comment     : Initial implementation of basic test runner interface
 *  EMail       : jds2@users.sourceforge.net
 *
 *  Modified    : 8-Jan-2005 (JDS)
 *  Comment     : Fixed reporting bug (bug report cunit-Bugs-1093861).
 *  Email       : jds2@users.sourceforge.net
 *
 *  Modified    : 30-Apr-2005 (JDS)
 *  Comment     : Added notification of suite cleanup failure.
 *  Email       : jds2@users.sourceforge.net
 */

/** @file
 * Basic interface with output to stdout.
 */
/** @addtogroup Basic
 * @{
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include "CUnit.h"
#include "TestDB.h"
#include "Util.h"
#include "TestRun.h"
#include "Basic.h"

#if defined(ANDROID_NDK)
    #include "revpallogutil.h"
#endif

// Included from <stdio.h> for build with WinCE
#ifdef WINCE
    #define _IONBF          0x0004
#endif // WINCE

/** Pointer to the currently running suite. */
static CU_pSuite f_pRunningSuite = NULL;
/** Current run mode. */
static CU_BasicRunMode f_run_mode = CU_BRM_NORMAL;

/* Forward declaration of module functions */
static CU_ErrorCode basic_initialize(void);
static CU_ErrorCode basic_run_all_tests(CU_pTestRegistry pRegistry);
static CU_ErrorCode basic_run_suite(CU_pSuite pSuite);
static CU_ErrorCode basic_run_single_test(CU_pSuite pSuite, CU_pTest pTest);

static void basic_test_start_message_handler(const CU_pTest pTest, const CU_pSuite pSuite);
static void basic_test_complete_message_handler(const CU_pTest pTest, const CU_pSuite pSuite, const CU_pFailureRecord pFailureList);
static void basic_all_tests_complete_message_handler(const CU_pFailureRecord pFailure);
static void basic_suite_init_failure_message_handler(const CU_pSuite pSuite);
static void basic_suite_cleanup_failure_message_handler(const CU_pSuite pSuite);

#if defined(WIN32) || defined(WINCE)
#include <share.h>
#include "windows.h"
#endif

#ifdef UNIX
#include <sys/stat.h>
#endif

#if defined(WIN32) && !defined(WINCE)
#define WIN32_LEAN_AND_MEAN
#endif

#define OUTPUT_MESSAGE OutputMessage
static void OutputMessage(FILE* fileStd, FILE* fileOut, const char* format, ...);

/*------------------------------------------------------------------------*/
/** Run all registered CUnit tests using the basic interface.
 *  The default CU_BasicRunMode is used unless it has been
 *  previously changed using CU_basic_set_mode().  The CUnit test
 *  registry must have been initialized before calling this function.
 *  @return A CU_ErrorCode indicating the framework error condition, including
 *          CUE_NOREGISTRY - Registry has not been initialized.
 */
CU_DEFINE CU_ErrorCode CU_basic_run_tests(void)
{
  CU_ErrorCode error;

  if (NULL == CU_get_registry()) {
    if (CU_BRM_SILENT != f_run_mode)
      OUTPUT_MESSAGE(stderr, NULL, "\n\nFATAL ERROR - Test registry is not initialized.\n");
    error = CUE_NOREGISTRY;
  }
  else if (CUE_SUCCESS == (error = basic_initialize()))
    error = basic_run_all_tests(NULL);

  return error;
}

/*------------------------------------------------------------------------*/
/** Run all tests for a specific suite in the basic interface.
 *  If pSuite is NULL, the function returns without taking any
 *  action. The default CU_BasicRunMode is used unless it has
 *  been changed using CU_basic_set_mode().
 *  @param pSuite The CU_Suite to run.
 *  @return A CU_ErrorCode indicating the framework error condition, including
 *          CUE_NOSUITE - pSuite was NULL.
 */
CU_DEFINE CU_ErrorCode CU_basic_run_suite(CU_pSuite pSuite)
{
  CU_ErrorCode error;

  if (NULL != pSuite)
    error = CUE_NOSUITE;
  else if (CUE_SUCCESS == (error = basic_initialize()))
    error = basic_run_suite(pSuite);

  return error;
}

/*------------------------------------------------------------------------*/
/** Run a single test in a specific suite in the basic interface.
 *  If pSuite or pTest is NULL, the function returns without
 *  taking any action.  The default CU_BasicRunMode is used unless
 *  it has been changed using CU_basic_set_mode.
 *  @param pSuite The CU_Suite holding the CU_Test to run.
 *  @param pTest  The CU_Test to run.
 *  @return A CU_ErrorCode indicating the framework error condition, including
 *          CUE_NOSUITE - pSuite was NULL.
 *          CUE_NOTEST  - pTest was NULL.
 */
CU_DEFINE CU_ErrorCode CU_basic_run_test(CU_pSuite pSuite, CU_pTest pTest)
{
  CU_ErrorCode error;

  if (NULL != pSuite)
    error = CUE_NOSUITE;
  else if (NULL != pTest)
    error = CUE_NOTEST;
  else if (CUE_SUCCESS == (error = basic_initialize()))
    error = basic_run_single_test(pSuite, pTest);

  return error;
}

/*------------------------------------------------------------------------*/
/** Set the run mode for the basic interface.
 *  @param mode The new CU_BasicRunMode for subsequent test
 *              runs using the basic interface.
 */
CU_DEFINE void CU_basic_set_mode(CU_BasicRunMode mode)
{
  f_run_mode = mode;
}

/*------------------------------------------------------------------------*/
/** Retrieve the current run mode for the basic interface.
 *  @return The current CU_BasicRunMode setting for test
 *              runs using the basic interface.
 */
CU_DEFINE CU_BasicRunMode CU_basic_get_mode(void)
{
  return f_run_mode;
}

/*------------------------------------------------------------------------*/
/** Print a summary of run failures to stdout.
 *  This is provided for user convenience upon request, and
 *  does not take into account the current run mode.  The
 *  failures are printed to stdout independent of the most
 *  recent run mode.
 *  @param pFailure List of CU_pFailureRecord's to output.
 */
CU_DEFINE void CU_basic_show_failures(CU_pFailureRecord pFailure)
{
  int i = 0;

  for (i = 1 ; (NULL != pFailure) ; pFailure = pFailure->pNext, i++) {
    OUTPUT_MESSAGE(stdout, NULL, "\n  %d. %s:%u  - %s", i,
        (NULL != pFailure->strFileName) ? pFailure->strFileName : "",
        pFailure->uiLineNumber,
        (NULL != pFailure->strCondition) ? pFailure->strCondition : "");
  }
}

/*------------------------------------------------------------------------*/
/** Perform inialization actions for the basic interface.
 *  This includes setting output to unbuffered, printing a
 *  welcome message, and setting the test run handlers.
 *  @return An error code indicating the framework error condition.
 */
static CU_ErrorCode basic_initialize(void)
{
  /* Unbuffered output so everything reaches the screen */
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  CU_set_error(CUE_SUCCESS);

  if (CU_BRM_SILENT != f_run_mode)
    OUTPUT_MESSAGE(stdout, NULL, "\n\n     CUnit - A Unit testing framework for C - Version " CU_VERSION
                  "\n     http://cunit.sourceforge.net/\n\n");

  CU_set_test_start_handler(basic_test_start_message_handler);
  CU_set_test_complete_handler(basic_test_complete_message_handler);
  CU_set_all_test_complete_handler(basic_all_tests_complete_message_handler);
  CU_set_suite_init_failure_handler(basic_suite_init_failure_message_handler);
  CU_set_suite_cleanup_failure_handler(basic_suite_cleanup_failure_message_handler);

  return CU_get_error();
}

/*------------------------------------------------------------------------*/
/** Run all tests within the basic interface.
 *  If non-NULL, the test registry is changed to the specified
 *  registry before running the tests, and reset to the original
 *  registry when done.  If NULL, the default CUnit test registry
 *  will be used.
 *  @param pRegistry The CU_pTestRegistry containing the tests
 *                   to be run.  If NULL, use the default registry.
 *  @return An error code indicating the error status
 *          during the test run.
 */
static CU_ErrorCode basic_run_all_tests(CU_pTestRegistry pRegistry)
{
  CU_pTestRegistry pOldRegistry = NULL;
  CU_ErrorCode result;

  f_pRunningSuite = NULL;

  if (NULL != pRegistry)
    pOldRegistry = CU_set_registry(pRegistry);
  result = CU_run_all_tests();
  if (NULL != pRegistry)
    CU_set_registry(pOldRegistry);
  return result;
}

/*------------------------------------------------------------------------*/
/** Run a specified suite within the basic interface.
 *  @param pSuite The suite to be run (non-NULL).
 *  @return An error code indicating the error status
 *          during the test run.
 */
static CU_ErrorCode basic_run_suite(CU_pSuite pSuite)
{
  f_pRunningSuite = NULL;
  return CU_run_suite(pSuite);
}

/*------------------------------------------------------------------------*/
/** Run a single test for the specified suite within
 *  the console interface.
 *  @param pSuite The suite containing the test to be run (non-NULL).
 *  @param pTest  The test to be run (non-NULL).
 *  @return An error code indicating the error status
 *          during the test run.
 */
static CU_ErrorCode basic_run_single_test(CU_pSuite pSuite, CU_pTest pTest)
{
  f_pRunningSuite = NULL;
  return CU_run_test(pSuite, pTest);
}

/*------------------------------------------------------------------------*/
/** Handler function called at start of each test.
 *  @param pTest  The test being run.
 *  @param pSuite The suite containing the test.
 */
static void basic_test_start_message_handler(const CU_pTest pTest, const CU_pSuite pSuite)
{
  FILE* fo = NULL;
  char* path = NULL;
  size_t pathSize = 0;
  size_t convertedSize = 0;
  wchar_t* wcPath = NULL;

  assert(NULL != pSuite);
  assert(NULL != pTest);

  path = malloc(strlen(TXT) + strlen(RESULT_FOLDER_PATH) + strlen(pSuite->pName) + 1);
  if (!path)
  {
    return;
  }

  strcpy(path, RESULT_FOLDER_PATH);

#if defined(WIN32) || defined(WINCE)
  pathSize = strlen(path) + 1;
  wcPath = malloc(sizeof(wchar_t)*pathSize);
  if (!wcPath)
  {
    return;
  }

  // Convert char* to wchar_t* for CreateDirectory()
  mbstowcs_s(&convertedSize, wcPath, pathSize, path, _TRUNCATE);

  // Create directory from RESULT_FOLDER_PATH if it does not exist
  CreateDirectory(wcPath, NULL);

  free(wcPath);
#else //UNIX
  mkdir(path, 0777);
#endif

  strcat(path, pSuite->pName);
  strcat(path, TXT);

  if (CU_BRM_VERBOSE == f_run_mode)
  {
    if ((NULL == f_pRunningSuite) || (f_pRunningSuite != pSuite))
    {
      fo = fopen(path, "w");
      if (!fo)
      {
        return;
      }
      f_pRunningSuite = pSuite;
      OUTPUT_MESSAGE(stdout, fo, "\nSuite: %s", (NULL != pSuite->pName) ? pSuite->pName : "");
      OUTPUT_MESSAGE(stdout, fo, "\n  Test: %s ... ", (NULL != pTest->pName) ? pTest->pName : "");
    }
    else
    {
      fo = fopen(path, "a");
      if (!fo)
      {
        return;
      }
      OUTPUT_MESSAGE(stdout, fo, "\n  Test: %s ... ", (NULL != pTest->pName) ? pTest->pName : "");
    }
  }
  fclose(fo);
  free(path);
}

/*------------------------------------------------------------------------*/
/** Handler function called at completion of each test.
 *  @param pTest   The test being run.
 *  @param pSuite  The suite containing the test.
 *  @param pFailure Pointer to the 1st failure record for this test.
 */
static void basic_test_complete_message_handler(const CU_pTest pTest,
                                                const CU_pSuite pSuite,
                                                const CU_pFailureRecord pFailureList)
{
  CU_pFailureRecord pFailure = pFailureList;
  int i = 0;
  FILE* fo = NULL;
  char* path = NULL;

  assert(NULL != pSuite);
  assert(NULL != pTest);

  path = malloc(strlen(TXT) + strlen(RESULT_FOLDER_PATH) + strlen(pSuite->pName) + 1);
  if (!path)
  {
    return;
  }

  strcpy(path, RESULT_FOLDER_PATH);
  strcat(path, pSuite->pName);
  strcat(path, TXT);
  fo = fopen(path, "a");
  if (!fo)
  {
    return;
  }

  if (NULL == pFailure) {
    if (CU_BRM_VERBOSE == f_run_mode) {
      OUTPUT_MESSAGE(stdout, fo, "PASSED");
    }
  }
  else {
    switch (f_run_mode) {
      case CU_BRM_VERBOSE:
        OUTPUT_MESSAGE(stdout, fo, "FAILED");
        break;
      case CU_BRM_NORMAL:
        OUTPUT_MESSAGE(stdout, fo, "\nSuite %s, Test %s had failures:", 
                        (NULL != pSuite->pName) ? pSuite->pName : "",
                        (NULL != pTest->pName) ? pTest->pName : "");
        break;
      default:  /* gcc wants all enums covered.  ok. */
        break;
    }
    if (CU_BRM_SILENT != f_run_mode)
    {
      for (i = 1 ; (NULL != pFailure) ; pFailure = pFailure->pNext, i++)
      {
        OUTPUT_MESSAGE(stdout, fo, "\n    %d. %s:%u  - %s", i,
            (NULL != pFailure->strFileName) ? pFailure->strFileName : "",
            pFailure->uiLineNumber,
            (NULL != pFailure->strCondition) ? pFailure->strCondition : "");
      }
    }
  }
  fclose(fo);
  free(path);
}

/*------------------------------------------------------------------------*/
/** Handler function called at completion of all tests in a suite.
 *  @param pFailure Pointer to the test failure record list.
 */
static void basic_all_tests_complete_message_handler(const CU_pFailureRecord pFailure)
{
  CU_pRunSummary pRunSummary = CU_get_run_summary();
  CU_pTestRegistry pRegistry = CU_get_registry();
  FILE *res = NULL;

#if defined(WIN32) || defined(WINCE)
  // Using _fsopen() instead of fopen() allows us to ensure that
  // this file is not in use in another process
  res = _fsopen(RESULT_FILE_PATH, "a", _SH_DENYRW);

  if (res == NULL)
  {
    // Here we need to handle situation when RESULT_FILE_PATH had been opened in another process
    unsigned long start_time = GetTickCount();
    while ((GetTickCount() - start_time) < 60000 && !res)
    {
      Sleep(1000); // Wait for 1 second then try to open file again
      res = _fsopen(RESULT_FILE_PATH, "a", _SH_DENYRW);
    }
    if (res == NULL)
    {
      fprintf(stderr, "Can not open result file for edit: access denied.");
      return;
    }
  }
#else //UNIX
  res = fopen(RESULT_FILE_PATH, "a");
#endif 

  CU_UNREFERENCED_PARAMETER(pFailure); /* not used in basic interface */

  assert(NULL != pRunSummary);
  assert(NULL != pRegistry);

  f_pRunningSuite = NULL;

  if (CU_BRM_SILENT != f_run_mode)
  {
    OUTPUT_MESSAGE(stdout, res, "\n\n--Run Summary: Type      Total     Ran  Passed  Failed"
                     "\n               suites %8u%8u     n/a%8u"
                     "\n               tests  %8u%8u%8u%8u"
                     "\n               asserts%8u%8u%8u%8u\n",
                    pRegistry->uiNumberOfSuites,
                    pRunSummary->nSuitesRun,
                    pRunSummary->nSuitesFailed,
                    pRegistry->uiNumberOfTests,
                    pRunSummary->nTestsRun,
                    pRunSummary->nTestsRun - pRunSummary->nTestsFailed,
                    pRunSummary->nTestsFailed,
                    pRunSummary->nAsserts,
                    pRunSummary->nAsserts,
                    pRunSummary->nAsserts - pRunSummary->nAssertsFailed,
                    pRunSummary->nAssertsFailed);
  }
  if(res) fclose(res);
}

/*------------------------------------------------------------------------*/
/** Handler function called when suite initialization fails.
 *  @param pSuite The suite for which initialization failed.
 */
static void basic_suite_init_failure_message_handler(const CU_pSuite pSuite)
{
  FILE* fo = NULL;
  char* path = NULL;

  assert(NULL != pSuite);

  path = malloc(strlen(TXT)+strlen(RESULT_FOLDER_PATH)+strlen(pSuite->pName) + 1);
  if (!path)
  {
    return;
  }

  strcpy(path, RESULT_FOLDER_PATH);
  strcat(path, pSuite->pName);
  strcat(path, TXT);
  fo = fopen(path, "a");
  if (!fo)
  {
    return;
  }

  if (CU_BRM_SILENT != f_run_mode)
    OUTPUT_MESSAGE(stdout, fo,
            "\nWARNING - Suite initialization failed for %s.",
            (NULL != pSuite->pName) ? pSuite->pName : "");

  fclose(fo);
  free(path);
}

/*------------------------------------------------------------------------*/
/** Handler function called when suite cleanup fails.
 *  @param pSuite The suite for which cleanup failed.
 */
static void basic_suite_cleanup_failure_message_handler(const CU_pSuite pSuite)
{
  FILE* fo = NULL;
  char* path = NULL;

  assert(NULL != pSuite);

  path = malloc(strlen(TXT)+strlen(RESULT_FOLDER_PATH)+strlen(pSuite->pName) + 1);
  if (!path)
  {
    return;
  }

  strcpy(path, RESULT_FOLDER_PATH);
  strcat(path, pSuite->pName);
  strcat(path, TXT);
  fo = fopen(path, "a");
  if (!fo)
  {
    return;
  }

  if (CU_BRM_SILENT != f_run_mode)
    OUTPUT_MESSAGE(stdout, fo,
            "\nWARNING - Suite cleanup failed for %s.",
            (NULL != pSuite->pName) ? pSuite->pName : "");

  fclose(fo);
  free(path);
}

/*! OutputMessage displays message on specified file and, on Win32, in debugger window

This function is not part of standard CUnit and was added by Networks In Motion
*/
static void OutputMessage(FILE* fileStd, FILE* fileOut, const char* format, ...)
{
#if defined(WIN32) || defined(WINCE)
  va_list args;
  int len = 0;
  char* buffer = NULL;

  va_start(args, format);
  len = _vscprintf(format, args) + 1;
  buffer = malloc(len * sizeof(char));
  if (!buffer)
  {
    return;
  }

  vsprintf(buffer, format, args);

  OutputDebugStringA(buffer);
  if (fileStd)
  {
    fprintf(fileStd, buffer);
  }
  if (fileOut)
  {
    fprintf(fileOut, buffer);
  }
  free(buffer);
  va_end(args);
#else //UNIX
    va_list args;
    va_start(args, format);
    if (fileStd)
    {
        #if defined(ANDROID_NDK)
          if(fileOut) vfprintf(fileOut, format, args);
          vprintf(format, args);
        #else
		vfprintf(fileStd, format, args);
        #endif
	}
	va_end(args);
#endif
}
/** @} */
