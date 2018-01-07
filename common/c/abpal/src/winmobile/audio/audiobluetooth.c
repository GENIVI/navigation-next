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

    @file     audiobluetooth.c
    @defgroup PAL Audio API

    @brief    Bluetooth audio control

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

#include "audiobluetooth.h"
#include "windows.h"
#include "regext.h"

const wchar_t* BLUETOOTH_AUDIO_GATEWAY = L"BAG0:";

#define IOCTL_AG_OPEN_AUDIO         0x01
#define IOCTL_AG_CLOSE_AUDIO        0x02
#define IOCTL_AG_CLOSE_CONTROL      0x03
#define IOCTL_AG_SET_SPEAKER_VOL    0x04
#define IOCTL_AG_SET_MIC_VOL        0x05
#define IOCTL_AG_GET_SPEAKER_VOL    0x06
#define IOCTL_AG_GET_MIC_VOL        0x07
#define IOCTL_AG_GET_POWER_MODE     0x08
#define IOCTL_AG_SET_POWER_MODE     0x09
#define IOCTL_AG_OPEN_CONTROL       0x0A

#define SN_BLUETOOTHSTATEA2DPCONNECTED_ROOT HKEY_LOCAL_MACHINE
#define SN_BLUETOOTHSTATEA2DPCONNECTED_PATH TEXT("System\\State\\Hardware")
#define SN_BLUETOOTHSTATEA2DPCONNECTED_VALUE TEXT("Bluetooth")
#define SN_BLUETOOTHSTATEA2DPCONNECTED_BITMASK 4

static BOOL
IsA2DPConnected()
{
    BOOL a2dpConnected = FALSE;   

    HRESULT hr = S_FALSE;
    DWORD value = 0;

    hr = RegistryGetDWORD(SN_BLUETOOTHSTATEA2DPCONNECTED_ROOT,
        SN_BLUETOOTHSTATEA2DPCONNECTED_PATH,
        SN_BLUETOOTHSTATEA2DPCONNECTED_VALUE,
        &value);

    if (hr == S_OK && value & SN_BLUETOOTHSTATEA2DPCONNECTED_BITMASK)
    {
        a2dpConnected = TRUE;
    }
    
    return a2dpConnected;
}

static HANDLE
OpenBluetoothAudio(void)
{
    return CreateFile(BLUETOOTH_AUDIO_GATEWAY, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
}

static PAL_Error
SendBluetoothAudioCommand(HANDLE device, int32 command, void* inValue, int32 inValueSize, void* outValue, int32 outValueSize)
{
    PAL_Error result = PAL_ErrAudioGeneral;
    if (device)
    {
        if (DeviceIoControl(device, command, inValue, inValueSize, outValue, outValueSize, NULL, NULL))
        {
            result = PAL_Ok;
        }
        else
        {
            switch (GetLastError())
            {
            case ERROR_NOT_READY:
            case ERROR_BAD_UNIT:
                result = PAL_ErrAudioDeviceOpen;
                break;
            default:
                result = PAL_ErrAudioGeneral;
                break;
            }
        }
    }

    return result;
}

static void
CloseBluetoothAudio(HANDLE device)
{
    (void)CloseHandle(device);
}

static PAL_Error
SendBluetoothAudioGatewayCommand(int32 status)
{
    PAL_Error result = PAL_ErrAudioGeneral;

    HANDLE device = OpenBluetoothAudio();
    if (device)
    {
        result = SendBluetoothAudioCommand(device, status, NULL, 0, NULL, 0);
        CloseBluetoothAudio(device);
    }

    return result;
}

ABPAL_DEC PAL_Error
AudioBluetoothGatewayOpen(void)
{
    PAL_Error result = PAL_Ok;
            
    if (!IsA2DPConnected())
    {
        result = SendBluetoothAudioGatewayCommand(IOCTL_AG_OPEN_AUDIO);
    }
    
    return result;
}

ABPAL_DEC PAL_Error
AudioBluetoothGatewayClose(void)
{
    PAL_Error result = PAL_Ok;
    
    if (!IsA2DPConnected())
    {
        result = SendBluetoothAudioGatewayCommand(IOCTL_AG_CLOSE_AUDIO);
    }
    
    return result;
}

ABPAL_DEC PAL_Error
AudioBluetoothSetSpeakerVolume(uint16 level)
{
    PAL_Error result = PAL_ErrAudioGeneral;

    HANDLE device = OpenBluetoothAudio();
    if (device)
    {
        result = SendBluetoothAudioCommand(device, IOCTL_AG_SET_SPEAKER_VOL, &level, sizeof(level), NULL, 0);
        CloseBluetoothAudio(device);
    }

    return result;
}

ABPAL_DEC PAL_Error
AudioBluetoothGetSpeakerVolume(uint16* level)
{
    PAL_Error result = PAL_ErrAudioGeneral;

    HANDLE device = OpenBluetoothAudio();
    if (device)
    {
        result = SendBluetoothAudioCommand(device, IOCTL_AG_GET_SPEAKER_VOL, NULL, 0, level, sizeof(*level));
        CloseBluetoothAudio(device);
    }

    return result;
}


ABPAL_DEC PAL_Error
AudioBluetoothSetMicrophoneVolume(uint16 level)
{
    PAL_Error result = PAL_ErrAudioGeneral;

    HANDLE device = OpenBluetoothAudio();
    if (device)
    {
        result = SendBluetoothAudioCommand(device, IOCTL_AG_SET_MIC_VOL, &level, sizeof(level), NULL, 0);
        CloseBluetoothAudio(device);
    }

    return result;
}

ABPAL_DEC PAL_Error
AudioBluetoothGetMicrophoneVolume(uint16* level)
{
    PAL_Error result = PAL_ErrAudioGeneral;

    HANDLE device = OpenBluetoothAudio();
    if (device)
    {
        result = SendBluetoothAudioCommand(device, IOCTL_AG_GET_MIC_VOL, NULL, 0, level, sizeof(*level));
        CloseBluetoothAudio(device);
    }

    return result;
}

/*! @} */
