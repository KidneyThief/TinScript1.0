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
        static void Initialize();
        static void Shutdown();
        static void Update(unsigned int curtime);

        class CCommand {
            public:
                CCommand(int _reqid, unsigned int _objectid = 0, unsigned int _dispatchtime = 0,
                         const char* _command = NULL);
                CCommand(int _reqid, unsigned int _objectid, unsigned int _dispatchtime,
                         unsigned int _funchash);

                virtual ~CCommand();

                CCommand* prev;
                CCommand* next;

                int reqid;
                unsigned int objectid;
                unsigned int dispatchtime;
                char commandbuf[kMaxTokenLength];

                unsigned int funchash;
                CFunctionContext* funccontext;
        };

        static int Schedule(unsigned int objectid, int delay, const char* commandstring);
        static int Thread(int reqid, unsigned int objectid, int delay, const char* commandstring);
        static void CancelObject(unsigned int objectid);
        static void CancelRequest(int reqid);
        static void Cancel(unsigned int objectid, int reqid);
        static void Dump();

        static CCommand* ScheduleCreate(unsigned int objectid, int delay, unsigned int funchash);

    private:
        static CCommand* head;
};

} // TinScript

#endif

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
