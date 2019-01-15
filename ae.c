#include "ae.h"
#include <sys/select.h>
#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include "zmalloc.h"
#include <errno.h>

aeEventLoop *aeCreateEventLoop(void){
    aeEventLoop *eventLoop;
    
    eventLoop = zmalloc(sizeof(*eventLoop));
    if(!eventLoop) return NULL;
    fflush(stdout);
    eventLoop->fileEvent = NULL;
    eventLoop->timeEvent = NULL;
    eventLoop->timeEventNextId = 0;
    eventLoop->stop = 0;
    return eventLoop;
}

void *aeEventLoopStop(aeEventLoop *eventLoop){
    eventLoop->stop = 1;
}

void *aeEventLoopDelete(aeEventLoop *eventLoop){
    zfree(eventLoop);
}

static aeTimeEvent *aeFindNearestTimeEvent(aeEventLoop *eventLoop);

void *aeEventLoopProcess(aeEventLoop *eventLoop, int flags){
    int maxfd = 0 ;
    fd_set rfds, wfds, efds;

    aeFileEvent *fe;
    aeTimeEvent *te;
    
    fe = eventLoop->fileEvent;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    while(fe){
        if(fe->mask & AE_READABLE)
            FD_SET(fe->fd, &rfds);
        if(fe->mask & AE_WRITABLE)
            FD_SET(fe->fd, &wfds);
        if(fe->mask & AE_EXCEPTION)
            FD_SET(fe->fd, &efds);
        if(maxfd < fe->fd) maxfd = fe->fd;
        fe = fe->next;
    }

    // get timeval of nearest timeEvent
    struct timeval tvNear, tvNow;
    aeTimeEvent *nearTE;
    nearTE = aeFindNearestTimeEvent(eventLoop);
    if(nearTE){
        gettimeofday(&tvNow);
        if(tvNow.tv_sec < nearTE->when_sec){
            if(tvNow.tv_usec < nearTE->when_msec * 1000){
                tvNear.tv_sec = nearTE->when_sec - tvNow.tv_sec;
                tvNear.tv_usec = nearTE->when_msec * 1000 - tvNow.tv_usec;
            }else{
                tvNear.tv_sec = nearTE->when_sec - tvNow.tv_sec - 1;
                tvNear.tv_usec = nearTE->when_msec * 1000 + 1000000 - tvNow.tv_usec;
            }
        }else{
            if(tvNow.tv_usec == nearTE->when_sec && tvNow.tv_usec < nearTE->when_msec * 1000){
                tvNear.tv_sec = 0;
                tvNear.tv_usec = nearTE->when_msec * 1000 - tvNow.tv_usec;
            }else{
                tvNear.tv_sec = 0;
                tvNear.tv_usec = 0;
            }
        }
    }

    // find FileEvent of ready fd, then process
    // no need to delete the fileEvent after process
    int select_ret;
    tvNear.tv_sec = 1;
    select_ret = select(maxfd + 1, &rfds, &wfds, &efds, &tvNear);
    printf("select %d \n", select_ret);
    fflush(stdout);
    if(select_ret == -1)
        printf("select error %s\n", strerror(errno));
    if(select_ret > 0){
        printf("select ok %d\n", select_ret);
        fe = eventLoop->fileEvent;
        while(fe){
            if((FD_ISSET(fe->fd, &rfds) && (fe->mask & AE_READABLE)) ||
                (FD_ISSET(fe->fd, &wfds) && (fe->mask & AE_WRITABLE)) ||
                (FD_ISSET(fe->fd, &efds) && (fe->mask & AE_EXCEPTION)) ){
                int mask;
                if(FD_ISSET(fe->fd, &rfds) && (fe->mask & AE_READABLE))
                    mask |= AE_READABLE;
                if(FD_ISSET(fe->fd, &wfds) && (fe->mask & AE_WRITABLE))
                    mask |= AE_WRITABLE;
                if(FD_ISSET(fe->fd, &efds) && (fe->mask & AE_EXCEPTION))
                    mask |= AE_EXCEPTION;

                fe->fileProc(eventLoop, fe->fd, fe->clientData, mask);
                fe = eventLoop->fileEvent;
                FD_CLR(fe->fd, &rfds);
                FD_CLR(fe->fd, &wfds);
                FD_CLR(fe->fd, &efds);
            }else{
                fe = fe->next;
            }
        }
    }

   // process time event, then delete them
   // 1. after every event processed, we start from begining, because some previous skipped events may be ready.
   // 2. we don't process events registerd by events processed in this loop by maxid

   int maxId = eventLoop->timeEventNextId - 1;
   te = eventLoop->timeEvent;
   while(te){
       // skip new events
       if(te->id > maxId){
           te = te->next;
       }

       gettimeofday(&tvNow);
       if(te->when_sec > tvNow.tv_sec || 
               (te->when_sec == tvNow.tv_sec && 
                te->when_msec * 1000 >= tvNow.tv_usec)){
           te->timeProc(eventLoop, te->id, te->clientData);
           aeDeleteTimeEvent(eventLoop, te->id);
           te = eventLoop->timeEvent;
       }else{
           te = te->next;
       }
   }
}


void *aeMain(aeEventLoop *eventLoop){
    eventLoop->stop = 0;
    while(!eventLoop->stop)
        aeEventLoopProcess(eventLoop, AE_ALLEVENT);
}

void *aeWait(aeEventLoop *eventLoop);

int aeCreateFileEvent(aeEventLoop *eventLoop, aeFileEvent *fileEvent){
    fileEvent->next = eventLoop->fileEvent;
    eventLoop->fileEvent = fileEvent;
    return AE_OK;
}

void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask){
    aeFileEvent *prev, *cur;

    prev = NULL;
    cur = eventLoop->fileEvent;

    while(cur){
        if(cur->fd == fd && cur->mask == mask){
            if(prev)
                prev->next = cur->next;
            else
                eventLoop->fileEvent->next = cur->next;

            if(cur->finalizerProc)
               cur->finalizerProc(eventLoop, cur->clientData); 
            zfree(cur);

            return;
        }
        prev = cur;
        cur = cur->next;
    }
}



int aeCreateTimeEvent(aeEventLoop *eventLoop, aeTimeEvent *timeEvent){
    timeEvent->id = eventLoop->timeEventNextId++;

    timeEvent->next = eventLoop->timeEvent;
    eventLoop->timeEvent = timeEvent;
    return AE_OK;
}


void aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id){
    aeTimeEvent *prev, *cur;

    prev = NULL;
    cur = eventLoop->fileEvent;

    while(cur){
        if(cur->id == id ){
            if(prev)
                prev->next = cur->next;
            else
                eventLoop->timeEvent->next = cur->next;

            if(cur->finalizerProc)
               cur->finalizerProc(eventLoop, cur->clientData); 
            zfree(cur);

            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

static aeTimeEvent *aeFindNearestTimeEvent(aeEventLoop *eventLoop){
    aeTimeEvent *te = eventLoop->timeEvent;
    aeTimeEvent *nearest = NULL;

    while(te){
        if(!nearest || nearest->when_sec > te->when_sec ||
                (nearest->when_sec == te->when_sec &&
                 nearest->when_msec > te->when_msec))
            nearest = te;
        te = te->next;
    }
    return nearest;
}

