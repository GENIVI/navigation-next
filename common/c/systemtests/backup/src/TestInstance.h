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
   @file        TestInstance.h
   @defgroup

   Description: This file provides several classes for other test cases to
   inherit, and some macros to perform tests quickly.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */


#ifndef _TESTINSTANCE_H_
#define _TESTINSTANCE_H_


#include "pal.h"
#include "nbcontext.h"

#include <vector>
#include "base.h"

#define DESTROY_TIMEOUT 2 * 1000

/*! A abstract class to execute functions through function pointers in TestIntance.

    This class does nothing but provide a way to store class pointers in TestInstance.
*/
class BaseTask : public Base
{
public:
    BaseTask()
            : Base()
    {
        // Nothing to do.
    }
    virtual ~BaseTask() {}

    /*! Execute the test task, ie, the real test function.

        This task is usually the real test function that should be executed in CCC thread.

        @return None.
    */
    virtual void ExecuteTask() = 0;

    /*! Execute the callback function.

        Usually this callback function should called by CCC thread to  set m_completeEvent
        to unblock main ui thread.

        @return void
    */
    virtual void ExecuteCallback() = 0;
};

/*! Template class inherited from BaseClass to execute test functions.

    Each TestInstance should create a instantiated objec of TestTask by providing parameters
    as explained in the constructor. This class will stored in TestInstance.

 */
template <typename T>
class TestTask : public BaseTask
{
public:
    /*! Constructor */
    TestTask(T* object,                         /*!< Pointer to a test instance */
             void (T::*pTestFunction)(),        /*!< Pointer to test function in TestInstance*/
             void (T::*pTestCallbackFunction)() /*!< Pointer to callback function in TestInstance*/
             )
            : m_pObject(object),
              m_pTestFunction(pTestFunction),
              m_pTestCallbackFunction(pTestCallbackFunction)
    {
    }
    /*! Deconstructor */
    virtual ~TestTask()
    {
    }

    // Refer to description of BaseTask;
    void ExecuteTask()
    {
        if (m_pObject && m_pTestFunction)
        {
            (m_pObject->*m_pTestFunction)();
        }
    }

    // Refer to description of BaseTask;
    void ExecuteCallback()
    {
        if (m_pObject && m_pTestCallbackFunction)
        {
            (m_pObject->*m_pTestCallbackFunction)();
        }
    }

private:
    T* m_pObject;                           /*!< Pointer to a test instance */
    void (T::*m_pTestFunction)();           /*!< Pointer to test function in TestInstance*/
    void (T::*m_pTestCallbackFunction)();   /*!< Pointer to callback function in TestInstance*/
};

/*! Test instance class provides serveral basic facilities to Initialize NB_Pal and
    NB_Context for SystemTests.

    Test cases should inherit from this class, and extend it with their own test cases.
    Refer to TestLocaltionInstance in TestLoaction.cpp for example.

    @note: this is just test, make all functions and member fields public make it easier to
    write test cases.
 */
class TestInstance
{
public:
    TestInstance();
    virtual ~TestInstance();

    /*! Initializes the TestInstance

        @return NB_OK if succeeded.
    */
    NB_Error Initialize();

    /*! Schedule to finish the test.

        This function will release related resources and set m_completeEvent.

        @return void
    */
    void ScheduleFinishTest();

    /*! Schedule to invoke test function in correct thread.

        @return None
    */
    void CallTestFunction(void);

    /*! Clean up local allocated resources.

        classes inherits from this class should implement this function to cleanup allocated
        resources here. Any objects that needs to be destroyed before NB_Context should be
        called here.

        @return None.
    */
    virtual void FinishTestPrivate() = 0;

    /*! Wait complete event to be set.

      @return true if succeeded.
    */
    bool WaitEvent(uint32 timeout);

    PAL_Instance* m_pal;
    NB_Context*   m_context;
    bool          m_initialized;
    void*         m_completeEvent;
    BaseTask*     m_functor;
private:

    /*! Helper function to invoke tasks.

      @return void
    */
    static void EventTaskCallback(PAL_Instance* pal, void* userData);

    /*! Release resources and finish test.

      @return void
    */
    void FinishTest();
};




#endif /* _TESTINSTANCE_H_ */

/*! @} */
