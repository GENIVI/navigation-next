/*!--------------------------------------------------------------------------

 

 @file     SpeexEncoderInterface.h

 @date     02/23/2012

 @defgroup Speex Encoder Public Interface

 

 @brief    Speex Encoder Public Interface

 

 */

/*

 (C) Copyright 2012 by TeleCommunication Systems, Inc.           



 The information contained herein is confidential, proprietary 

 to TeleCommunication Systems, Inc., and considered a trade secret as 

 defined in section 499C of the penal code of the State of     

 California. Use of this information by anyone other than      

 authorized employees of TeleCommunication Systems is granted only    

 under a written non-disclosure agreement, expressly           

 prescribing the scope and manner of such use.                 

 

 ---------------------------------------------------------------------------*/



/*! @{ */



#ifndef SPEEXENCODERINTERFACE_H

#define SPEEXENCODERINTERFACE_H





#include "palexp.h"

#include "pal.h"

#include "base.h"





namespace tpspeexencoder

{



/*! Speex Encoder Public Interface

 */

class SpeexEncoderInterface : public Base

{

public:

/*! Destroys previously created object belong to SpeexEncoderInterface



 @returns none

*/

    virtual void Release(void) = 0;





/*! Encodes passed buffer to speex format. Client is responsible for free output buffer.



 @param inBuffer byte* pointer to buffer in wav format to be encoded to speex

 @param inSize uint32 size of buffer to be encoded

 @param lastBuffer nb_boolean Pass TRUE for last buffer in audio sequence and FALSE otherwise

 @param outBuffer byte** pointer to encoded buffer. Client is responsible to free this data.

 @param outSize uint32* size of encoded buffer



 @returns PAL_Error

*/

    virtual PAL_Error EncodeBuffer(byte* inBuffer, uint32 inSize, nb_boolean lastBuffer, byte** outBuffer, uint32* outSize) = 0;





/*! Decodes passed buffer from speex format. Client is responsible for free output buffer.



 @param inBuffer byte* pointer to buffer in speex format to be decoded to initial format

 @param inSize uint32 size of buffer to be decoded

 @param outBuffer byte** pointer to decoded buffer. Client is responsible to free this data.

 @param outSize uint32* size of decoded buffer



 @returns PAL_Error

*/

    virtual PAL_Error DecodeBuffer(byte* inBuffer, uint32 inSize, byte** outBuffer, uint32* outSize) = 0;





/*! Resets speex encoder history. Should be called on start processing new audio stream.

 */

    virtual void ResetHistory(void) = 0;



protected:

    virtual ~SpeexEncoderInterface(void) {};

};





/*! Creates an instance of the Speech Encoder object



 @param pal PAL_Instance

 @param bitsPerSample uint32 input audio bits per sample

 @param sampleRate uint32 input audio sample rate

 @param bigEndian nb_boolean big endian data format

 @param speexEncoder output pointer to newly created Speex Encoder object



 @return PAL_Error

 */

PAL_DEC PAL_Error CreateSpeexEncoder(PAL_Instance* pal,

                                     uint32 bitsPerSample,

                                     uint32 sampleRate,

                                     nb_boolean bigEndian,

                                     SpeexEncoderInterface** speexEncoder

                                     );



} //tpspeexencoder



#endif //SPEEXENCODERINTERFACE_H



/*! @} */

