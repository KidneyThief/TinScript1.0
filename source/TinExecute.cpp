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
// TinExecute.cpp : Implementation of the virtual machine
// ------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "assert.h"
#include "string.h"
#include "stdio.h"

#include "socket.h"

#include "TinScript.h"
#include "TinCompile.h"
#include "TinNamespace.h"
#include "TinScheduler.h"
#include "TinExecute.h"
#include "TinOpExecFunctions.h"

namespace TinScript {

OpExecuteFunction gOpExecFunctions[OP_COUNT] = {
    #define OperationEntry(a) OpExec##a,
    OperationTuple
    #undef OperationEntry
};

bool8 CopyStackParameters(CFunctionEntry* fe, CExecStack& execstack,
                          CFunctionCallStack& funccallstack) {

    // -- sanity check
    if(fe == NULL || !fe->GetContext()) {
        ScriptAssert_(fe->GetScriptContext(), 0, "<internal>", -1,
                      "Error - invalid function entry\n");
        return false;
    }

    // -- initialize the parameters of our fe with the function context
    CFunctionContext* parameters = fe->GetContext();
    int32 srcparamcount = parameters->GetParameterCount();
    for(int32 i = 0; i < srcparamcount; ++i) {
        CVariableEntry* src = parameters->GetParameter(i);
        void* dst = GetStackVarAddr(fe->GetScriptContext(), execstack, funccallstack,
                                    src->GetStackOffset());
        if(!dst) {
            ScriptAssert_(fe->GetScriptContext(), 0, "<internal>", -1,
                          "Error - unable to assign parameter %d, calling function %s()\n",
                          i, UnHash(fe->GetHash()));
            return false;
        }

        // -- set the value - note, parameters in a function context are never
        // -- CVariableEntry's with object offsets
        if(src) {
            memcpy(dst, src->GetAddr(NULL), MAX_TYPE_SIZE * sizeof(uint32));
        }
        else
            memset(dst, 0, MAX_TYPE_SIZE * sizeof(uint32));
    }

    return true;
}

int32 CFunctionCallStack::DebuggerGetCallstack(uint32* codeblock_array, uint32* objid_array,
                                               uint32* namespace_array, uint32* func_array,
                                               uint32* linenumber_array, int32 max_array_size) {

    int32 entry_count = 0;
    int32 temp = stacktop - 1;
    while(temp >= 0 && entry_count < max_array_size) {
        if(funcentrystack[temp].isexecuting) {
            CCodeBlock* codeblock = NULL;
            funcentrystack[temp].funcentry->GetCodeBlockOffset(codeblock);
            uint32 codeblock_hash = codeblock->GetFilenameHash();
            uint32 objid = funcentrystack[temp].objentry ? funcentrystack[temp].objentry->GetID() : 0;
            uint32 namespace_hash = funcentrystack[temp].funcentry->GetNamespaceHash();
            uint32 func_hash = funcentrystack[temp].funcentry->GetHash();
            uint32 linenumber = funcentrystack[temp].linenumberfunccall;

            codeblock_array[entry_count] = codeblock_hash;
            objid_array[entry_count] = objid;
            namespace_array[entry_count] = namespace_hash;
            func_array[entry_count] = func_hash;
            linenumber_array[entry_count] = linenumber;
            ++entry_count;
        }
        --temp;
    }
    return (entry_count);
}

int32 CFunctionCallStack::DebuggerGetStackVarEntries(CScriptContext* script_context, CExecStack& execstack,
                                                     CDebuggerWatchVarEntry* entry_array, int32 max_array_size)
{
    int32 entry_count = 0;
    int32 stack_index = stacktop - 1;
    while (stack_index >= 0 && entry_count < max_array_size)
    {
        if (funcentrystack[stack_index].isexecuting)
        {
            // -- if this function call is a method, send the "self" variable
            if (funcentrystack[stack_index].objentry != NULL)
            {
                // -- limit of kDebuggerWatchWindowSize
                if (entry_count >= max_array_size)
                    return (entry_count);

                CDebuggerWatchVarEntry* cur_entry = &entry_array[entry_count++];

                // -- copy the calling function info
                cur_entry->mFuncNamespaceHash = funcentrystack[stack_index].funcentry->GetNamespaceHash();
                cur_entry->mFunctionHash = funcentrystack[stack_index].funcentry->GetHash();
                cur_entry->mFunctionObjectID = funcentrystack[stack_index].objentry->GetID();

                // -- this isn't a member of an object
                cur_entry->mObjectID = 0;
                cur_entry->mNamespaceHash = 0;

                // -- copy the var type, name and value
                cur_entry->mType = TYPE_object;
                strcpy_s(cur_entry->mVarName, "self");
                sprintf_s(cur_entry->mValue, "%d", cur_entry->mFunctionObjectID);

                // -- fill in the cached members
                cur_entry->mVarHash = Hash("self");
                cur_entry->mVarObjectID = cur_entry->mFunctionObjectID;
            }

            // -- get the variable table
            tVarTable* func_vt = funcentrystack[stack_index].funcentry->GetLocalVarTable();
            CVariableEntry* ve = func_vt->First();
            while (ve)
            {
                // -- limit of kDebuggerWatchWindowSize
                if (entry_count >= max_array_size)
                    return (entry_count);

                // -- fill in the current entry
                CDebuggerWatchVarEntry* cur_entry = &entry_array[entry_count++];
                if (entry_count >= max_array_size)
                    return max_array_size;

                // -- copy the calling function info
                cur_entry->mFuncNamespaceHash = funcentrystack[stack_index].funcentry->GetNamespaceHash();
                cur_entry->mFunctionHash = funcentrystack[stack_index].funcentry->GetHash();
                cur_entry->mFunctionObjectID = funcentrystack[stack_index].objentry
                                               ? funcentrystack[stack_index].objentry->GetID()
                                               : 0;

                // -- this isn't a member of an object
                cur_entry->mObjectID = 0;
                cur_entry->mNamespaceHash = 0;

                // -- copy the var type
                cur_entry->mType = ve->GetType();

                // -- copy the var name
                SafeStrcpy(cur_entry->mVarName, UnHash(ve->GetHash()), kMaxNameLength);

                // -- get the address on the stack, where this local var is stored
                int32 func_stacktop = funcentrystack[stack_index].stackvaroffset;
                int32 var_stackoffset = ve->GetStackOffset();
                void* stack_var_addr = execstack.GetStackVarAddr(func_stacktop, var_stackoffset);

                // -- copy the value, as a string (to a max length)
              	gRegisteredTypeToString[ve->GetType()](stack_var_addr, cur_entry->mValue, kMaxNameLength);

                // -- fill in the hash of the var name, and if applicable, the var object ID
                cur_entry->mVarHash = ve->GetHash();
                cur_entry->mVarObjectID = 0;
                if (ve->GetType() == TYPE_object)
                {
                    cur_entry->mVarObjectID = stack_var_addr ? *(uint32*)stack_var_addr : 0;

                    // -- ensure the object actually exists
                    if (script_context->FindObjectEntry(cur_entry->mVarObjectID) == NULL)
                    {
                        cur_entry->mVarObjectID = 0;
                    }
                }

                // -- get the next
                ve = func_vt->Next();
            }
        }
        --stack_index;
    }

    return (entry_count);
}

void CFunctionCallStack::BeginExecution(const uint32* instrptr) {
    // -- the top entry on the function stack is what we're about to call...
    // -- the stacktop - 2, therefore, is the calling function (if it exists)...
    // -- tag it with the offset into the codeblock, for a debugger callstack
    if (stacktop >= 2 && funcentrystack[stacktop - 2].funcentry->GetType() == eFuncTypeScript) {
        CCodeBlock* callingfunc_cb = NULL;
        funcentrystack[stacktop - 2].funcentry->GetCodeBlockOffset(callingfunc_cb);
        funcentrystack[stacktop - 2].linenumberfunccall = callingfunc_cb->CalcLineNumber(instrptr);
    }

    BeginExecution();
}

void CFunctionCallStack::BeginExecution() {
	assert(stacktop > 0);
    assert(funcentrystack[stacktop - 1].isexecuting == false);
    funcentrystack[stacktop - 1].isexecuting = true;
}

bool8 CodeBlockCallFunction(CFunctionEntry* fe, CObjectEntry* oe, CExecStack& execstack,
                            CFunctionCallStack& funccallstack) {

    // -- at this point, the funccallstack has the CFunctionEntry pushed
    // -- and all parameters have been copied - either to the function's local var table
    // -- for registered 'C' functions, or to the execstack for scripted functions

    // -- scripted function
    if(fe->GetType() == eFuncTypeScript) {
        // -- for scripted functions, we need to copy the localvartable onto the stack,
        // -- to ensure threaded or recursive function calls don't stomp each other
        CopyStackParameters(fe, execstack, funccallstack);

        CCodeBlock* funccb = NULL;
        uint32 funcoffset = fe->GetCodeBlockOffset(funccb);
        if(!funccb) {
            ScriptAssert_(fe->GetScriptContext(), 0, "<internal>", -1,
                          "Error - Undefined function: %s()\n", UnHash(fe->GetHash()));
            return false;
        }

        // -- execute the function via codeblock/offset
        bool8 success = funccb->Execute(funcoffset, execstack, funccallstack);

        if(!success) {
            ScriptAssert_(fe->GetScriptContext(), 0, "<internal>", -1,
                          "Error - error executing function: %s()\n",
                          UnHash(fe->GetHash()));
            return false;
        }
    }

    // -- registered 'C' function
    else if(fe->GetType() == eFuncTypeGlobal) {
        fe->GetRegObject()->DispatchFunction(oe ? oe->GetAddr() : NULL);

        // -- if the function has a return type, push it on the stack
        if(fe->GetReturnType() > TYPE_void) {
            assert(fe->GetContext() && fe->GetContext()->GetParameterCount() > 0);
            CVariableEntry* returnval = fe->GetContext()->GetParameter(0);
            assert(returnval);
            execstack.Push(returnval->GetAddr(NULL), returnval->GetType());
        }

        // -- all functions must push a return value
        else {
            int32 empty = 0;
            execstack.Push(&empty, TYPE_int);
        }

        // -- clear all parameters for the function - this will ensure all
        // -- strings are decremented, keeping the string table clear of unassigned values
        fe->GetContext()->ClearParameters();
        fe->GetScriptContext()->GetStringTable()->RemoveUnreferencedStrings();
        
        // -- since we called a 'C' function, there's no OP_FuncReturn - pop the function call stack
        funccallstack.Pop(oe);
    }

    return true;
}

bool8 ExecuteCodeBlock(CCodeBlock& codeblock) {

	// -- create the stack to use for the execution
	CExecStack execstack(codeblock.GetScriptContext(), kExecStackSize);
    CFunctionCallStack funccallstack(kExecFuncCallDepth);

    return codeblock.Execute(0, execstack, funccallstack);
}

bool8 ExecuteScheduledFunction(CScriptContext* script_context, uint32 objectid, uint32 funchash,
                               CFunctionContext* parameters) {

    // -- sanity check
    if(funchash == 0 && parameters == NULL) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                      "Error - invalid funchash/parameters\n");
        return false;
    }

    // -- see if this is a method or a function
    CObjectEntry* oe = NULL;
    CFunctionEntry* fe = NULL;
    if(objectid != 0) {
        // -- find the object
        oe = script_context->FindObjectEntry(objectid);
        if(!oe) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - unable to find object: %d\n", objectid);
            return false;
        }

        // -- get the namespace, then the function
        fe = oe->GetFunctionEntry(0, funchash);
    }
    else {
        fe = script_context->GetGlobalNamespace()->GetFuncTable()->FindItem(funchash);
    }

    // -- ensure we found our function
    if(!fe) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                      "Error - unable to find function: %s\n", UnHash(funchash));
        return false;
    }

	// -- create the stack to use for the execution
	CExecStack execstack(script_context, kExecStackSize);
    CFunctionCallStack funccallstack(kExecFuncCallDepth);

    // -- nullvalue used to clear parameter values
    char nullvalue[MAX_TYPE_SIZE];
    memset(nullvalue, 0, MAX_TYPE_SIZE);

    // -- initialize the parameters of our fe with the function context
    int32 srcparamcount = parameters->GetParameterCount();
    for(int32 i = 0; i < srcparamcount; ++i) {
        CVariableEntry* src = parameters->GetParameter(i);
        CVariableEntry* dst = fe->GetContext()->GetParameter(i);
        if(!dst) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - unable to assign parameter %d, calling function %s()\n",
                          i, UnHash(funchash));
            return false;
        }

        // -- ensure the type of the parameter value is converted to the type required
        void* srcaddr = NULL;
        if(src && dst->GetType() >= FIRST_VALID_TYPE)
            srcaddr = TypeConvert(script_context, src->GetType(), src->GetAddr(NULL), dst->GetType());
        else
            srcaddr = nullvalue;

        // -- set the value - note stack parameters are always local variables, never members
        dst->SetValue(NULL, srcaddr);
    }

    // -- initialize any remaining parameters
    int32 dstparamcount = fe->GetContext()->GetParameterCount();
    for(int32 i = srcparamcount; i < dstparamcount; ++i) {
        CVariableEntry* dst = fe->GetContext()->GetParameter(i);
        dst->SetValue(NULL, nullvalue);
    }

    // -- push the function entry onto the call stack (same as if OP_FuncCallArgs had been used)
    funccallstack.Push(fe, oe, 0);
    
    // -- create space on the execstack, if this is a script function
    if(fe->GetType() != eFuncTypeGlobal) {
        int32 localvarcount = fe->GetLocalVarTable()->Used();
        execstack.Reserve(localvarcount * MAX_TYPE_SIZE);
    }

    // -- scheduled functions are never nested, so it's ok to tag this function as having started
    // -- execution
    funccallstack.BeginExecution();

    // -- call the function
    bool8 result = CodeBlockCallFunction(fe, oe, execstack, funccallstack);
    if(!result) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                      "Error - Unable to call function: %s()\n",
                      UnHash(fe->GetHash()));
        return false;
    }

    // -- because every function is required to push a value onto the stack, pop the stack and
    // -- copy it to the _return parameter of this scheduled function
    eVarType contenttype;
    void* contentptr = execstack.Pop(contenttype);
    if(!contentptr) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                                         "Error - no return value for scheduled func: %s()\n",
                                          UnHash(fe->GetHash()));
        return (false);
    }

    // -- get the return variable
    CVariableEntry* return_ve = parameters->GetParameter(0);
    if(!return_ve || return_ve->GetType() != TYPE__resolve) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                     "Error - invalid return parameter for scheduled func: %s()\n",
                     UnHash(fe->GetHash()));
        return (false);
    }

    // -- copy the stack contents into the return address of the scheduled function call
    return_ve->ResolveValueType(contenttype, contentptr);

    return (true);
}

OpExecuteFunction GetOpExecFunction(eOpCode curoperation) {
    return (gOpExecFunctions[curoperation]);
}

// ====================================================================================================================
// DebuggerBreakLoop():  Handles a failed assert condition, and either breaks in the remote debugger, or asserts
// ====================================================================================================================
bool8 DebuggerAssertLoop(const char* condition, CCodeBlock* cb, const uint32* instrptr, CExecStack& execstack,
                         CFunctionCallStack& funccallstack, const char* fmt, ...)
{
    // -- pull the file and line number, and construct the full condition message
    const char* filename = cb->GetFileName();
    int32 line_number = cb->CalcLineNumber(instrptr);
    char cond_buf[512];
    sprintf_s(cond_buf, "Assert(%s) file: %s, line %d:", condition, filename, line_number + 1);

    // -- compose the assert message
    va_list args;
    va_start(args, fmt);
    char msg_buf[512];
    vsprintf_s(msg_buf, 512, fmt, args);
    va_end(args);

    // -- put both messages together, and send that to the DebuggerBreakLoop
    char assert_msg[2048];
    sprintf_s(assert_msg, "%s\n%s", cond_buf, msg_buf);

    return (DebuggerBreakLoop(cb, instrptr, execstack, funccallstack, assert_msg));
}

// ====================================================================================================================
// DebuggerBreakLoop():  If a remote debugger is connected, we'll halt the VM until released by the debugger
// ====================================================================================================================
bool8 DebuggerBreakLoop(CCodeBlock* cb, const uint32* instrptr, CExecStack& execstack,
                        CFunctionCallStack& funccallstack, const char* assert_msg)
{
    // -- asserts, and breakpoints both need to do the same thing - notify the debugger of the file/line,
    // -- loop until the user makes choice.  Asserts have an additional message

    // -- if we're not able to handle the break loop, return false (possibly so an assert can trigger instead)
    if (!cb || !cb->GetScriptContext() || !cb->GetScriptContext()->IsDebuggerConnected())
        return (false);

    CScriptContext* script_context = cb->GetScriptContext();
    int32 cur_line = cb->CalcLineNumber(instrptr);
    uint32 codeblock_hash = cb->GetFilenameHash();

    // -- if we're already broken in an assert of some sort, we need to protect this loop from being re-entrant
    if (script_context->mDebuggerBreakLoopGuard || script_context->IsAssertStackSkipped())
    {
        // -- print the assert message, if we have one, but simply return (true)
        if (assert_msg && assert_msg[0])
        {
            TinPrint(script_context, assert_msg);
            return (true);
        }
    }

    // -- set the guard
    script_context->mDebuggerBreakLoopGuard = true;

    // -- set the current line we're broken on
    funccallstack.mDebuggerLastBreak = cur_line;

    // -- build the callstack arrays, in preparation to send them to the debugger
    uint32 codeblock_array[kDebuggerCallstackSize];
    uint32 objid_array[kDebuggerCallstackSize];
    uint32 namespace_array[kDebuggerCallstackSize];
    uint32 func_array[kDebuggerCallstackSize];
    uint32 linenumber_array[kDebuggerCallstackSize];
    int32 stack_size =
        funccallstack.DebuggerGetCallstack(codeblock_array, objid_array,
                                            namespace_array, func_array,
                                            linenumber_array, kDebuggerCallstackSize);

    // -- note - the line number for the function we're currently in, won't have been set
    // -- in the function call stack
    linenumber_array[0] = cur_line;

    script_context->DebuggerSendCallstack(codeblock_array, objid_array,
                                            namespace_array, func_array,
                                            linenumber_array, stack_size);

    // -- get the entire list of variables, at every level for the current call stack
    CDebuggerWatchVarEntry watch_var_stack[kDebuggerWatchWindowSize];
    int32 watch_entry_size =
        funccallstack.DebuggerGetStackVarEntries(script_context, execstack,
                                                    watch_var_stack, kDebuggerWatchWindowSize);

    // -- now loop through all stack variables, and any that are objects, send their
    // -- member dictionaries as well
    for (int32 i = 0; i < watch_entry_size; ++i)
    {
        script_context->DebuggerSendWatchVariable(&watch_var_stack[i]);

        // -- if the watch var is of type object, send the object members over as well
        if (watch_var_stack[i].mType == TYPE_object)
        {
            script_context->DebuggerSendObjectMembers(&watch_var_stack[i],
                                                        watch_var_stack[i].mVarObjectID);
        }
    }

    // -- send a message to the debugger - either this is an assert, or a breakpoint
    bool is_assert = (assert_msg && assert_msg[0]);
    if (is_assert)
    {
        script_context->DebuggerSendAssert(assert_msg, codeblock_hash, cur_line);
    }
    else
    {
        script_context->DebuggerBreakpointHit(codeblock_hash, cur_line);
    }

    // -- wait for the debugger to either continue to step or run
    script_context->SetBreakActionStep(false);
    script_context->SetBreakActionRun(false);
    while (true)
    {
        // -- disable breaking on any asserts while we're waiting for the original loop to exit
        if (is_assert)
        {
            script_context->SetAssertStackSkipped(true);
            script_context->SetAssertEnableTrace(true);
        }

        // -- we spin forever in this loop, until either the debugger disconnects,
        // -- or sends a message to step or run
        script_context->ProcessThreadCommands();

        // -- if either mDebuggerBreakStep or mDebuggerBreakRun was set, exit the loop
        if (script_context->mDebuggerActionStep || script_context->mDebuggerActionRun)
        {
            // -- set the bool to continue to break, based on which action is true
            // -- (unless it's an assert)
            funccallstack.mDebuggerBreakStep = !is_assert && script_context->mDebuggerActionStep;
            break;
        }

        // -- otherwise, sleep
        Sleep(1);
    }

    // -- disable further asserts until the stack is unwound.
    if (is_assert)
    {
        script_context->SetAssertStackSkipped(true);
        script_context->SetAssertEnableTrace(true);
    }

    // -- release the guard
    script_context->mDebuggerBreakLoopGuard = false;

    // -- we successfully handled the breakpoint with the loop
    return (true);
}

bool8 CCodeBlock::Execute(uint32 offset, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
#if DEBUG_CODEBLOCK
    if(GetDebugCodeBlock()) {
        printf("\n*** EXECUTING: %s\n\n", mFileName && mFileName[0] ? mFileName : "<stdin>");
    }
#endif

    // -- initialize the function return value
    GetScriptContext()->SetFunctionReturnValue(NULL, TYPE_NULL);

    const uint32* instrptr = GetInstructionPtr();
    instrptr += offset;

	while (instrptr != NULL) {

// -- Debugging is done through a remote connection, which right now is only supported through WIN32
#ifdef WIN32
#if TIN_DEBUGGER

        // -- see if there's a breakpoint set for this line
        CScriptContext* script_context = GetScriptContext();
        if (script_context->mDebuggerConnected && (funccallstack.mDebuggerBreakStep || HasBreakpoints()))
        {
            // -- get the current line number - see if we should break
            bool isNewLine = false;
            int32 cur_line = CalcLineNumber(instrptr, &isNewLine);

            // -- if we're stepping, and we're on a different line, or if
            // -- we're not stepping, and on a different line, and this new line has a breakpoint
            if ((funccallstack.mDebuggerBreakStep && funccallstack.mDebuggerLastBreak != cur_line) ||
                (!funccallstack.mDebuggerBreakStep && (isNewLine || cur_line != funccallstack.mDebuggerLastBreak) &&
                 mBreakpoints->FindItem(cur_line)))
            {

                DebuggerBreakLoop(this, instrptr, execstack, funccallstack);
            }
        }

#endif // TIN_DEBUGGER
#endif // WIN32

		// -- get the operation and process it
		eOpCode curoperation = (eOpCode)(*instrptr++);

        // -- execute the op - check the return value to ensure all operations are successful
        bool8 success = GetOpExecFunction(curoperation)(this, curoperation, instrptr, execstack, funccallstack);
        if(! success) {
            ScriptAssert_(GetScriptContext(), false, GetFileName(), CalcLineNumber(instrptr - 1),
                          "Error - Unable to execute OP:  %s\n", GetOperationString(curoperation));
            return (false);
        }

        // -- two notable exceptions - if the curoperation was either OP_FuncReturn or OP_EOF,
        // -- we're finished executing this codeblock
        if(curoperation == OP_FuncReturn || curoperation == OP_EOF) {
            return (true);
        }
	}

	// -- ran out of instructions, without a legitimate OP_EOF
	return false;
}

}  // TinScript

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
