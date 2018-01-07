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

    @file     contextbasedsingleton.h
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef CONTEXTBASEDSINGLETON_H
#define CONTEXTBASEDSINGLETON_H

extern "C"
{
    #include "nbcontextprotected.h"
    #include "palstdlib.h"
}

#include "smartpointer.h"
#include "Tuple.h"
#include <map>

//using namespace protocol;

typedef void (*deleter)(void* deletee);

/*! Context based data, it is used to save the context based singleton instance.
*/
class ContextBasedData
{
public:
    /*! ContextBasedData constructor */
    ContextBasedData(void* instance,
                     shared_ptr<std::map<NB_Context*, ContextBasedData> > dataMap)
    : m_instance(instance),
      m_dataMap(dataMap)
    {
    }

    /*! ContextBasedData copy constructor */
    ContextBasedData(const ContextBasedData& anotherData)
    : m_instance(anotherData.m_instance),
      m_dataMap(anotherData.m_dataMap)
    {
    }

    /*! ContextBasedData assignment operator */
    ContextBasedData&
    operator=(const ContextBasedData& anotherData)
    {
        if ((&anotherData) == this)
        {
            return *this;
        }

        m_instance = anotherData.m_instance;
        m_dataMap = anotherData.m_dataMap;
        return *this;
    }

    /*! ContextBasedData destructor */
    ~ContextBasedData()
    {
    }

    void* m_instance;       /*!< Context based singleton instance */

    /*! Reference of the global variable 'm_mapping'

        TRICKY: It is Circular Reference for the global variable
                'm_mapping'. It is used to ensure the global
                variable is destroyed after all NB_Context
                instances. When a NB_Context instance is destroyed,
                function ContextBasedSingleton<T>::ContextCallback
                is called, all ContextBasedData objects are
                destroyed and removed from the global variable, so
                the global variable is destroyed.
    */
    shared_ptr<std::map<NB_Context*, ContextBasedData> > m_dataMap;
};

/*
* Wraps an arbitrary class T and ensures that for a given NB_Context it always returns the same instance of T.
*/
template <class T>
class ContextBasedSingleton
{
private:
    static shared_ptr<std::map<NB_Context*, ContextBasedData> > m_mapping;

private:
    ContextBasedSingleton()
    {
        //std::cout << "Creating Context Based Singleton";
    }

    ContextBasedSingleton(const ContextBasedSingleton<T>&)
    {
        //std::cout << "Creating Context Based Singleton";
    }

    static void ContextCallback(void* userData)
    {
        shared_ptr<T>* instance = static_cast<shared_ptr<T>*>(userData);

        shared_ptr<std::map<NB_Context*, ContextBasedData> > mapping(m_mapping);
        if (mapping.get() &&
            (!(mapping->empty())) &&
            instance &&
            instance->get())
        {
            std::map<NB_Context*, ContextBasedData>::iterator iter = mapping->begin();
            std::map<NB_Context*, ContextBasedData>::iterator end  = mapping->end();
            for (; iter != end; ++ iter)
            {
                if (iter->second.m_instance == userData)
                {
                    mapping->erase(iter);
                    break;
                }
            }

            delete instance;
        }
    }

    static void DeleterCallback(T* t)
    {
        NB_Context* context = NULL;
        shared_ptr<T>* instance = NULL;

        if( t == NULL )
        {
            return;
        }
        context = t->GetContext();

        shared_ptr<std::map<NB_Context*, ContextBasedData> > mapping(m_mapping);
        if(mapping.get() &&
           (!(mapping->empty())) &&
           context)
        {
            std::map<NB_Context*, ContextBasedData>::iterator iter =
                    mapping->find(context);
            if (iter != mapping->end())
            {
                instance = (shared_ptr<T>*)(iter->second.m_instance);

                // Check if the found instance is same with the destroyed one.
                if (instance->get() == t)
                {
                    /* Remove the destroyed instance from map. The instance
                       should not be destroyed here, because this function is
                       the deleter of shared_ptr.
                     */
                    mapping->erase(iter);
                }
            }
        }

        delete t;
    }


    ~ContextBasedSingleton()
    {
        //std::cout << "Destroying Context based singleton";
    }

    /*! Functions to create instances with different count of elements (count from 1 to 6) in A Tuple object */
    template <class U1>
    static shared_ptr<T>*
    CreateInstance(NB_Context* context,
                   nbcommon::Tuple<TYPELIST_1(U1)>& otherParameters,
                   deleter callback)
    {
        return new shared_ptr<T>(new T(context,
                                       nbcommon::GetValue<0>(otherParameters)),
                                 callback);
    }

    template <class U1, class U2>
    static shared_ptr<T>*
    CreateInstance(NB_Context* context,
                   nbcommon::Tuple<TYPELIST_2(U1, U2)>& otherParameters,
                   deleter callback)
    {
        return new shared_ptr<T>(new T(context,
                                       nbcommon::GetValue<0>(otherParameters),
                                       nbcommon::GetValue<1>(otherParameters)),
                                 callback);
    }

    template <class U1, class U2, class U3>
    static shared_ptr<T>*
    CreateInstance(NB_Context* context,
                   nbcommon::Tuple<TYPELIST_3(U1, U2, U3)>& otherParameters,
                   deleter callback)
    {
        return new shared_ptr<T>(new T(context,
                                       nbcommon::GetValue<0>(otherParameters),
                                       nbcommon::GetValue<1>(otherParameters),
                                       nbcommon::GetValue<2>(otherParameters)),
                                 callback);
    }

    template <class U1, class U2, class U3, class U4>
    static shared_ptr<T>*
    CreateInstance(NB_Context* context,
                   nbcommon::Tuple<TYPELIST_4(U1, U2, U3, U4)>& otherParameters,
                   deleter callback)
    {
        return new shared_ptr<T>(new T(context,
                                       nbcommon::GetValue<0>(otherParameters),
                                       nbcommon::GetValue<1>(otherParameters),
                                       nbcommon::GetValue<2>(otherParameters),
                                       nbcommon::GetValue<3>(otherParameters)),
                                 callback);
    }

    template <class U1, class U2, class U3, class U4, class U5>
    static shared_ptr<T>*
    CreateInstance(NB_Context* context,
                   nbcommon::Tuple<TYPELIST_5(U1, U2, U3, U4, U5)>& otherParameters,
                   deleter callback)
    {
        return new shared_ptr<T>(new T(context,
                                       nbcommon::GetValue<0>(otherParameters),
                                       nbcommon::GetValue<1>(otherParameters),
                                       nbcommon::GetValue<2>(otherParameters),
                                       nbcommon::GetValue<3>(otherParameters),
                                       nbcommon::GetValue<4>(otherParameters)),
                                 callback);
    }

    template <class U1, class U2, class U3, class U4, class U5, class U6>
    static shared_ptr<T>*
    CreateInstance(NB_Context* context,
                   nbcommon::Tuple<TYPELIST_6(U1, U2, U3, U4, U5, U6)>& otherParameters,
                   deleter callback)
    {
        return new shared_ptr<T>(new T(context,
                                       nbcommon::GetValue<0>(otherParameters),
                                       nbcommon::GetValue<1>(otherParameters),
                                       nbcommon::GetValue<2>(otherParameters),
                                       nbcommon::GetValue<3>(otherParameters),
                                       nbcommon::GetValue<4>(otherParameters),
                                       nbcommon::GetValue<5>(otherParameters)),
                                 callback);
    }

    static
    shared_ptr<T>* FindTargetFromMapping(NB_Context* context)
    {
        shared_ptr<T>* result = NULL;

        shared_ptr<std::map<NB_Context*, ContextBasedData> > mapping(m_mapping);
        if (context &&
            mapping.get() &&
            (!(mapping->empty())))
        {
            std::map<NB_Context*, ContextBasedData>::iterator iter = mapping->find(context);
            if (iter != mapping->end())
            {
                void *pData = iter->second.m_instance;
                if (pData)
                {
                    shared_ptr<T>* t = static_cast<shared_ptr<T>*>(pData);
                    if (t && *t)
                    {
                        result = t;
                    }
                }
            }
        }
        return result;
    }

public:

    //@note: This function should only be called in CCC thread, or there might be threading
    //       issues when destroying nbcontext. Before NB_Context destroy, CCC will invoke
    //       registered callbacks in CCC thread, and will destroy this single instance of
    //       type T. This means, if user uses this getInstance thread other than CCC thread,
    //       instance will be created in one thread, but it will be destroyed in another
    //       thread ! Please beware of this threading issue if you insist use it in other
    //       threads.

    static shared_ptr<T> getInstance(NB_Context* context)
    {
        if (!context) // If context is NULL, return NULL pointer.
        {
            return shared_ptr<T>();
        }

        /* Create the global variable 'm_mapping' if it has not
           been created before.
        */
        shared_ptr<std::map<NB_Context*, ContextBasedData> > mapping(m_mapping);
        if (!(mapping.get()))
        {
            mapping.reset(new std::map<NB_Context*, ContextBasedData>());
            if (!(mapping.get()))
            {
                return shared_ptr<T>();
            }
            m_mapping = mapping;
        }

        shared_ptr<T>* t = FindTargetFromMapping(context);
        if (!t)
        {
            void*          pData = NULL;

            deleter m_callback=(deleter)&DeleterCallback;
            NB_ContextCallback context_callback;
            memset(&context_callback, 0, sizeof(context_callback));

            t = new shared_ptr<T>(new T(context), m_callback);
            pData = (void *)t;
            mapping->insert(pair<NB_Context*, ContextBasedData>(context, ContextBasedData(pData, mapping)));

            context_callback.callback = &ContextCallback;
            context_callback.callbackData = pData;
            NB_ContextRegisterCallback(context, &context_callback);
        }

        return *t;
    }

    template <class Typelist>
    static shared_ptr<T> getInstance(NB_Context* context,
                                     nbcommon::Tuple<Typelist>& otherParameters)
    {
        if (!context)
        {
            return shared_ptr<T>();
        }

        /* Create the global variable 'm_mapping' if it has not
           been created before.
        */
        shared_ptr<std::map<NB_Context*, ContextBasedData> > mapping(m_mapping);
        if (!(mapping.get()))
        {
            mapping.reset(new std::map<NB_Context*, ContextBasedData>());
            if (!(mapping.get()))
            {
                return shared_ptr<T>();
            }
            m_mapping = mapping;
        }

        shared_ptr<T>* t = FindTargetFromMapping(context);
        if (!t)
        {
            void*          pData = NULL;

            deleter m_callback=(deleter)&DeleterCallback;
            NB_ContextCallback context_callback;
            memset(&context_callback, 0, sizeof(context_callback));

            t = CreateInstance(context, otherParameters, m_callback);
            pData = (void *)t;
            mapping->insert(pair<NB_Context*, ContextBasedData>(context, ContextBasedData(pData, mapping)));

            context_callback.callback = &ContextCallback;
            context_callback.callbackData = pData;
            NB_ContextRegisterCallback(context, &context_callback);
        }

        return *t;
    }

};

template <class T> shared_ptr<std::map<NB_Context*, ContextBasedData> > ContextBasedSingleton<T>::m_mapping;

#endif  // CONTEXTBASEDSINGLETON_H
