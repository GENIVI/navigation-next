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

 @file palaudioqnx.c
 @date 4/18/2012
 @defgroup PAL QNX Audio API

 @brief    QNX Audio API interface

 */
/*
 (C) Copyright 2012 by TeleCommunication Systems

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret
 as defined in section 499C of the penal code of the State of
 California. Use of pThis information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly prescribing
 the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

extern "C"
{
#include "palaudioqnx.h"
#include <abpalaudio.h>
#include <sys/stat.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <ftw.h>
#include <atomic.h>
#include <pthread.h>
#include <stdlib.h>
#include <bps/audiomixer.h>
#include <bps/mmrenderer.h>
#include <pthread.h>
#include <semaphore.h>
#include <audio/audio_manager_routing.h>
#include <process.h>
}
#include <nimaudiocombiner.h>


// define TABLET_OS_SIMULATOR to disable audio feature checks on simulator
//#define TABLET_OS_SIMULATOR

#define    MMR_CONTEXT_NAME_STR "com.tcs.nav"
#define    AUDIO_TEMP_DIR "tmp/" MMR_CONTEXT_NAME_STR
#define    FILE_SCHEME_PREFIX_LEN    7  // file://
#define    MMR_CONTEXT_NAME_LENGTH   50

/*
 * Internal command set for Player
 */
typedef enum
{
    QNXP_CMD_NONE,
    QNXP_CMD_SET_PARAM,
    QNXP_CMD_PLAY,
    QNXP_CMD_PLAYLIST,
    QNXP_CMD_STOP
} QNXPlayerCommands;

/*
 * Internal command parameters for commands
 */
typedef enum
{
    QNXP_PARAM_NONE,
    QNXP_PARAM_VOLUME,
    QNXP_PARAM_OUTPUT_TYPE
} QNXPlayerParam;

/*
 * Command structure for Player thread
 */
typedef    struct
{
    QNXPlayerCommands         command;    // command
    QNXPlayerParam            param;      // command parameter
    const void                *value;     // parameter value
} QNXPlayerCommand;

struct QNXPlayer
{
    unsigned               refCnt;            // reference count for singleton structure
    mmr_connection_t       *pMmrConnection;   // MMR connection
    mmr_context_t          *pMmrContext;      // MMR context
    mmrenderer_monitor_t   *pMMRMonitor;      // bps event monitor
    int                    iOutput;           // MMR output

    unsigned               bStopRequested;    // Stop request flag for player thread
    pthread_t              tThread;           // Player thread
    pthread_mutex_t        tQueMutex;         // Command queue mutex
    sem_t                  tQueSemaphore;     // Command queue semaphore
    void                   *commands;         // Command queue

    nb_boolean             playCanceled;
    ABPAL_AudioPlayerCallback *cb;            // player callback
    void                      *cbdata;        // callback data
};

static char MMR_CONTEXT_NAME[MMR_CONTEXT_NAME_LENGTH]; // unique context name used for MM renderer
static const long  COMMAND_WAIT_TIME = 50000000; // 50 msec wait on que mutex,semaphore poll

// NB: simulate singleton due to multiple navigation sessions(and navigationaudiomanager initializations)
static ABPAL_QNXPlayer    *s_QNXPlayer = 0;

static PAL_Error initMMR();
static void    cleanupMMR(ABPAL_QNXPlayer *pPlayer);
static int    getMMRError(ABPAL_QNXPlayer *p);
static PAL_Error makeTempDir();
static PAL_Error cleanupTempDir();
static int    ftwCallback(const char *fpath, const struct stat *sb,int typeflag);
static void    *playerThread(void *);
static char *makeTempFile(   const byte* buffer, size_t bufferSize, ABPAL_AudioFormat fmt);
static PAL_Error addCommand(ABPAL_QNXPlayer *pPlayer, QNXPlayerCommand *c);
static struct timespec *addTimeout2Now(struct timespec *wt, long timeout);
static PAL_Error setVolume(ABPAL_QNXPlayer *p, int level);
static PAL_Error stopPlayback(ABPAL_QNXPlayer *p);
static PAL_Error playFile(ABPAL_QNXPlayer *pPlayer, const char *filename);


PAL_Error setOutputType(ABPAL_QNXPlayer *p, int outputType)
{
    PAL_Error err = PAL_Ok;
    if (!p)
    {
        return PAL_ErrBadParam;
    }

#ifndef TABLET_OS_SIMULATOR
    if(p->pMmrContext)
    {
        strm_dict_t* dict = strm_dict_new();
        if (dict)
        {
            strm_dict_t* dict_audio_type = strm_dict_set(dict, "audio_type", audio_manager_get_name_from_type(outputType ?
                    AUDIO_TYPE_VOICE : AUDIO_TYPE_TEXT_TO_SPEECH));
            if (dict_audio_type)
            {
                float level = -1.0;

                if (audiomixer_get_output_level(AUDIOMIXER_OUTPUT_DEFAULT, &level) != BPS_SUCCESS)
                {
                    ERRLOG(errno,"audiomixer set volume");
                }
                //dictionary handle gets consumed by pThis call, even on failure
                if (mmr_output_parameters(p->pMmrContext, p->iOutput, dict_audio_type) < 0)
                {
                    err = PAL_ErrAudioGeneral;
                    ERRLOG(getMMRError(p), "mmr_output_parameters" );
                }
                if (audiomixer_set_output_level( AUDIOMIXER_OUTPUT_DEFAULT, level) != BPS_SUCCESS)
                {
                    ERRLOG(errno,"audiomixer set volume");
                }
            }
            else
            {
                ERRLOG(BPS_FAILURE, "strm_dict_set" );
                err = PAL_ErrNoInit;
                strm_dict_destroy(dict);
            }
        }
        else
        {
            ERRLOG(BPS_FAILURE, "strm_dict_new" );
            err = PAL_ErrNoInit;
        }
    }
#endif // TABLET_OS_SIMULATOR
    return err;
}

/*
 * @brief Initialize Native MM renderer
 */
PAL_Error initMMR(ABPAL_QNXPlayer *pThis)
{
    PAL_Error err = PAL_Ok;

    CONLOG(PAL_LogLevelInfo, "init");
    if(!pThis)
    {
        return PAL_ErrBadParam;
    }

    if(makeTempDir() != PAL_Ok)
    {
        return PAL_ErrFileFailed;
    }

#ifndef    TABLET_OS_SIMULATOR
    if (bps_initialize() == BPS_SUCCESS)
    {
        pThis->pMmrConnection = mmr_connect(0);
        if (pThis->pMmrConnection)
        {
            CONLOG(PAL_LogLevelInfo, "Creating context %s", MMR_CONTEXT_NAME);

            // first try to open existing context for player
            pThis->pMmrContext = mmr_context_open(pThis->pMmrConnection, MMR_CONTEXT_NAME);
            if(!pThis->pMmrContext)
            {
                CONLOG(PAL_LogLevelInfo, "context_open err=%d",errno);
                // If open failed then assume context does not exist and try to create one
                pThis->pMmrContext = mmr_context_create(pThis->pMmrConnection, MMR_CONTEXT_NAME, 0, 0777);
            }
            else
            {
                CONLOG(PAL_LogLevelInfo, "context %s opened", MMR_CONTEXT_NAME);
                err = PAL_ErrAudioDeviceOpen;
            }

            if (!err && pThis->pMmrContext)
            {
                CONLOG(PAL_LogLevelInfo, "context %s OK", MMR_CONTEXT_NAME);

                pThis->iOutput = mmr_output_attach(pThis->pMmrContext, "audio:default", "audio");
                if (pThis->iOutput < 0)
                {
                    ERRLOG(getMMRError(pThis), "mmr_output_attach" );
                    err= PAL_ErrNoInit;
                }

                setOutputType(pThis, 0);

                pThis->pMMRMonitor = mmrenderer_request_events(MMR_CONTEXT_NAME, 0, 0);
                if (!pThis->pMMRMonitor)
                {
                    ERRLOG(getMMRError(pThis),"mmrenderer_request_events");
                    err = PAL_ErrNoInit;
                }
                if( audiomixer_request_events(0) != BPS_SUCCESS )
                {
                    ERRLOG(errno,"audiomixer_request_events");
                    err = PAL_ErrNoInit;
                }
            }
            else
            {
                ERRLOG(errno,"mmr_context_create");
                err = PAL_ErrAudioDeviceOpen;
            }
        }
        else
        {
            ERRLOG(errno,"mmr_connect");
            err = PAL_ErrNoInit;
        }
    }
    else
    {
        ERRLOG(errno, "bps init");
        err = PAL_ErrNoInit;
    }
#endif // TABLET_OS_SIMULATOR
    return err;
}


void cleanupMMR(ABPAL_QNXPlayer *pPlayer)
{
    CONLOG(PAL_LogLevelInfo, "cleanup");
#ifndef TABLET_OS_SIMULATOR
    audiomixer_stop_events(0);
    if (pPlayer->pMmrConnection)
    {
        if (pPlayer->pMmrContext)
        {
            mmr_input_detach(pPlayer->pMmrContext);
            if (pPlayer->iOutput != -1)
            {
                mmr_output_detach(pPlayer->pMmrContext, pPlayer->iOutput);
            }

            if (pPlayer->pMMRMonitor)
            {
                mmrenderer_stop_events(pPlayer->pMMRMonitor);
                pPlayer->pMMRMonitor = 0;
            }

            CONLOG(PAL_LogLevelInfo, "Destroy context %s", MMR_CONTEXT_NAME);
            mmr_context_destroy(pPlayer->pMmrContext);
            pPlayer->pMmrContext = 0;
        }

        mmr_disconnect(pPlayer->pMmrConnection);
        pPlayer->pMmrConnection = 0;
    }
    bps_shutdown();
#endif // TABLET_OS_SIMULATOR
    cleanupTempDir();
}

/*! Adds command to Player queue

@brief Adds command @c to player @pPlayer' queue
@parameter pPlayer QNXPlayer pointer
@parameter c QNXPlayerCommand pointer
@return PAL_Error
*/

PAL_Error addCommand(ABPAL_QNXPlayer *pPlayer, QNXPlayerCommand *c)
{
    PAL_Error err = PAL_Ok;
    struct timespec  wt;
    int       r;

    if(!pPlayer || !c)
    {
        return PAL_ErrBadParam;
    }

    // Try to lock queue mutex in COMMAND_WAIT_TIME(default 50) msecs.
    r = pthread_mutex_timedlock(& pPlayer->tQueMutex, addTimeout2Now(&wt, COMMAND_WAIT_TIME));
    if(r == ETIMEDOUT)
    {
        return PAL_ErrAudioBusy;
    }
    if(!r)
    {
        // if queue is empty(commands is 0) then assign command to player queue
        if(pPlayer->commands)
        {
            err = PAL_ErrAudioBusy;
        }
        else
        {
            pPlayer->commands = (void *) c;
        }
        pthread_mutex_unlock(& pPlayer->tQueMutex);

        // notify player thread about available command
        sem_post(&pPlayer->tQueSemaphore);
    }
    return err;
}

PAL_Error QNXPlayerCreate(ABPAL_QNXPlayer **p)
{
    PAL_Error err = PAL_Ok;
    ABPAL_QNXPlayer *pThis = 0;

    if (!p)
        return PAL_ErrBadParam;
    *p = 0;


    // return singleton if not first call
    if(s_QNXPlayer)
    {
        CONLOG( PAL_LogLevelInfo, "Create: add: #ref=%d", s_QNXPlayer->refCnt);
        atomic_add( &s_QNXPlayer->refCnt,1);
        *p = s_QNXPlayer;
        return PAL_Ok;
    }
       CONLOG( PAL_LogLevelInfo, "Create: new");
    pThis = (ABPAL_QNXPlayer *) nsl_malloc(sizeof(ABPAL_QNXPlayer));
    if (!pThis)
    {
        return PAL_ErrNoMem;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    if(pthread_mutex_init(& pThis->tQueMutex, 0))
    {
        err = PAL_ErrNoInit;
    }

    if(err == PAL_Ok && sem_init(&pThis->tQueSemaphore, 0, 0) != EOK)
    {
        perror("Sem_init");
        ERRLOG(errno,"sem init");
        pthread_mutex_destroy(& pThis->tQueMutex);
        err = PAL_ErrNoInit;
    }

    if(err == PAL_Ok && pthread_create(& pThis->tThread, 0, playerThread, (void *)pThis) != EOK)
    {
        sem_destroy(&pThis->tQueSemaphore);
        pthread_mutex_destroy(& pThis->tQueMutex);
        err = PAL_ErrNoInit;
    }

    if(err == PAL_Ok)
    {
        pThis->refCnt = 1;
        *p = s_QNXPlayer = pThis;
    }

    nsl_snprintf(MMR_CONTEXT_NAME, MMR_CONTEXT_NAME_LENGTH, "%s_%d", MMR_CONTEXT_NAME_STR, getpid());

    return err;
}

PAL_Error QNXPlayerDestroy(ABPAL_QNXPlayer *pPlayer)
{
    PAL_Error err = PAL_Ok;
    struct timespec wt;

    if (!pPlayer || !s_QNXPlayer)
    {
        return PAL_ErrBadParam;
    }
    QNXPlayerStop(pPlayer);

    if (pthread_mutex_timedlock(& pPlayer->tQueMutex, const_cast<const timespec*>(addTimeout2Now(&wt,COMMAND_WAIT_TIME))) != EOK)
    {
        return PAL_ErrAudioBusy;
    }
    CONLOG( PAL_LogLevelInfo, "Destroy: #ref=%d", s_QNXPlayer->refCnt );
    atomic_sub( &s_QNXPlayer->refCnt,1);
    if(s_QNXPlayer->refCnt)
    {
        pthread_mutex_unlock(& pPlayer->tQueMutex);
        return PAL_ErrQueueNotEmpty;
    }

    // set stop request flag for player thread
    atomic_set(& pPlayer->bStopRequested, 1);
    pthread_join(pPlayer->tThread, 0);
    sem_destroy(& pPlayer->tQueSemaphore);

    pthread_mutex_unlock(& pPlayer->tQueMutex);
    pthread_mutex_destroy(& pPlayer->tQueMutex);

    nsl_free(s_QNXPlayer);
    s_QNXPlayer = 0;
    return err;
}

PAL_Error QNXPlayerStop(ABPAL_QNXPlayer *pPlayer)
{
    CONLOG( PAL_LogLevelInfo, "Stop: #ref=%d", pPlayer->refCnt );
    PAL_Error err = PAL_Ok;
    if (!pPlayer)
    {
        return PAL_ErrBadParam;
    }
    QNXPlayerCommand *c = (QNXPlayerCommand *) nsl_malloc(sizeof(QNXPlayerCommand));
    if(!c)
    {
        err = PAL_ErrNoMem;
    }
    else
    {
        nsl_memset(c, 0, sizeof(QNXPlayerCommand));
        c->command = QNXP_CMD_STOP;
        err = addCommand(pPlayer, c);
    }
    return err;
}

PAL_Error QNXPlayerGetVolume(ABPAL_QNXPlayer *pPlayer, int32 *level)
{
    float l = -1.0;

    if( !level )
    {
        return PAL_ErrBadParam;
    }

    if( audiomixer_get_output_level( AUDIOMIXER_OUTPUT_DEFAULT, &l) != BPS_SUCCESS )
    {
        ERRLOG(errno,"Get Volume");
        return PAL_ErrAudioGeneral;
    }
    else
    {
        *level = (int32)l;
    }
    return PAL_Ok;
}

PAL_Error QNXPlayerSetVolume(ABPAL_QNXPlayer *pPlayer, int32 level)
{
    PAL_Error err = PAL_Ok;
    if (!pPlayer)
    {
        return PAL_ErrBadParam;
    }

    QNXPlayerCommand *c = (QNXPlayerCommand *) nsl_malloc(sizeof(QNXPlayerCommand));
    if(!c)
    {
        err = PAL_ErrNoMem;
    }
    else
    {
        nsl_memset(c, 0, sizeof(QNXPlayerCommand));
        c->command = QNXP_CMD_SET_PARAM;
        c->param = QNXP_PARAM_VOLUME;
        c->value = (void*)level;
        err = addCommand(pPlayer, c);
    }
    return err;
}

PAL_Error QNXPlayerSetOutput(ABPAL_QNXPlayer *pPlayer, int32 *output)
{
    PAL_Error err = PAL_Ok;
    if (!pPlayer)
    {
        return PAL_ErrBadParam;
    }

    QNXPlayerCommand *c = (QNXPlayerCommand *) nsl_malloc(sizeof(QNXPlayerCommand));
    if(!c)
    {
        err = PAL_ErrNoMem;
    }
    else
    {
        nsl_memset(c, 0, sizeof(QNXPlayerCommand));
        c->command = QNXP_CMD_SET_PARAM;
        c->param = QNXP_PARAM_OUTPUT_TYPE;
        c->value = (void*)output;
        err = addCommand(pPlayer, c);
    }
    return err;
}

PAL_Error QNXPlayerPlay(ABPAL_QNXPlayer *pPlayer, byte *data, uint32 nSize, ABPAL_AudioFormat format,
                        ABPAL_AudioPlayerCallback *cb, void *cbdata)
{
    PAL_Error err = PAL_Ok;
    if (!pPlayer)
    {
        return PAL_ErrBadParam;
    }

    const char *tmpFile=0;

    pPlayer->cb = cb;
    pPlayer->cbdata = cbdata;

    CONLOG( PAL_LogLevelInfo, "Play: CB=%p/%p", cb, cbdata);
    // write data to play in the temporary audio file
    tmpFile = makeTempFile(data, (size_t) nSize, format);
    if(tmpFile)
    {
        QNXPlayerCommand *c = (QNXPlayerCommand *) nsl_malloc(sizeof(QNXPlayerCommand));
        if(!c)
        {
            err = PAL_ErrNoMem;
        }
        else
        {
            nsl_memset(c, 0, sizeof(QNXPlayerCommand));
            c->command = QNXP_CMD_PLAY;
            c->value = tmpFile; // must be freed after use, in addCommand
            err = addCommand(pPlayer, c);
        }
    }
    else
    {
        err = PAL_ErrFileFailed;
    }

    return err;
}

/*! Handler for playback events
*/
void bpsEventHandler(bps_event_t *event)
{
    ABPAL_AudioState state = ABPAL_AudioState_Unknown;
    int iBpsDomain = bps_event_get_domain(event);
    int iMessageCode = bps_event_get_code(event);

    if (!s_QNXPlayer)
    {
        return;
    }
    if (iBpsDomain == mmrenderer_get_domain())
    {
        if (iMessageCode == MMRENDERER_STATE_CHANGE)
        {
            const mmrenderer_state_t newState = mmrenderer_event_get_state(event);
            const mmr_error_info_t *ei = mmrenderer_event_get_error( event );
            if (ei && newState == MMR_STOPPED)
            {
                if (s_QNXPlayer->playCanceled)
                {
                    state = ABPAL_AudioState_Cancel;
                }
                else
                {
                    state = ABPAL_AudioState_Ended;
                }

                if (s_QNXPlayer->cb)
                {
                    CONLOG( PAL_LogLevelInfo, "BPSEvent: calling cb %p/%p, state %d", s_QNXPlayer->cb, s_QNXPlayer->cbdata, state );
                    s_QNXPlayer->cb(s_QNXPlayer->cbdata, state);
                    s_QNXPlayer->cb = NULL;                     // we could get the same event several times
                                                                // to call callback only once set it to NULL
                                                                // it will be updated on next play command
                }
                s_QNXPlayer->playCanceled = FALSE;
            }
        }
    }
    else if( iBpsDomain == audiomixer_get_domain() )
    {
        if( iMessageCode == AUDIOMIXER_INFO )
        {
            float fl = audiomixer_event_get_output_level( event, AUDIOMIXER_OUTPUT_DEFAULT );
            CONLOG( PAL_LogLevelInfo, "audiomixer volume level %f", fl );
        }
    }
    else
    {
        CONLOG( PAL_LogLevelInfo, "BPS event %d from domain %d", iMessageCode, iBpsDomain );
    }
}

/*! Internal function to stop playback

pThis function is called from player thread to stop playback.

@return PAL_Error
*/

PAL_Error stopPlayback(ABPAL_QNXPlayer *p)
{
    PAL_Error err = PAL_Ok;
    if (!p)
    {
        return PAL_ErrBadParam;
    }

    p->playCanceled = TRUE;
#ifndef TABLET_OS_SIMULATOR
    mmr_stop(p->pMmrContext);
#endif // TABLET_OS_SIMULATOR
    return err;
}

/*! Internal function that sets playback volume level.

pThis function is called from player thread to set volume level from the stored level.


@return PAL_Error
*/

PAL_Error setVolume(ABPAL_QNXPlayer *p, int level)
{
    PAL_Error err = PAL_Ok;
    if (!p)
    {
        return PAL_ErrBadParam;
    }

#ifndef TABLET_OS_SIMULATOR
    if(p->pMmrContext)
    {
        CONLOG( PAL_LogLevelInfo, "Setting volume to %d", level);
        if (audiomixer_set_output_level( AUDIOMIXER_OUTPUT_DEFAULT, (float)level) != BPS_SUCCESS)
        {
            err = PAL_ErrAudioGeneral;
            ERRLOG(errno,"audiomixer set volume");
        }
    }
#endif // TABLET_OS_SIMULATOR
    return err;
}

int getMMRError(ABPAL_QNXPlayer *p)
{
    const mmr_error_info_t *ei = mmr_error_info(p->pMmrContext);
    return ei ? ei->error_code : BPS_SUCCESS;
}

/*! Create temporary audio file.

Internal function that creates temporary audio file with @buffer data of size
@bufferSize in AUDIO_TEMP_DIR directory with specific extension for format @fmt.

@return char* created temporary filename
*/

char *makeTempFile(   const byte* buffer, size_t bufferSize, ABPAL_AudioFormat fmt)
{
    char  fnTemplate[PATH_MAX] = "file://";
    char *tmpname=0;
    int      fd;

    PAL_Error    err = PAL_Ok;

    if (!buffer || !bufferSize)
    {
        return 0;
    }

    getcwd(fnTemplate + FILE_SCHEME_PREFIX_LEN, PATH_MAX - FILE_SCHEME_PREFIX_LEN);
    nsl_strlcat(fnTemplate, "/", sizeof(fnTemplate));
    nsl_strlcat(fnTemplate, AUDIO_TEMP_DIR, sizeof(fnTemplate));
    tmpname = tempnam(fnTemplate + FILE_SCHEME_PREFIX_LEN, "nav-");
    nsl_strlcpy(fnTemplate + FILE_SCHEME_PREFIX_LEN, tmpname, sizeof(fnTemplate)-FILE_SCHEME_PREFIX_LEN);

    if(tmpname)
    {
        switch (fmt)
        {
            case ABPAL_AudioFormat_AMR:
                nsl_strcat(fnTemplate, ".amr");
                break;
            case ABPAL_AudioFormat_QCP:
                nsl_strcat(fnTemplate, ".qcp");
                break;
            case ABPAL_AudioFormat_WAV:
                nsl_strcat(fnTemplate, ".wav");
                break;
            case ABPAL_AudioFormat_AAC:
                // TODO: Temporary solution for resolving audio bugs in MMRenderer
                nsl_strcat(fnTemplate, ".m4af");
                break;
            case ABPAL_AudioFormat_MP3:
                nsl_strcat(fnTemplate, ".mp3");
                break;
            default:
                return 0;
        }

        if((fd = open(fnTemplate + FILE_SCHEME_PREFIX_LEN, O_CREAT | O_WRONLY, 0644)) >= 0)
        {
            if (write(fd, buffer, bufferSize) != bufferSize)
            {
                err = PAL_ErrFileFailed;
            }
            close(fd);
        }
        else
        {
            ERRLOG( errno, "Open temp %s for writing", fnTemplate+FILE_SCHEME_PREFIX_LEN);
            err = PAL_ErrFileFailed;
        }
    }
    else
    {
        ERRLOG( errno, "Make temp name");
        err = PAL_ErrFileFailed;
    }

    if(err != PAL_Ok)
    {
        return 0;
    }
    return nsl_strdup(fnTemplate);
}

/*! Internal function that plays file.

pThis function is called from player thread to start playback of file @filename.

@return PAL_Error
*/

PAL_Error playFile(ABPAL_QNXPlayer *pPlayer, const char *filename)
{
    PAL_Error err = PAL_Ok;
    // todo: check autolist feature
    const char *ftype = "track";

    if(nsl_strcmp(filename + nsl_strlen(filename) - 4, ".m3u") == 0) // pThis is playlist
    {
        ftype = "playlist";
    }
#ifndef TABLET_OS_SIMULATOR
    CONLOG(PAL_LogLevelInfo, "attach file '%s'", filename);
    if (mmr_input_attach(pPlayer->pMmrContext, filename, ftype) != -1)
    {
        pPlayer->playCanceled = FALSE;
        if (mmr_play(pPlayer->pMmrContext) == -1)
        {
            err = PAL_ErrAudioGeneral;
            ERRLOG(getMMRError(pPlayer),"Play");
        }
    }
    else
    {
        ERRLOG(getMMRError(pPlayer),"Attach '%s'",filename);
        err = PAL_ErrAudioGeneral;
    }
#endif // TABLET_OS_SIMULATOR
    return err;
}

/*! Internal function that creates temporary directory.

pThis function is called from @initMMR and creates temporary directory
AUDIO_TEMP_DIR in <app>/tmp.

@return PAL_Error
*/

PAL_Error makeTempDir()
{
    int    err = mkdir(AUDIO_TEMP_DIR, 0777);
    if (err == -1 && errno != EEXIST)
    {
        ERRLOG(errno,"mkdir %s", AUDIO_TEMP_DIR );
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

/*! Internal function that deletes all temporary files.

pThis function is called from QNXPlayerDestroy/cleanup and removes teporary audio files.

@return PAL_Error
*/
PAL_Error cleanupTempDir()
{
    return PAL_Ok;
    return ftw(AUDIO_TEMP_DIR, ftwCallback, 2) == 0 ? PAL_Ok : PAL_ErrFileFailed;
}

int    ftwCallback(const char *fpath, const struct stat *sb,int typeflag)
{
    if (fpath && (typeflag & FTW_F))
    {
        unlink(fpath);
    }
    return 0;
}

/*! Player thread function.

pThis is main player thread function that initializes/cleanup MMR, monitors
command queue and executes player' MMR commands.

@return void*
*/

void    *playerThread(void *p)
{
    PAL_Error        result = PAL_Ok;
    ABPAL_QNXPlayer *pPlayer = (ABPAL_QNXPlayer *)p;
    struct    timespec    wt = {0,0};
    int        r;

    if(!pPlayer)
        return (void*) PAL_ErrBadParam;

    pthread_setname_np(0, "NavAudioPlayer");

    // Lock queue mutex during initialization.
    r = pthread_mutex_timedlock(& pPlayer->tQueMutex, addTimeout2Now(&wt,COMMAND_WAIT_TIME));
    if(r == EOK)
    {
        result = initMMR(pPlayer);

        // initialization complete, unlock mutex
        pthread_mutex_unlock(& pPlayer->tQueMutex);

        if(result == PAL_Ok)
        {
            QNXPlayerCommand *c=0;

            // main thread monitoring loop
            while(! pPlayer->bStopRequested)
            {
                bps_event_t *event = NULL;

                c = 0;

                bps_get_event(&event, 0);
                if (event)
                {
                    bpsEventHandler(event);
                }

                // check whether semaphore is positive
                if(sem_timedwait(&pPlayer->tQueSemaphore, addTimeout2Now(&wt,COMMAND_WAIT_TIME)) == EOK)
                {
                    // There is a command available, try to lock queue mutex
                    r = pthread_mutex_timedlock(& pPlayer->tQueMutex, addTimeout2Now(&wt,COMMAND_WAIT_TIME));
                    if(r == EOK)
                    {
                        // store command in local variable
                        c = (QNXPlayerCommand *) pPlayer->commands;

                        // empty the queue
                        pPlayer->commands = 0;
                        pthread_mutex_unlock(& pPlayer->tQueMutex);
                    }

                    // If command is available then execute it
                    if(c)
                    {
                        switch(c->command)
                        {
                        case QNXP_CMD_PLAY:
                        case QNXP_CMD_PLAYLIST:
                            if(c->value)
                            {
                                playFile(pPlayer, (const char *)c->value);
                                nsl_free((void *) c->value);
                            }
                            break;
                        case QNXP_CMD_STOP:
                            stopPlayback(pPlayer);
                            break;
                        case QNXP_CMD_SET_PARAM:
                            switch(c->param)
                            {
                            case QNXP_PARAM_VOLUME:
                                setVolume(pPlayer, (int) c->value );
                                break;
                            case QNXP_PARAM_OUTPUT_TYPE:
                                setOutputType(pPlayer, (int) c->value);
                                break;
                            default:
                                break;
                            }
                            break;
                        default:
                            break;
                        }
                        // free memory for passed in command
                        nsl_free(c);
                    }
                } // sem_wait
            } // while
        } // if initMMR ok
        else
        {
            ERRLOG( result, "initMMR");
        }
    } // lock on init
    else
    {
        ERRLOG(errno,"init: could not lock");
    }

    // clean up MMR on terminate
    cleanupMMR(pPlayer);
    return (void *)result;
}

/*! Add specified nanoseconds to current time in timespec structire

Adds @timeout nanoseconds to current time and store result in passed in timespec @wt
structure pointer.

@return timespec* Pointer to passed in timespec structure.
*/

struct timespec *addTimeout2Now(struct timespec *wt, long timeout)
{
    if(wt)
    {
        long nsecs = wt->tv_nsec + timeout;
        clock_gettime(CLOCK_REALTIME, wt);
        wt->tv_nsec = nsecs % 1000000000;
        wt->tv_sec = wt->tv_sec + nsecs / 1000000000;
    }
    return wt;
}

/*! @} */


