#ifndef __DEMAND_H__
#define __DEMAND_H__

#include <ti/sdo/dmai/Rendezvous.h>

typedef struct DemandEnv {
    Rendezvous_Handle hRendezvousInit;
    Rendezvous_Handle hRendezvousCleanup;
} DemandEnv;

extern void *demandThrFxn(void *arg);

#endif /* __DEMAND_H__ */
