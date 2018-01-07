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

    @file       StringUtility.h

    Class StringUtility contains common functions for
    shared_ptr<std::string>. User cannot create a StringUtility object.
    The functions of class StringUtility are static functions.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef STRINGUTILITY_H
#define STRINGUTILITY_H

/*!
    @addtogroup nbcommon
    @{
*/

#include "smartpointer.h"
#include <string>
#include <sstream>

namespace nbcommon
{

// Constants ....................................................................................


// Types ........................................................................................

/*! Common functions for shared_ptr<std::string> */
class StringUtility
{
public:
    // Public functions .........................................................................

    /*! Change the binary data to string.

        @return string. if string is empty then the convert fail, non-empty is success
    */
    static string BinaryBufferToString(const unsigned char* data, int len);

    /*! Check if two shared pointers to string objects are equal

        @return true if two objects are equal, false otherwise.
    */
    static bool
    IsStringEqual(shared_ptr<std::string> string1,
                  shared_ptr<std::string> string2);

    /*! Check if string1 is less than string2

        @return true if string1 is less than string2, false otherwise.
    */
    static bool
    IsStringLess(shared_ptr<std::string> string1,
                 shared_ptr<std::string> string2);

    /*! Compare two strings

        @return 0 if two strings are equal,
                negative sign if string1 is less than string2,
                positive sign if string1 is greater than string2.
    */
    static int
    CompareStdString(shared_ptr<std::string> string1,
                  shared_ptr<std::string> string2);

    /*! Replace all 'foundString' with 'replacedString' in the 'sourceString'

        @return None
    */
    static void
    ReplaceString(std::string& sourceString,         /*!< A string used to find and replace. It is
                                                          also the result of this function. */
                  const std::string& foundString,    /*!< A found string */
                  const std::string& replacedString, /*!< A replaced string */
                  bool matchWord = true              /*!< Whether to match the whole word */
                  );

    /*! Replace all 'foundString' with 'replacedString' in the 'sourceString'

      @return None
    */
    static void
    ReplaceString(std::string& sourceString,         /*!< A string used to find and replace. It is
                                                          also the result of this function. */
                  const char* foundString,           /*!< A found string */
                  const char* replacedString,        /*!< A replaced string */
                  bool matchWord = true              /*!< Whether to match the whole word */
                  );

    /*! Replace some special characters based on internal mapping.

        Some special characters should not appear in the base name part of file name. This
        function will replace those kind of characters based on internal mapping.

        @todo: If other module need this functionality too, we can move it to a PAL.

        @return None.
    */
    static void
    EscapeSpecialCharactersForBaseName(std::string& str);

    template <typename T>
    static void StringToNumber(const string& str, T& number)
    {
        std::stringstream ss(str);
        ss >> number;
    }

    template <typename T>
    static std::string NumberToString(T number)
    {
        std::stringstream ss;
        ss << number;
        return ss.str();
    }

    template <typename T>
    static shared_ptr<std::string> NumberToStringPtr(T number)
    {
        std::stringstream ss;
        ss << number;
        return shared_ptr<std::string>(new string(ss.str()));
    }

    static bool IsEmpty(const shared_ptr<string>& str)
    {
        return !str || str->empty();
    }

private:
    // Private functions ........................................................................

    // Constructor and destructor are not supported.
    StringUtility();
    ~StringUtility();
    StringUtility(const StringUtility& utility);
    StringUtility& operator=(const StringUtility& utility);
};

};  // namespace nbcommon

/*! @} */

#endif  // STRINGUTILITY_H
