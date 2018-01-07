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

    @file       StringUtility.cpp

    See header file for description.
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

#include "StringUtility.h"

extern "C"
{
#include "palstdlib.h"
}

/*! @{ */

using namespace std;
using namespace nbcommon;

// Local Constants ..............................................................................
typedef struct _SpecialCharacterMapping
{
    const char* source;
    const char* target;
} SpecialCharacterMapping;

static SpecialCharacterMapping gCharacterMapping[] =
{
    {"/", "-"},
    {NULL, NULL},
};

// Public functions .............................................................................

/* See header file for description */
bool
StringUtility::IsStringEqual(shared_ptr<string> string1,
                             shared_ptr<string> string2)
{
    return (StringUtility::CompareStdString(string1, string2) == 0);
}

/* See header file for description */
bool
StringUtility::IsStringLess(shared_ptr<string> string1,
                            shared_ptr<string> string2)
{
    return (StringUtility::CompareStdString(string1, string2) < 0);
}

/* See header file for description */
int
StringUtility::CompareStdString(shared_ptr<string> string1,
                             shared_ptr<string> string2)
{
    // Check if two pointers point to the same string.
    if (string1 == string2)
    {
        return 0;
    }

    // Check if one of pointers is 'NULL'.
    if (!string1)
    {
        return -1;
    }
    else if (!string2)
    {
        return 1;
    }

    return string1->compare(*string2);
}

/* See header file for description */
void
StringUtility::ReplaceString(string& sourceString,
                             const string& foundString,
                             const string& replacedString,
                             bool matchWord)
{
    string::size_type position = 0;
    string::size_type foundLength = foundString.size();
    string::size_type replacedLength = replacedString.size();

    // Find all 'foundString' in the 'sourceString' and replace with 'replacedString'.
    while ((position = sourceString.find(foundString, position)) != string::npos)
    {
        // @note: When matchWord is set to true, Only replace found string when
        // foudString is not embedded in other words. For example, if there is "$vfmt"
        // in sourceString, and we are going to replace "$v" with "1.0", we should not
        // replace the "$v" in "$vfmt" into "1.0".
        size_t endPosition = position+foundLength;
        if (!matchWord || endPosition == sourceString.size() ||
            !nsl_isalpha(sourceString[endPosition]))
        {
            sourceString.replace(position, foundLength, replacedString);
        }
        position += replacedLength;
    }
}

/* See header file for description */
void
StringUtility::ReplaceString(string& sourceString,
                             const char* foundString,
                             const char* replacedString,
                             bool matchWord)
{
    if (foundString && replacedString)
    {
        string::size_type position = 0;
        string::size_type foundLength = strlen(foundString);
        string::size_type replacedLength = strlen(replacedString);

        // Find all 'foundString' in the 'sourceString' and replace with 'replacedString'.
        while ((position = sourceString.find(foundString, position)) != string::npos)
        {
            // Only replace found string when foudString is not embedded in other words.
            // For example, if there is "$vfmt" in sourceString, and we are going to replace
            // "$v" with "1.0", we should not replace the "$v" in "$vfmt" into "1.0".
            if (!matchWord || !nsl_isalpha(sourceString[position+foundLength]))
            {
                sourceString.replace(position, foundLength, replacedString);
            }
            position += replacedLength;
        }
    }
}

/* See header file for description */
void StringUtility::EscapeSpecialCharactersForBaseName(string& str)
{
    SpecialCharacterMapping* ptr = &gCharacterMapping[0];
    while (ptr && ptr->source && ptr->target)
    {
        ReplaceString(str, ptr->source, ptr->target, false);
        ptr ++;
    }
}

string StringUtility::BinaryBufferToString(const unsigned char* data, int len)
{
    string binaryString;
    do
    {
        if (!data || len <= 0)
        {
            break;
        }

        // Create an array of char to convert binary data.
        int size = len;
        uint32 binaryCharLength = (uint32) ((size << 1) + 1) * sizeof(char);
        char*  binaryChar       = (char*) nsl_malloc(binaryCharLength);
        if (!binaryChar)
        {
            break;
        }

        nsl_memset(binaryChar, 0, binaryCharLength);

        // Convert to hex.
        int i = 0;
        for (i = 0; i < size; ++i)
        {
            nsl_sprintf((i << 1) + binaryChar, "%02X", *(data + i));
        }

        binaryString.assign(binaryChar);
        nsl_free(binaryChar);
    } while (0);

    return binaryString;
}

/*! @} */

