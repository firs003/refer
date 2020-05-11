/*
 * writer.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#ifndef _WRITER_H
#define _WRITER_H

#include <xdc/std.h>

#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include "TSpacket.h"	//steven 12-23-2009

//#define SLEEP_CUSTOMER_
#ifdef SLEEP_CUSTOMER_
#define INTERVAL_PER_SEND 10000	//10ms
#endif

/* Environment passed when creating the thread */
typedef struct WriterEnv {
    Rendezvous_Handle hRendezvousInit;
    Rendezvous_Handle hRendezvousCleanup;
    Pause_Handle      hPauseProcess;
    Fifo_Handle       hOutFifo;
    Fifo_Handle       hInFifo;
    Char             *videoFile;
    Int32             outBufSize;
    VisTS_params tsparams;		//steven 12-23-2009, update the TS package
    Int32             videoEnable;
#ifdef SLEEP_CUSTOMER_
	Int				  videoFrameRate;	//sleep for customer, ls 2013-06-20
#endif
} WriterEnv;

/* Thread function prototype */
extern Void *writerThrFxn(Void *arg);

#endif /* _WRITER_H */
