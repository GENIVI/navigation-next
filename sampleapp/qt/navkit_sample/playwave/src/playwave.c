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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */


#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#include "wav_parser.h"
#include "sndwav_common.h"
#include "playwave.h"

ssize_t SNDWAV_P_SaveRead(int fd, void *buf, size_t count)
{
    ssize_t result = 0, res;

    while (count > 0) {
        if ((res = read(fd, buf, count)) == 0)
            break;
        if (res < 0)
            return result > 0 ? result : res;
        count -= res;
        result += res;
        buf = (char *)buf + res;
    }
    return result;
}

void SNDWAV_Play(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav, int fd)
{
    int load, ret;
    off64_t written = 0;
    off64_t c;
    off64_t count = LE_INT(wav->chunk.length);

    load = 0;
    while (written < count) {
        /* Must read [chunk_bytes] bytes data enough. */
        do {
            c = count - written;
            if (c > sndpcm->chunk_bytes)
                c = sndpcm->chunk_bytes;
            c -= load;

            if (c == 0)
                break;
            ret = SNDWAV_P_SaveRead(fd, sndpcm->data_buf + load, c);
            if (ret < 0) {
                fprintf(stderr, "Error safe_read/n");
                exit(-1);
            }
            if (ret == 0)
                break;
            load += ret;
        } while ((size_t)load < sndpcm->chunk_bytes);

        /* Transfer to size frame */
        load = load * 8 / sndpcm->bits_per_frame;
        ret = SNDWAV_WritePcm(sndpcm, load);
        if (ret != load)
            break;

        ret = ret * sndpcm->bits_per_frame / 8;
        written += ret;
        load = 0;
    }
}

int PlayWave(const char * wavFileName)
{
    const char *filename = wavFileName;
    char *devicename = "default";
    int fd;
    WAVContainer_t wav;
    SNDPCMContainer_t playback;


    memset(&playback, 0x0, sizeof(playback));

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error open [%s]/n", filename);
        return -1;
    }

    if (WAV_ReadHeader(fd, &wav) < 0) {
        fprintf(stderr, "Error WAV_Parse [%s]/n", filename);
        goto Err;
    }

    if (snd_output_stdio_attach(&playback.log, stderr, 0) < 0) {
        fprintf(stderr, "Error snd_output_stdio_attach/n");
        goto Err;
    }

    if (snd_pcm_open(&playback.handle, devicename, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        fprintf(stderr, "Error snd_pcm_open [ %s]/n", devicename);
        goto Err;
    }

    if (SNDWAV_SetParams(&playback, &wav) < 0) {
        fprintf(stderr, "Error set_snd_pcm_params/n");
        goto Err;
    }
    snd_pcm_dump(playback.handle, playback.log);

    SNDWAV_Play(&playback, &wav, fd);

    snd_pcm_drain(playback.handle);

    close(fd);
    free(playback.data_buf);
    snd_output_close(playback.log);
    snd_pcm_close(playback.handle);
    return 0;

Err:
    close(fd);
    if (playback.data_buf) free(playback.data_buf);
    if (playback.log) snd_output_close(playback.log);
    if (playback.handle) snd_pcm_close(playback.handle);
    return -1;
}
