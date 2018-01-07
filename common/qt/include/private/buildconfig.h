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

#ifndef BUILDCONFIG_H
#define BUILDCONFIG_H

#define DEFAULT_PORT_HTTP               80
#define DEFAULT_PORT_TCP                8128
#define DEFAULT_PORT_TCPTLS             8130

#define DEFAULT_HOST_NAME               ""
#define DEFAULT_DEVICE                  "qt"
#define DEFAULT_DOMAIN                  "mobile.location.studio"
#define DEFAULT_COUNTRY                 "USA"
#define DEFAULT_LANGUAGE                "en_US"
#define DEFAULT_MDN                     0LL
#define DEFAULT_PRODUCT_NAME            "ltk_qt"
#define DEFAULT_PUSH_MESSAGE_GUID       ""

const byte VOICE_CACHE_OBFUSCATE_KEY[] =
{
    70, 225,  82,  73, 156, 130, 140, 111, 157, 241,  72,  76,  78, 245,
    167,  59,  40,  95, 131, 205,  65, 110, 123, 157, 172,   7, 189, 197,
    104,  51, 121,  62, 101,  50, 157,  94, 149, 201, 107, 202, 221, 200,
    97,  16,  90,  76, 214, 231, 110,  98, 178, 222,  76,   6, 229, 112,
    110, 187, 208, 148, 124,  66, 161, 228, 185,  29, 228, 196, 205, 149,
    86, 226,  84,   5, 203, 189, 221,  98, 243, 148, 120,  70, 131, 242,
    184, 183, 189, 237,  54, 147,  65, 106, 218,  12,  22,  62, 171, 195,
    176,  28, 142,  42,   5,  29, 205,  51,  26, 172, 158,  51, 147,  79,
    97, 208, 154, 189,   3
};

const byte RASTER_TILE_CACHE_OBFUSCATE_KEY[] =
{
    70, 225,  82,  73, 156, 130, 140, 111, 157, 241,  72,  76,  78, 245,
    167,  59,  40,  95, 131, 205,  65, 110, 123, 157, 172,   7, 189, 197,
    104,  51, 121,  62, 101,  50, 157,  94, 149, 201, 107, 202, 221, 200,
    97,  16,  90,  76, 214, 231, 110,  98, 178, 222,  76,   6, 229, 112,
    110, 187, 208, 148, 124,  66, 161, 228, 185,  29, 228, 196, 205, 149,
    86, 226,  84,   5, 203, 189, 221,  98, 243, 148, 120,  70, 131, 242,
    184, 183, 189, 237,  54, 147,  65, 106, 218,  12,  22,  62, 171, 195,
    176,  28, 142,  42,   5,  29, 205,  51,  26, 172, 158,  51, 147,  79,
    97, 208, 154, 189,   3
};

#endif // BUILDCONFIG_H
