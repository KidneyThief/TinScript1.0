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

// ====================================================================================================================
// TinScheduler.cpp
// ====================================================================================================================

// -- includes
#include "stdafx.h"
#include "stdio.h"

#include "TinScript.h"
#include "TinRegistration.h"
#include "TinInterface.h"
#include "TinExecute.h"
#include "TinScheduler.h"

// == namespace TinScript =============================================================================================

namespace TinScript
{

// == class CScheduler ================================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CScheduler::CScheduler(CScriptContext* script_context)
{
    mContextOwner = script_context;
    mHead = NULL;
    mCurrentSimTime = 0;
    mCurrentSchedule = NULL;
}

// ====================================================================================================================
// Destructor
// ====================================================================================================================
CScheduler::~CScheduler()
{
    // -- clean up all pending scheduled events
    while(mHead) {
        CCommand* next = mHead->mNext;
        TinFree(mHead);
        mHead = next;
    }
}

// ====================================================================================================================
// Update():  Iterates through the list of requests, executing those who's requested time has elapsed.
// ====================================================================================================================
void CScheduler::Update(uint32 curtime)
{
    // -- cache the current time
    mCurrentSimTime = curtime;

    // -- execute all commands scheduled for dispatch by this time
    while (mHead && mHead->mDispatchTime <= curtime)
    {
        // -- get the current command, and remove it from the list - now, before we execute,
        // -- since executing this command could 
        CCommand* curcommand = mHead;
        if(curcommand->mNext)
            curcommand->mNext->mPrev = NULL;
        mHead = curcommand->mNext;

        // -- dispatch the command - see if it's a direct function call, or a command buf
        if (curcommand->mFuncHash != 0)
        {
            ExecuteScheduledFunction(GetScriptContext(), curcommand->mObjectID, curcommand->mFuncHash,
                                     curcommand->mFuncContext);
        }
        else
        {
            if(curcommand->mObjectID > 0)
            {
                int32 dummy = 0;
                ObjExecF(curcommand->mObjectID, dummy, curcommand->mCommandBuf);
            }
            else
            {
                // $$$TZA is there anything we can do with the result?
                GetScriptContext()->ExecCommand(curcommand->mCommandBuf);
            }
        }

        // -- delete the command
        TinFree(curcommand);
    }
}

// ====================================================================================================================
// CancelObject():  On destruction of an object, cancel all scheduled method calls.
// ====================================================================================================================
void CScheduler::CancelObject(uint32 objectid)
{
    if(objectid == 0)
        return;
    Cancel(objectid, 0);
}

// ====================================================================================================================
// CancelRequest():  Cancel a scheduled function/method call by ID
// ====================================================================================================================
void CScheduler::CancelRequest(int32 reqid)
{
    if(reqid <= 0)
        return;
    Cancel(0, reqid);
}

// ====================================================================================================================
// Cancel():  Cancel a scheduled method call by ID, but for a specific object
// ====================================================================================================================
void CScheduler::Cancel(uint32 objectid, int32 reqid)
{
    // -- loop through and delete any schedules pending for this object
    CCommand** prevcommand = &mHead;
    CCommand* curcommand = mHead;
    while(curcommand) {
        if(curcommand->mObjectID == objectid || curcommand->mReqID == reqid) {
            *prevcommand = curcommand->mNext;
            TinFree(curcommand);
            curcommand = *prevcommand;
        }
        else {
            prevcommand = &curcommand->mNext;
            curcommand = curcommand->mNext;
        }
    }
}

// ====================================================================================================================
// Dump():  Display the list of scheduled requests through standard text.
// ====================================================================================================================
void CScheduler::Dump()
{
    // -- loop through and delete any schedules pending for this object
    CCommand* curcommand = mHead;
    while(curcommand) {
        TinPrint(GetScriptContext(), "ReqID: %d, ObjID: %d, Command: %s\n", curcommand->mReqID,
                 curcommand->mObjectID, curcommand->mCommandBuf);
        curcommand = curcommand->mNext;
    }
}

// == class CScheduler::CCommand ======================================================================================

// ====================================================================================================================
// Constructor: Schedule a raw text statement, to be parsed and executed.
// ====================================================================================================================
CScheduler::CCommand::CCommand(CScriptContext* script_context, int32 _reqid, uint32 _objectid,
                               uint32 _dispatchtime, const char* _command, bool8 immediate)
{
    // -- set the context
    mContextOwner = script_context;

    // --  members copy the command members
    mReqID = _reqid;
    mObjectID = _objectid;
    mDispatchTime = _dispatchtime;
    mImmediateExec = immediate;
    SafeStrcpy(mCommandBuf, _command, kMaxTokenLength);

    // -- command string, null out the direct function call members
    mFuncHash = 0;
    mFuncContext = NULL;
}

// ====================================================================================================================
// Constructor:  Schedule a specific function/method call - much more efficient than raw text.
// ====================================================================================================================
CScheduler::CCommand::CCommand(CScriptContext* script_context, int32 _reqid, uint32 _objectid,
                               uint32 _dispatchtime, uint32 _funchash, bool8 immediate)
{
    // -- set the context
    mContextOwner = script_context;

    // --  members copy the command members
    mReqID = _reqid;
    mObjectID = _objectid;
    mDispatchTime = _dispatchtime;
    mImmediateExec = immediate;
    mCommandBuf[0] = '\0';

    // -- command string, null out the direct function call members
    mFuncHash = _funchash;
    mFuncContext = TinAlloc(ALLOC_FuncContext, CFunctionContext, script_context);
}

// ====================================================================================================================
// Destructor
// ====================================================================================================================
CScheduler::CCommand::~CCommand()
{
    // clean up the function context, if it exists
    if(mFuncContext)
        TinFree(mFuncContext);
}

// ====================================================================================================================
// Schedule():  Schedule a raw text command.
// ====================================================================================================================
static int32 gScheduleID = 0;
int32 CScheduler::Schedule(uint32 objectid, int32 delay, const char* commandstring)
{
    ++gScheduleID;

    // -- ensure we have a valid command string
    if(!commandstring || !commandstring[0])
        return 0;

    // -- calculate the dispatch time - enforce a one-frame delay
    uint32 dispatchtime = mCurrentSimTime + (delay > 0 ? delay : 1);

    // -- create the new commmand
    CCommand* newcommand = TinAlloc(ALLOC_SchedCmd, CCommand, GetScriptContext(), gScheduleID,
                                    objectid, dispatchtime, commandstring);

    // -- see if it goes at the front of the list
    if (!mHead || dispatchtime <= mHead->mDispatchTime)
    {
        newcommand->mNext = mHead;
        newcommand->mPrev = NULL;
        if(mHead)
            mHead->mPrev = newcommand;
        mHead = newcommand;
    }
    else
    {
        // -- insert it into the list, in after curschedule
        CCommand* curschedule = mHead;
        while(curschedule->mNext && curschedule->mDispatchTime < dispatchtime)
            curschedule = curschedule->mNext;
        newcommand->mNext = curschedule->mNext;
        newcommand->mPrev = curschedule;
        if(curschedule->mNext)
            curschedule->mNext->mPrev = newcommand;
        curschedule->mNext = newcommand;
    }

    // -- return the request id, so we have a way to cancel
    return newcommand->mReqID;
}

// ====================================================================================================================
// ScheduleCreate():  Create a schedule request.
// ====================================================================================================================
CScheduler::CCommand* CScheduler::ScheduleCreate(uint32 objectid, int32 delay,
                                                 uint32 funchash, bool8 immediate)
{
    ++gScheduleID;

    // -- calculate the dispatch time - enforce a one-frame delay
    uint32 dispatchtime = mCurrentSimTime + (delay > 0 ? delay : 1);

    // -- create the new commmand
    CCommand* newcommand = TinAlloc(ALLOC_SchedCmd, CCommand, GetScriptContext(), gScheduleID,
                                    objectid, dispatchtime, funchash, immediate);

    // -- add space to store a return value
    newcommand->mFuncContext->AddParameter("_return", Hash("_return"), TYPE__resolve, 1, 0);

    // -- see if it goes at the front of the list
    if (!mHead || dispatchtime <= mHead->mDispatchTime)
    {
        newcommand->mNext = mHead;
        newcommand->mPrev = NULL;
        if(mHead)
            mHead->mPrev = newcommand;
        mHead = newcommand;
    }
    else
    {
        // -- insert it into the list, in after curschedule
        CCommand* curschedule = mHead;
        while(curschedule->mNext && curschedule->mDispatchTime < dispatchtime)
            curschedule = curschedule->mNext;
        newcommand->mNext = curschedule->mNext;
        newcommand->mPrev = curschedule;
        if(curschedule->mNext)
            curschedule->mNext->mPrev = newcommand;
        curschedule->mNext = newcommand;
    }

    // -- return the actual commmand object, since we'll be updating the parameter values
    return newcommand;
}

// ====================================================================================================================
// Thread():  Cancel a previous request, and replace with this one.
// ====================================================================================================================
// -- When iterating by reloading/re-executing scripts, a scheduled request can accidently queue up multiples, when
// -- only one request was ever meant to exist.  This interface can ensure that only one actual schedule exists.
int32 CScheduler::Thread(int32 reqid, uint32 objectid, int32 delay, const char* commandstring)
{
    CancelRequest(reqid);
    int32 newreqid = Schedule(objectid, delay, commandstring);
    return newreqid;
}

} // TinScript

// ------------------------------------------------------------------------------------------------
// script registered interface

// $$$TZA schedule is a keyword within the language...  these external functions either need
// -- to be converted to methods from an (CScriptContext*) object
/*
REGISTER_FUNCTION_P3(Schedule, TinScript::CScheduler::Schedule, int32, uint32, int32, const char*);
REGISTER_FUNCTION_P4(ScheduleThread, TinScript::CScheduler::Thread, int32, int32, uint32, int32, const char*);

REGISTER_FUNCTION_P1(ScheduleCancelObject, TinScript::CScheduler::CancelObject, void, uint32);
REGISTER_FUNCTION_P1(ScheduleCancelRequest, TinScript::CScheduler::CancelRequest, void, int32);
REGISTER_FUNCTION_P0(ListSchedules, TinScript::CScheduler::Dump, void);
*/

// ====================================================================================================================
// EOF
// ====================================================================================================================
