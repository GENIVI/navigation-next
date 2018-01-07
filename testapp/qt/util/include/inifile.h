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
    @file     inifile.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

using namespace std;

typedef map<string, string, less<string> > strMap;
typedef strMap::iterator strMapIt;

const char*const MIDDLESTRING = "_____***_______";//string key_____***______value store in the map

/*! Declare ini Ini file analysis struct

    This Class defined for analysis ini file
*/
struct analyzeini{
    string strsect;
    strMap *pmap;
    analyzeini(strMap & strmap):pmap(&strmap){}
//read the ini record,analyze and store in the map
    void operator()( const string & strini)
    {
//get ini term name
        int first =strini.find('[');
        int last = strini.rfind(']');
        if( first != string::npos && last != string::npos && first != last+1)
        {
            strsect = strini.substr(first+1,last-first-1);
            return ;
        }
        if(strsect.empty())
            return ;
//get ini sub term name and value
        if((first=strini.find('='))== string::npos)
            return ;
        string strtmp1= strini.substr(0,first);
        string strtmp2=strini.substr(first+1, string::npos);
//got the key
        first= strtmp1.find_first_not_of(" \t");
        last = strtmp1.find_last_not_of(" \t");
        if(first == string::npos || last == string::npos)
            return ;
        string strkey = strtmp1.substr(first, last-first+1);
//got the value
        first = strtmp2.find_first_not_of(" \t");
        if(((last = strtmp2.find("\t#", first )) != string::npos) ||
            ((last = strtmp2.find(" #", first )) != string::npos) ||
            ((last = strtmp2.find("\t//", first )) != string::npos)||
            ((last = strtmp2.find(" //", first )) != string::npos))
        {
            strtmp2 = strtmp2.substr(0, last-first);
        }
        last = strtmp2.find_last_not_of(" \t");
        if(first == string::npos || last == string::npos)
            return ;
        string value = strtmp2.substr(first, last-first+1);
        string mapkey = strsect + MIDDLESTRING;
        mapkey += strkey;
//store key,value in the map
        (*pmap)[mapkey]=value;
        return ;
    }
};

/*! Declare ini Ini file process class

    This Class defined for read process ini file
*/
class IniFile
{
public:
    IniFile( ){};
    ~IniFile( ){};

    static IniFile& Instance()
    {
         static IniFile obj;
         return obj;
    }
    bool open(const char* pinipath)
    {
        return do_open(pinipath);
    }
//got the value through key
    string read(const char*psect, const char*pkey)
    {
        string mapkey = psect;
        mapkey += MIDDLESTRING;
        mapkey += pkey;
        strMapIt it = c_inimap.find(mapkey);
        if(it == c_inimap.end())
            return "";
        else
            return it->second;
    }
//get the value through key
    string read(const char*psect, const char*pkey, const char* pDefualt)
    {
        string mapkey = psect;
        mapkey += MIDDLESTRING;
        mapkey += pkey;
        strMapIt it = c_inimap.find(mapkey);
        if(it == c_inimap.end())
            return pDefualt;
        else
            return it->second;
    }
protected:
    bool do_open(const char* pinipath)
    {
        ifstream fin(pinipath);
        if(!fin.is_open())
            return false;
        vector<string> strvect;
        while(!fin.eof())
        {
            string inbuf;
            getline(fin, inbuf,'\n');
            strvect.push_back(inbuf);
        }
        if(strvect.empty())
            return false;
        for_each(strvect.begin(), strvect.end(), analyzeini(c_inimap));
        return !c_inimap.empty();
    }
    strMap c_inimap;
};

/*! @} */
