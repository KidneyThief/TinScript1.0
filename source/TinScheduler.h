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
// TinScheduler.h
// ------------------------------------------------------------------------------------------------

#ifndef __TINSCHEDULER_H
#define __TINSCHEDULER_H

namespace TinScript {

class CFunctionContext;

class CScheduler {
    public:

        CScheduler(CScriptContext* script_context = NULL);
        virtual ~CScheduler();

        CScriptContext* GetScriptContext() {
            return (mContextOwner);
        }

        void Update(uint32 curtime);

        class CCommand {
            public:
                CCommand(CScriptContext* script_context, int _reqid, uint32 _objectid = 0,
                         uint32 _dispatchtime = 0, const char* _command = NULL);
                CCommand(CScriptContext* script_context, int _reqid, uint32 _objectid,
                         uint32 _dispatchtime, uint32 _funchash);

                virtual ~CCommand();

                CCommand* mPrev;
                CCommand* mNext;

                CScriptContext* mContextOwner;

                int mReqID;
                uint32 mObjectID;
                uint32 mDispatchTime;
                char mCommandBuf[kMaxTokenLength];

                uint32 mFuncHash;
                CFunctionContext* mFuncContext;
        };

        int Schedule(uint32 objectid, int delay, const char* commandstring);
        int Thread(int reqid, uint32 objectid, int delay, const char* commandstring);
        void CancelObject(uint32 objectid);
        void CancelRequest(int reqid);
        void Cancel(uint32 objectid, int reqid);
        void Dump();

        CCommand* ScheduleCreate(uint32 objectid, int delay, uint32 funchash);
        CScheduler::CCommand* mCurrentSchedule;

    private:
        CScriptContext* mContextOwner;

        CCommand* mHead;
        uint32 mCurrentSimTime;
};

} // TinScript

#endif

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
