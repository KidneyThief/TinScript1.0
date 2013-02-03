// ------------------------------------------------------------------------------------------------
//  The MIT License
//  
//  Copyright (c) 2013 Tim Andersen
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without
//  restriction, including without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// TinScheduler.cpp
// ------------------------------------------------------------------------------------------------

#include "stdafx.h"

#include "TinScript.h"
#include "TinRegistration.h"
#include "TinInterface.h"
#include "TinExecute.h"
#include "TinScheduler.h"

// ------------------------------------------------------------------------------------------------
// -- forward declares
extern unsigned int GetCurrentSimTime();

namespace TinScript {

CScheduler::CCommand* CScheduler::head = NULL;

void CScheduler::Initialize() {
}

void CScheduler::Shutdown() {
    // -- clean up all pending scheduled events
    while(head) {
        CCommand* next = head->next;
        delete head;
        head = next;
    }
}

void CScheduler::Update(unsigned int curtime) {

    // -- execute all commands scheduled for dispatch by this time
    while(head && head->dispatchtime <= curtime) {

        // -- get the current command, and remove it from the list - now, before we execute,
        // -- since executing this command could 
        CCommand* curcommand = head;
        if(curcommand->next)
            curcommand->next->prev = NULL;
        head = curcommand->next;

        // -- dispatch the command - see if it's a direct function call, or a command buf
        if(curcommand->funchash != 0) {
            ExecuteScheduledFunction(curcommand->objectid, curcommand->funchash,
                                     curcommand->funccontext);
        }
        else {
            if(curcommand->objectid > 0) {
                int dummy = 0;
                ObjExecF(curcommand->objectid, dummy, curcommand->commandbuf);
            }
            else {
                // $$$TZA is there anything we can do with the result?
                ExecCommand(curcommand->commandbuf);
            }
        }

        // -- delete the command
        delete curcommand;
    }
}

void CScheduler::CancelObject(unsigned int objectid) {
    if(objectid == 0)
        return;
    Cancel(objectid, 0);
}

void CScheduler::CancelRequest(int reqid) {
    if(reqid <= 0)
        return;
    Cancel(0, reqid);
}

void CScheduler::Cancel(unsigned int objectid, int reqid) {
    // -- loop through and delete any schedules pending for this object
    CCommand** prevcommand = &head;
    CCommand* curcommand = head;
    while(curcommand) {
        if(curcommand->objectid == objectid || curcommand->reqid == reqid) {
            *prevcommand = curcommand->next;
            delete curcommand;
            curcommand = *prevcommand;
        }
        else {
            prevcommand = &curcommand->next;
            curcommand = curcommand->next;
        }
    }
}

void CScheduler::Dump() {
    // -- loop through and delete any schedules pending for this object
    CCommand* curcommand = head;
    while(curcommand) {
        printf("ReqID: %d, ObjID: %d, Command: %s\n", curcommand->reqid, curcommand->objectid,
               curcommand->commandbuf);
        curcommand = curcommand->next;
    }
}

CScheduler::CCommand::CCommand(int _reqid, unsigned int _objectid, unsigned int _dispatchtime,
                               const char* _command) {
    // --  members copy the command members
    reqid = _reqid;
    objectid = _objectid;
    dispatchtime = _dispatchtime;
    SafeStrcpy(commandbuf, _command, kMaxTokenLength);

    // -- command string, null out the direct function call members
    funchash = 0;
    funccontext = NULL;
}

CScheduler::CCommand::CCommand(int _reqid, unsigned int _objectid, unsigned int _dispatchtime,
                               unsigned int _funchash) {
    // --  members copy the command members
    reqid = _reqid;
    objectid = _objectid;
    dispatchtime = _dispatchtime;
    commandbuf[0] = '\0';

    // -- command string, null out the direct function call members
    funchash = _funchash;
    funccontext = new CFunctionContext();
}

CScheduler::CCommand::~CCommand() {
    // clean up the function context, if it exists
    if(funccontext)
        delete funccontext;
}

static int gScheduleID = 0;
int CScheduler::Schedule(unsigned int objectid, int delay, const char* commandstring) {
    ++gScheduleID;

    // -- ensure we have a valid command string
    if(!commandstring || !commandstring[0])
        return 0;

    // -- calculate the dispatch time - enforce a one-frame delay
    unsigned int dispatchtime = GetCurrentSimTime() + (delay > 0 ? delay : 1);

    // -- create the new commmand
    CCommand* newcommand = new CCommand(gScheduleID, objectid, dispatchtime, commandstring);

    // -- see if it goes at the front of the list
    if(!head || dispatchtime <= head->dispatchtime) {
        newcommand->next = head;
        newcommand->prev = NULL;
        if(head)
            head->prev = newcommand;
        head = newcommand;
    }
    else {
        // -- insert it into the list, in after curschedule
        CCommand* curschedule = head;
        while(curschedule->next && curschedule->dispatchtime < dispatchtime)
            curschedule = curschedule->next;
        newcommand->next = curschedule->next;
        newcommand->prev = curschedule;
        if(curschedule->next)
            curschedule->next->prev = newcommand;
        curschedule->next = newcommand;
    }

    // -- return the request id, so we have a way to cancel
    return newcommand->reqid;
}

CScheduler::CCommand* CScheduler::ScheduleCreate(unsigned int objectid, int delay,
                                                 unsigned int funchash) {
    ++gScheduleID;

    // -- calculate the dispatch time - enforce a one-frame delay
    unsigned int dispatchtime = GetCurrentSimTime() + (delay > 0 ? delay : 1);

    // -- create the new commmand
    CCommand* newcommand = new CCommand(gScheduleID, objectid, dispatchtime, funchash);

    // -- see if it goes at the front of the list
    if(!head || dispatchtime <= head->dispatchtime) {
        newcommand->next = head;
        newcommand->prev = NULL;
        if(head)
            head->prev = newcommand;
        head = newcommand;
    }
    else {
        // -- insert it into the list, in after curschedule
        CCommand* curschedule = head;
        while(curschedule->next && curschedule->dispatchtime < dispatchtime)
            curschedule = curschedule->next;
        newcommand->next = curschedule->next;
        newcommand->prev = curschedule;
        if(curschedule->next)
            curschedule->next->prev = newcommand;
        curschedule->next = newcommand;
    }

    // -- return the actual commmand object, since we'll be updating the parameter values
    return newcommand;
}

int CScheduler::Thread(int reqid, unsigned int objectid, int delay, const char* commandstring) {
    CancelRequest(reqid);
    int newreqid = Schedule(objectid, delay, commandstring);
    return newreqid;
}

} // TinScript

// ------------------------------------------------------------------------------------------------
// script registered interface

REGISTER_FUNCTION_P3(Schedule, TinScript::CScheduler::Schedule, int, unsigned int, int, const char*);
REGISTER_FUNCTION_P4(ScheduleThread, TinScript::CScheduler::Thread, int, int, unsigned int, int, const char*);

REGISTER_FUNCTION_P1(ScheduleCancelObject, TinScript::CScheduler::CancelObject, void, unsigned int);
REGISTER_FUNCTION_P1(ScheduleCancelRequest, TinScript::CScheduler::CancelRequest, void, int);
REGISTER_FUNCTION_P0(ListSchedules, TinScript::CScheduler::Dump, void);

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
