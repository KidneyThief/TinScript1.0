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

#ifndef __TINEXECUTE_H
#define __TINEXECUTE_H

#include "integration.h"
#include "TinCompile.h"

namespace TinScript {

class CExecStack {
	public:
		CExecStack(CScriptContext* script_context, uint32 _size = 0) {
            mContextOwner = script_context;

			mStack = NULL;
			mSize = _size;
			assert(mSize > 0);

			mStack = TinAllocInstrBlock(mSize);
			mStackTop = mStack;
		}

		virtual ~CExecStack() {
			if(mStack)
                TinFreeArray(mStack);
		}

        CScriptContext* GetContextOwner() {
            return (mContextOwner);
        }

		void Push(void* content, eVarType contenttype)
		{
			assert(content != NULL);
			uint32 contentsize = kBytesToWordCount(gRegisteredTypeSize[contenttype]);
			uint32* contentptr = (uint32*)content;
			for(uint32 i = 0; i < contentsize; ++i)
				*mStackTop++ = *contentptr++;

			// -- push the type of the content as well, so we know what to pull
			*mStackTop++ = (uint32)contenttype;
		}

		void* Pop(eVarType& contenttype) {
			uint32 stacksize = ((uint32)mStackTop - (uint32)mStack) / 4;
			assert(stacksize > 0);
			contenttype = (eVarType)(*(--mStackTop));
			assert(contenttype >= 0 && contenttype < TYPE_COUNT);
			uint32 contentsize = kBytesToWordCount(gRegisteredTypeSize[contenttype]);

			// -- ensure we have enough data on the stack, both the content, and the type
			assert(stacksize >= contentsize + 1);
			mStackTop -= contentsize;
			return (void*)mStackTop;
		}

        void Reserve(int wordcount) {
            mStackTop += wordcount;
        }

        void UnReserve(int wordcount) {
            mStackTop -= wordcount;
        }

        int GetStackTop() {
            return ((uint32)mStackTop - (uint32)mStack) / sizeof(uint32);
        }

        void* GetStackVarAddr(int varstacktop, int varoffset) {
            uint32* varaddr = &mStack[varstacktop];

            // -- increment by (varoffset * MAX_TYPE_SIZE), so we're pointing
            // -- at the start of the memory block for the variable.
            varaddr += varoffset * MAX_TYPE_SIZE;

            // -- validate and return the addr
            if(varaddr < mStack || varaddr >= mStackTop) {
                ScriptAssert_(GetContextOwner(), 0, "<internal>", -1,
                              "Error - GetStackVarAddr() out of range\n");
                return NULL;
            }
            return varaddr;
        }

	private:
        CScriptContext* mContextOwner;

		uint32* mStack;
		uint32 mSize;

		uint32* mStackTop;
};

class CFunctionCallStack {
	public:
        struct tFunctionCallEntry;
		CFunctionCallStack(uint32 _size = 0) {
   			size = _size;
			assert(size > 0);
			funcentrystack = TinAllocArray(ALLOC_FuncCallEntry, tFunctionCallEntry, size);
			stacktop = 0;
		}

		virtual ~CFunctionCallStack() {
			if(funcentrystack)
				delete[] funcentrystack;
		}

		void Push(CFunctionEntry* functionentry, CObjectEntry* objentry, int varoffset)
		{
			assert(functionentry != NULL);
            assert(stacktop < size);
            funcentrystack[stacktop].objentry = objentry;
            funcentrystack[stacktop].funcentry = functionentry;
            funcentrystack[stacktop].stackvaroffset = varoffset;
            funcentrystack[stacktop].isexecuting = false;
            ++stacktop;
		}

		CFunctionEntry* Pop(CObjectEntry*& objentry) {
			assert(stacktop > 0);
            objentry = funcentrystack[stacktop - 1].objentry;
            return funcentrystack[--stacktop].funcentry;
		}

   		CFunctionEntry* GetTop(CObjectEntry*& objentry, int& varoffset) {
            if(stacktop > 0) {
                objentry = funcentrystack[stacktop - 1].objentry;
                varoffset = funcentrystack[stacktop - 1].stackvaroffset;
                return funcentrystack[stacktop - 1].funcentry;
            }
            else {
                objentry = NULL;
                varoffset = -1;
                return NULL;
            }
        }

        int32 DebuggerGetCallstack(uint32* codeblock_array, uint32* objid_array,
                                   uint32* namespace_array, uint32* func_array,
                                   uint32* linenumber_array, int max_array_size);

        void BeginExecution(const uint32* instrptr);
        void BeginExecution();

        CFunctionEntry* GetExecuting(CObjectEntry*& objentry, int& varoffset) {
            int temp = stacktop - 1;
            while(temp >= 0) {
                if(funcentrystack[temp].isexecuting) {
                    objentry = funcentrystack[temp].objentry;
                    varoffset = funcentrystack[temp].stackvaroffset;
                    return funcentrystack[temp].funcentry;
                }
                --temp;
            }
            return NULL;
        }

   		CFunctionEntry* GetTopMethod(CObjectEntry*& objentry) {
            int depth = 0;
            while(stacktop - depth > 0) {
                ++depth;
                if(funcentrystack[stacktop - depth].objentry) {
                    objentry = funcentrystack[stacktop - depth].objentry;
                    return funcentrystack[stacktop - depth].funcentry;
                }
            }

            // -- no methods
            objentry = NULL;
            return NULL;
        }

        struct tFunctionCallEntry {
            tFunctionCallEntry(CFunctionEntry* _funcentry = NULL, CObjectEntry* _objentry = NULL,
                               int _varoffset = -1) {
                funcentry = _funcentry;
                objentry = _objentry;
                stackvaroffset = _varoffset;
                linenumberfunccall = 0;
                isexecuting = false;
            }

            CFunctionEntry* funcentry;
            CObjectEntry* objentry;
            int stackvaroffset;
            uint32 linenumberfunccall;
            bool isexecuting;
        };

	private:
        tFunctionCallEntry* funcentrystack;
		uint32 size;
		uint32 stacktop;
};

bool ExecuteCodeBlock(CCodeBlock& codeblock);
bool ExecuteScheduledFunction(CScriptContext* script_context, uint32 objectid, uint32 funchash,
                              CFunctionContext* parameters);

}  // TinScript

#endif // __TINEXECUTE_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
