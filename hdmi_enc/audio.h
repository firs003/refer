/*
 * audio.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#ifndef _AUDIO_H
#define _AUDIO_H

#include <xdc/std.h>

#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Sound.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include "TSpacket.h"	//sp 02-09-2010

/* Environment passed when creating the thread */
typedef struct AudioEnv {
    Rendezvous_Handle       hRendezvousInit;
    Rendezvous_Handle       hRendezvousCleanup;
    Pause_Handle            hPauseProcess;
    Int                     soundSource; //0:aic31xx  1:adv7441 HDMI
    Sound_Input             soundInput;
    Char                   *audioEncoder;
    Char                   *audioFile;
    Char                   *engineName;
    Void                   *params;
    Void                   *dynParams;
    Int                     soundBitRate;
    Int                     sampleRate;
    Int                     volume;
    VisTS_params tsparams;		//sp 02-09-2010, update the TS package
} AudioEnv;

/* Thread function prototype */
extern Void *audioThrFxn(Void *arg);

#endif /* _AUDIO_H */
