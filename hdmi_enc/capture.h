/*
 * capture.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#ifndef _CAPTURE_H
#define _CAPTURE_H

#include <xdc/std.h>

#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include <ti/sdo/dmai/VideoStd.h>

/* Environment passed when creating the thread */
typedef struct CaptureEnv {
    Rendezvous_Handle hRendezvousInit;
    Rendezvous_Handle hRendezvousCapStd;
    Rendezvous_Handle hRendezvousCleanup;
    Rendezvous_Handle hRendezvousPrime;
    Pause_Handle      hPauseProcess;
    Fifo_Handle       hOutFifo;
    Fifo_Handle       hInFifo;
    VideoStd_Type     videoStd;
    Int32             imageWidth;
    Int32             imageHeight;
    Capture_Input     videoInput;
    Int32             videoEnable;
    Int               videoFrameRate;
    Int               videoLum;
} CaptureEnv;

struct capdim_save_params {
	int videoStd;
	unsigned short top;
	unsigned short left;
	unsigned short width;
	unsigned short height;
};

/* Thread function prototype */
extern Void *captureThrFxn(Void *arg);
extern Void *dynamicThrFxn(Void *arg);

#endif /* _CAPTURE_H */
