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

#include "TinScript.h"
#include "TinCompile.h"
#include "TinNamespace.h"
#include "TinScheduler.h"
#include "TinExecute.h"

namespace TinScript {

static const int32 kExecStackSize = 2048;
static const int32 kExecFuncCallDepth = 1024;

// enable this for debug output while the executing the virtual machine
bool8 gDebugTrace = false;

void DebugTrace(eOpCode opcode, const char* fmt, ...) {
#if DEBUG_TRACE
    if(!gDebugTrace)
        return;
    // -- expand the formated buffer
    va_list args;
    va_start(args, fmt);
    char tracebuf[kMaxTokenLength];
    vsprintf_s(tracebuf, kMaxTokenLength, fmt, args);
    va_end(args);

    printf("OP [%s]: %s\n", GetOperationString(opcode), tracebuf);
#endif
}

void* GetStackVarAddr(CExecStack& execstack, CFunctionCallStack& funccallstack,
                      int32 stackvaroffset) {
    int32 stacktop = 0;
    CObjectEntry* oe = NULL;
    CFunctionEntry* fe = funccallstack.GetExecuting(oe, stacktop);
    if(!fe || stackvaroffset < 0) {
        ScriptAssert_(0, "<internal>", -1, "Error - GetStackVarAddr() failed\n");
        return NULL;
    }

    void* varaddr = execstack.GetStackVarAddr(stacktop, stackvaroffset);
    return varaddr;
}

bool8 GetStackValue(CExecStack& execstack, CFunctionCallStack& funccallstack,
                   void*& valaddr, eVarType& valtype, CVariableEntry*& ve, CObjectEntry*& oe) {
    // -- we'll always return a value, but if that comes from a var or an object member,
    // -- return those as well
    ve = NULL;
    oe = NULL;

	// -- if a variable was pushed, use the var addr instead
	if(valtype == TYPE__var || valtype == TYPE__hashvar) {
        uint32 val1ns = ((uint32*)valaddr)[0];
        uint32 val1func = ((uint32*)valaddr)[1];
        uint32 val1hash = ((uint32*)valaddr)[2];

        // -- one more level of dereference for variables that are actually hashtables
        uint32 val1hashvar = (valtype == TYPE__hashvar) ? ((uint32*)valaddr)[3] : 0;

		ve = GetVariable(GetGlobalNamespace()->GetVarTable(), val1ns, val1func, val1hash,
                         val1hashvar);
        if(!ve) {
            ScriptAssert_(0, "<internal>", -1, "Error - Unable to find variable %d\n",
                          UnHash(val1hash));
            return false;
        }
		valaddr = ve->GetAddr(NULL);
		valtype = ve->GetType();
	}
	// -- if a member was pushed, use the var addr instead
    else if(valtype == TYPE__member) {
        uint32 varhash = ((uint32*)valaddr)[0];
        uint32 varsource = ((uint32*)valaddr)[1];
        // -- find the object
        oe = CNamespace::FindObject(varsource);
        if(!oe) {
            ScriptAssert_(0, "<internal>", -1, "Error - Unable to find object %d\n", varsource);
            return false;
        }

        // -- find the variable entry from the object's namespace variable table
        ve = oe->GetVariableEntry(varhash);
		assert(ve != NULL);
        valaddr = ve->GetAddr(oe->GetAddr());
		valtype = ve->GetType();
    }
    // -- if a stack variable was pushed...
    else if(valtype == TYPE__stackvar) {
        // -- we already know to do a stackvar lookup - replace the var with the actual value type
        valtype = (eVarType)((uint32*)valaddr)[0];

        int32 stackvaroffset = ((uint32*)valaddr)[1];
        valaddr = GetStackVarAddr(execstack, funccallstack, stackvaroffset);
        if(!valaddr) {
            ScriptAssert_(0, "<internal>", -1, "Error - Unable to find stack var\n");
            return false;
        }
    }

    // -- if the valtype wasn't either a var or a member, they remain unchanged
    return true;
}

bool8 GetBinOpValues(CExecStack& execstack, CFunctionCallStack& funccallstack,
                    void*& val0, eVarType& val0type,
					void*& val1, eVarType& val1type) {

	// -- Note:  values come off the stack in reverse order
	// -- get the 2nd value
    CVariableEntry* ve1 = NULL;
    CObjectEntry* oe1 = NULL;
	val1 = execstack.Pop(val1type);
    if(!GetStackValue(execstack, funccallstack, val1, val1type, ve1, oe1))
        return false;

	// -- get the 1st value
    CVariableEntry* ve0 = NULL;
    CObjectEntry* oe0 = NULL;
	val0 = execstack.Pop(val0type);
    if(!GetStackValue(execstack, funccallstack, val0, val0type, ve0, oe0))
        return false;

	return true;
}

// -- this is to consolidate all the math operations that pop two values from the stack
// -- and compbine them... the operation is still responsible for handling pushing the result
bool8 PerformNumericalBinOp(CExecStack& execstack, CFunctionCallStack& funccallstack,
                           eOpCode op, float32& result) {

	// -- Get both args from the stacks
	eVarType val0type;
	void* val0 = NULL;
	eVarType val1type;
	void* val1 = NULL;
	if(!GetBinOpValues(execstack, funccallstack, val0, val0type, val1, val1type)) {
		printf("Error - failed GetBinopValues() for operation: %s\n",
				GetOperationString(op));
		return false;
	}
 
	// -- there are only two valid types to compare, floats, andints
	// -- any other form of "add" must be done through a function call
	// $$$TZA expand the TypeToString tables to include operations
    if((val0type != TYPE_int && val0type != TYPE_float && val0type != TYPE_bool) ||
        (val1type != TYPE_int && val1type != TYPE_float && val1type != TYPE_bool)) {
            ScriptAssert_(0, "<internal>", -1, "Error - trying to compare non-numeric types\n");
		return false;
    }
    void* val0addr = TypeConvert(val0type, val0, TYPE_float);
    void* val1addr = TypeConvert(val1type, val1, TYPE_float);
    float32 val0float = *(float32*)val0addr;
    float32 val1float = *(float32*)val1addr;

    // -- now perform the op
    switch(op) {
        case OP_Add:
            result = val0float + val1float;
            break;
        case OP_Sub:
            result = val0float - val1float;
            break;
        case OP_Mult:
            result = val0float * val1float;
            break;
        case OP_Div:
            ScriptAssert_(val1float != 0.0f, "<internal>", -1, "Error - Divide by 0\n");
            result = val0float / val1float;
            break;
        case OP_Mod:
        {
            ScriptAssert_(val1float != 0.0f, "<internal>", -1, "Error - Mod Divide by 0\n");
            int32 val0int = (int32)val0float;
            int32 val1int = val1float < 0.0f ? -(int32)val1float : (int32)val1float;
            while(val0int < 0)
                val0int += val1int;
            result = (float32)(val0int % val1int);
            break;
        }

        case OP_CompareEqual:
        case OP_CompareNotEqual:
        case OP_CompareLess:
        case OP_CompareLessEqual:
        case OP_CompareGreater:
        case OP_CompareGreaterEqual:
            result = val0float - val1float;
            break;

        case OP_BooleanAnd:
            result = (val0float != 0.0f && val1float != 0.0f);
            break;

        case OP_BooleanOr:
            result = (val0float != 0.0f || val1float != 0.0f);
            break;

        default:
            return false;
    }

    // -- success 
    return true;
}

// -- this is to consolidate all the math operations that pop two values from the stack
// -- and compbine them... the operation is still responsible for handling pushing the result
bool8 PerformIntegerBinOp(CExecStack& execstack, CFunctionCallStack& funccallstack,
                           eOpCode op, int32& result) {

	// -- Get both args from the stacks
	eVarType val0type;
	void* val0 = NULL;
	eVarType val1type;
	void* val1 = NULL;
	if(!GetBinOpValues(execstack, funccallstack, val0, val0type, val1, val1type)) {
		printf("Error - failed GetBinopValues() for operation: %s\n",
				GetOperationString(op));
		return false;
	}
 
	// -- there are only two valid types to compare, floats, andints
	// -- any other form of "add" must be done through a function call
	// $$$TZA expand the TypeToString tables to include operations
    if((val0type != TYPE_int && val0type != TYPE_float) ||
       (val1type != TYPE_int && val1type != TYPE_float)) {
            ScriptAssert_(0, "<internal>", -1, "Error - trying to compare non-int32 types\n");
		return false;
    }
    void* val0addr = TypeConvert(val0type, val0, TYPE_int);
    void* val1addr = TypeConvert(val1type, val1, TYPE_int);
    int32 val0int = *(int32*)val0addr;
    int32 val1int = *(int32*)val1addr;

    // -- now perform the op
    switch(op) {
    case OP_BitLeftShift:
            result = val0int << val1int;
            break;
    case OP_BitRightShift:
            result = val0int >> val1int;
            break;
    case OP_BitAnd:
            result = val0int & val1int;
            break;
        case OP_BitOr:
            result = val0int | val1int;
            break;
        case OP_BitXor:
            result = val0int ^ val1int;
            break;

        default:
            return false;
    }

    // -- success 
    return true;
}

bool8 PerformAssignOp(CExecStack& execstack, CFunctionCallStack& funccallstack, eOpCode op) {

	// -- pop the value
    CVariableEntry* ve1 = NULL;
    CObjectEntry* oe1 = NULL;
	eVarType val1type;
	void* val1addr = execstack.Pop(val1type);
    if(!GetStackValue(execstack, funccallstack, val1addr, val1type, ve1, oe1)) {
        ScriptAssert_(0, "<internal>", -1, "Error - Failed to pop assignment value\n");
        return false;
    }

	// -- pop the (hash) name of the var
    bool8 isstackvar = false;
    CVariableEntry* ve0 = NULL;
    CObjectEntry* oe0 = NULL;
	eVarType varhashtype;
	void* var = execstack.Pop(varhashtype);
    isstackvar = (varhashtype == TYPE__stackvar);
    if(!GetStackValue(execstack, funccallstack, var, varhashtype, ve0, oe0)) {
        ScriptAssert_(0, "<internal>", -1, "Error - Failed to pop assignment variable\n");
        return false;
    }

    // -- ensure we're assigning to a variable, an object member, or a local stack variable
    if(!ve0 && !isstackvar) {
        ScriptAssert_(0, "<internal>", -1, "Error - Attempting to assign to a non-variable\n");
        return false;
    }

    // -- if we're doing a straight up assignment, don't convert to float32
    if(op == OP_Assign )
    {
        if(isstackvar) {
            val1addr = TypeConvert(val1type, val1addr, varhashtype);
            memcpy(var, val1addr, MAX_TYPE_SIZE * sizeof(uint32));
            DebugTrace(op, "StackVar: %s", DebugPrintVar(val1addr, varhashtype));
        }
        else {
            val1addr = TypeConvert(val1type, val1addr, ve0->GetType());
    	    ve0->SetValue(oe0 ? oe0->GetAddr() : NULL, val1addr);
            DebugTrace(op, "Var %s: %s", UnHash(ve0->GetHash()),
                       DebugPrintVar(val1addr, ve0->GetType()));
        }
        return true;
    }

    void* ve0addr = isstackvar ? TypeConvert(varhashtype, var, TYPE_float)
                               : TypeConvert(ve0->GetType(), ve0->GetAddr(oe0), TYPE_float);
    val1addr = TypeConvert(val1type, val1addr, TYPE_float);
    float32 vefloat = *(float32*)ve0addr;
    float32 val1float = *(float32*)val1addr;

    // -- now perform the op
    float32 result = 0.0f;
    switch(op) {
        case OP_AssignAdd:
            result = vefloat + val1float;
            break;
        case OP_AssignSub:
            result = vefloat - val1float;
            break;
        case OP_AssignMult:
            result = vefloat * val1float;
            break;
        case OP_AssignDiv:
            ScriptAssert_(val1float != 0.0f, "<internal>", -1, "Error - Divide by 0\n");
            result = vefloat / val1float;
            break;
        case OP_AssignMod:
        {
            ScriptAssert_(val1float != 0.0f, "<internal>", -1, "Error - Mod Divide by 0\n");
            int32 val0int = (int32)vefloat;
            int32 val1int = val1float < 0.0f ? -(int32)val1float : (int32)val1float;
            while(val0int < 0)
                val0int += val1int;
            result = (float32)(val0int % val1int);
            break;
        }
    }

    // -- convert back to our variable type
    if(isstackvar) {
        void* convertptr = TypeConvert(TYPE_float, &result, varhashtype);
        if(!convertptr) {
            ScriptAssert_(0, "<internal>", -1, "Error - Unable to convert from type %s to %s\n",
                          GetRegisteredTypeName(TYPE_float),
                          GetRegisteredTypeName(varhashtype));
            return false;
        }
        memcpy(var, convertptr, MAX_TYPE_SIZE);
        DebugTrace(op, "Var %s: %s", UnHash(ve0->GetHash()),
                    DebugPrintVar(val1addr, ve0->GetType()));
    }
    else {
        void* convertptr = TypeConvert(TYPE_float, &result, ve0->GetType());
        if(!convertptr) {
            ScriptAssert_(0, "<internal>", -1, "Error - Unable to convert from type %s to %s\n",
                          GetRegisteredTypeName(TYPE_float),
                          GetRegisteredTypeName(ve0->GetType()));
            return false;
        }
	    ve0->SetValue(oe0 ? oe0->GetAddr() : NULL, convertptr);
        if(oe0) {
            DebugTrace(op, "Obj: %d, Var %s: %s", oe0->GetID(), UnHash(ve0->GetHash()),
                        DebugPrintVar(val1addr, ve0->GetType()));
        }
        else {
            DebugTrace(op, "Var %s: %s", UnHash(ve0->GetHash()),
                        DebugPrintVar(val1addr, ve0->GetType()));
        }
    }

	return true;
}

bool8 PerformBitAssignOp(CExecStack& execstack, CFunctionCallStack& funccallstack, eOpCode op) {

	// -- pop the value
    CVariableEntry* ve1 = NULL;
    CObjectEntry* oe1 = NULL;
	eVarType val1type;
	void* val1addr = execstack.Pop(val1type);
    if(!GetStackValue(execstack, funccallstack, val1addr, val1type, ve1, oe1)) {
        ScriptAssert_(0, "<internal>", -1, "Error - Failed to pop assignment value\n");
        return false;
    }

	// -- pop the (hash) name of the var
    bool8 isstackvar = false;
    CVariableEntry* ve0 = NULL;
    CObjectEntry* oe0 = NULL;
	eVarType varhashtype;
	void* var = execstack.Pop(varhashtype);
    isstackvar = (varhashtype == TYPE__stackvar);
    if(!GetStackValue(execstack, funccallstack, var, varhashtype, ve0, oe0)) {
        ScriptAssert_(0, "<internal>", -1, "Error - Failed to pop assignment variable\n");
        return false;
    }

    // -- ensure we're assigning to a variable, an object member, or a local stack variable
    if(!ve0 && !isstackvar) {
        ScriptAssert_(0, "<internal>", -1, "Error - Attempting to assign to a non-variable\n");
        return false;
    }

    void* ve0addr = isstackvar ? TypeConvert(varhashtype, var, TYPE_int)
                               : TypeConvert(ve0->GetType(), ve0->GetAddr(oe0), TYPE_int);
    val1addr = TypeConvert(val1type, val1addr, TYPE_int);
    int32 veint = *(int32*)ve0addr;
    int32 val1int = *(int32*)val1addr;

    // -- now perform the op
    int32 result = 0;
    switch(op) {
        case OP_AssignLeftShift:
            result = veint << val1int;
            break;
        case OP_AssignRightShift:
            result = veint >> val1int;
            break;
        case OP_AssignBitAnd:
            result = veint & val1int;
            break;
        case OP_AssignBitOr:
            result = veint | val1int;
            break;
        case OP_AssignBitXor:
            result = veint ^ val1int;
            break;
    }

    // -- convert back to our variable type
    if(isstackvar) {
        void* convertptr = TypeConvert(TYPE_int, &result, varhashtype);
        if(!convertptr) {
            ScriptAssert_(0, "<internal>", -1, "Error - Unable to convert from type %s to %s\n",
                          GetRegisteredTypeName(TYPE_int),
                          GetRegisteredTypeName(varhashtype));
            return false;
        }
        memcpy(var, convertptr, MAX_TYPE_SIZE);
        DebugTrace(op, "Var %s: %s", UnHash(ve0->GetHash()),
                   DebugPrintVar(val1addr, ve0->GetType()));
    }
    else {
        void* convertptr = TypeConvert(TYPE_int, &result, ve0->GetType());
        if(!convertptr) {
            ScriptAssert_(0, "<internal>", -1, "Error - Unable to convert from type %s to %s\n",
                          GetRegisteredTypeName(TYPE_int),
                          GetRegisteredTypeName(ve0->GetType()));
            return false;
        }
	    ve0->SetValue(oe0 ? oe0->GetAddr() : NULL, convertptr);
        if(oe0) {
            DebugTrace(op, "Obj: %d, Var %s: %s", oe0->GetID(), UnHash(ve0->GetHash()),
                       DebugPrintVar(val1addr, ve0->GetType()));
        }
        else {
            DebugTrace(op, "Var %s: %s", UnHash(ve0->GetHash()),
                       DebugPrintVar(val1addr, ve0->GetType()));
        }
    }

	return true;
}

bool8 PerformUnaryOp(CExecStack& execstack, CFunctionCallStack& funccallstack, eOpCode op) {
	// -- pop the value
    CVariableEntry* ve = NULL;
    CObjectEntry* oe = NULL;
	eVarType valtype;
	void* valaddr = execstack.Pop(valtype);
    if(!GetStackValue(execstack, funccallstack, valaddr, valtype, ve, oe)) {
        ScriptAssert_(0, "<internal>", -1, "Error - Failed to pop unary op value\n");
        return false;
    }

    // $$$TZA This seems clunky... need to consider a 2-D function map that handles all
    // -- type operations:  Convert, IsZero, Init, ToString, FromString, and all unary operations
    switch(op) {
        case OP_UnaryPreInc:
        case OP_UnaryPreDec:
        case OP_UnaryNeg:
        case OP_UnaryPos:
        {
            // -- verify the types - this is only valid for int32 and float32
            if(valtype != TYPE_int && valtype != TYPE_float) {
                ScriptAssert_(0, "<internal>", -1,
                              "Error - Only types int32 and float32 are supported for op: %s\n",
                              GetOperationString(op));
                return false;
            }

            void* result = NULL;
            int32 intresult = 0;
            float32 floatresult = 0.0f;

            if(valtype == TYPE_int) {
                intresult = *(int32*)valaddr;

                // -- perform the actual operation
                if(op == OP_UnaryPreInc)
                    ++intresult;
                else if(op == OP_UnaryPreDec)
                    --intresult;
                else if(op == OP_UnaryNeg)
                    intresult = -intresult;
                // -- UnaryPos has no effect

                result = (void*)&intresult;
            }
            else {
                floatresult = *(float32*)valaddr;

                // -- perform the actual operation
                if(op == OP_UnaryPreInc)
                    ++floatresult;
                else if(op == OP_UnaryPreDec)
                    --floatresult;
                else if(op == OP_UnaryNeg)
                    floatresult = -floatresult;
                // -- UnaryPos has no effect

                result = (void*)&floatresult;
            }

	        // -- push the the value onto the stack
	        execstack.Push(result, valtype);
            DebugTrace(op, "result: %s", DebugPrintVar(result, valtype));
            return true;
        }

        case OP_UnaryBitInvert:
        {
            // -- verify the types - this is only valid for int32 and float32
            if(valtype != TYPE_int) {
                ScriptAssert_(0, "<internal>", -1,
                              "Error - Only type int32 is supported for op: %s\n",
                              GetOperationString(op));
                return false;
            }
            int32 value = *(int32*)valaddr;
            value = ~value;

	        // -- push the the value onto the stack
	        execstack.Push(&value, valtype);
            DebugTrace(op, "result: %s", DebugPrintVar(&value, valtype));
            return true;
        }

        case OP_UnaryNot:
        {
            // -- verify the types - this is only valid for int32 and float32
            if(valtype != TYPE_bool) {
                ScriptAssert_(0, "<internal>", -1,
                              "Error - Only type bool8 is supported for op: %s\n",
                              GetOperationString(op));
                return false;
            }
            bool8 value = *(bool8*)valaddr;
            value = !value;

	        // -- push the the value onto the stack
	        execstack.Push(&value, valtype);
            DebugTrace(op, "result: %s", DebugPrintVar(&value, valtype));
            return true;
        }

        default:
        {
            ScriptAssert_(0, "<internal>", -1,
                            "Error - unsupported (non unary?) op: %s\n",
                            GetOperationString(op));
            return false;
        }
    }

    return true;
}

bool8 CopyStackParameters(CFunctionEntry* fe, CObjectEntry* oe, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {

    // -- sanity check
    if(fe == NULL || !fe->GetContext()) {
        ScriptAssert_(0, "<internal>", -1, "Error - invalid function entry\n");
        return false;
    }

    // -- initialize the parameters of our fe with the function context
    CFunctionContext* parameters = fe->GetContext();
    int32 srcparamcount = parameters->GetParameterCount();
    for(int32 i = 0; i < srcparamcount; ++i) {
        CVariableEntry* src = parameters->GetParameter(i);
        void* dst = GetStackVarAddr(execstack, funccallstack, src->GetStackOffset());
        if(!dst) {
            ScriptAssert_(0, "<internal>", -1,
                          "Error - unable to assign parameter %d, calling function %s()\n",
                          i, UnHash(fe->GetHash()));
            return false;
        }

        // -- set the value
        if(src)
            memcpy(dst, src->GetAddr(oe ? oe->GetAddr() : NULL),
                   MAX_TYPE_SIZE * sizeof(uint32));
        else
            memset(dst, 0, MAX_TYPE_SIZE * sizeof(uint32));
    }

    return true;
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
        CopyStackParameters(fe, oe, execstack, funccallstack);

        CCodeBlock* funccb = NULL;
        uint32 funcoffset = fe->GetCodeBlockOffset(funccb);
        if(!funccb) {
            ScriptAssert_(0, "<internal>", -1, "Error - Undefined function: %s()\n",
                            UnHash(fe->GetHash()));
            return false;
        }

        // -- execute the function via codeblock/offset
        bool8 success = funccb->Execute(funcoffset, execstack, funccallstack);
        if(!success) {
            ScriptAssert_(0, "<internal>", -1, "Error - error executing function: %s()\n",
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

        // -- since we called a 'C' function, there's no OP_FuncReturn - pop the stack
        funccallstack.Pop(oe);
    }

    return true;
}

bool8 ExecuteCodeBlock(CCodeBlock& codeblock) {

	// -- create the stack to use for the execution
	CExecStack execstack(kExecStackSize);
    CFunctionCallStack funccallstack(kExecFuncCallDepth);

    return codeblock.Execute(0, execstack, funccallstack);
}

bool8 ExecuteScheduledFunction(uint32 objectid, uint32 funchash,
                              CFunctionContext* parameters) {

    // -- sanity check
    if(funchash == 0 && parameters == NULL) {
        ScriptAssert_(0, "<internal>", -1, "Error - invalid funchash/parameters\n");
        return false;
    }

    // -- see if this is a method or a function
    CObjectEntry* oe = NULL;
    CFunctionEntry* fe = NULL;
    if(objectid != 0) {
        // -- find the object
        oe = CNamespace::FindObject(objectid);
        if(!oe) {
            ScriptAssert_(0, "<internal>", -1, "Error - unable to find object: %d\n", objectid);
            return false;
        }

        // -- get the namespace, then the function
        fe = oe->GetFunctionEntry(0, funchash);
    }
    else {
        fe = GetGlobalNamespace()->GetFuncTable()->FindItem(funchash);
    }

    // -- ensure we found our function
    if(!fe) {
        ScriptAssert_(0, "<internal>", -1, "Error - unable to find function: %s\n", UnHash(funchash));
        return false;
    }

	// -- create the stack to use for the execution
	CExecStack execstack(kExecStackSize);
    CFunctionCallStack funccallstack(kExecFuncCallDepth);

    // -- initialize the parameters of our fe with the function context
    int32 srcparamcount = parameters->GetParameterCount();
    for(int32 i = 0; i < srcparamcount; ++i) {
        CVariableEntry* src = parameters->GetParameter(i);
        CVariableEntry* dst = fe->GetContext()->GetParameter(i);
        if(!dst) {
            ScriptAssert_(0, "<internal>", -1,
                          "Error - unable to assign parameter %d, calling function %s()\n",
                          i, UnHash(funchash));
            return false;
        }

        // -- ensure the type of the parameter value is converted to the type required
        int32 nullvalue = 0;
        void* srcaddr = NULL;
        if(src)
            srcaddr = TypeConvert(src->GetType(), src->GetAddr(NULL), dst->GetType());
        else
            srcaddr = TypeConvert(TYPE_int, &nullvalue, dst->GetType());

        // -- set the value
        dst->SetValue(oe ? oe->GetAddr() : NULL, srcaddr);
    }

    // -- initialize any remaining parameters
    int32 dstparamcount = fe->GetContext()->GetParameterCount();
    for(int32 i = srcparamcount; i < dstparamcount; ++i) {
        CVariableEntry* dst = fe->GetContext()->GetParameter(i);
        int32 nullvalue = 0;
        void* srcaddr = TypeConvert(TYPE_int, &nullvalue, dst->GetType());
        dst->SetValue(oe ? oe->GetAddr() : NULL, srcaddr);
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
        ScriptAssert_(0, "<internal>", -1,
                        "Error - Unable to call function: %s()\n",
                        UnHash(fe->GetHash()));
        return false;
    }

    return true;
}

bool8 CCodeBlock::Execute(uint32 offset, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
#if DEBUG_CODEBLOCK
    if(GetDebugCodeBlock()) {
        printf("\n*** EXECUTING: %s\n\n", filename && filename[0] ? filename : "<stdin>");
    }
#endif

    CScheduler::CCommand* currentschedule = NULL;

    const uint32* instrptr = GetInstructionPtr();
    instrptr += offset;

	while (instrptr != NULL) {

		// -- get the operation and process it
		eOpCode curoperation = (eOpCode)(*instrptr++);
		switch (curoperation) {
			case OP_NOP:
                DebugTrace(curoperation, "");
				break;

            case OP_VarDecl:
            {
                uint32 varhash = *instrptr++;
				eVarType vartype = (eVarType)(*instrptr++);

                // -- if we're in the middle of a function definition, the var is local
                // -- otherwise it's global... there are no nested function definitions allowed
                int32 stacktop = 0;
                CObjectEntry* oe = NULL;
                AddVariable(GetGlobalNamespace()->GetVarTable(), funccallstack.GetTop(oe, stacktop),
                            UnHash(varhash), varhash, vartype); 
                DebugTrace(curoperation, "Var: %s", UnHash(varhash));
                break;
            }

            case OP_ParamDecl:
            {
                uint32 varhash = *instrptr++;
				eVarType vartype = (eVarType)(*instrptr++);

                int32 stacktop = 0;
                CObjectEntry* oe = NULL;
                CFunctionEntry* fe = funccallstack.GetTop(oe, stacktop);
                assert(fe != NULL);
                fe->GetContext()->AddParameter(UnHash(varhash), varhash, vartype);
                DebugTrace(curoperation, "Var: %s", UnHash(varhash));
                break;
            }

			case OP_Assign:
            case OP_AssignAdd:
            case OP_AssignSub:
            case OP_AssignMult:
            case OP_AssignDiv:
            case OP_AssignMod:
			{
                if(!PerformAssignOp(execstack, funccallstack, curoperation)) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - unable to perform op: %s\n",
                                  GetOperationString(curoperation));
                    return false;
                }
				break;
			}

            case OP_AssignLeftShift:
            case OP_AssignRightShift:
            case OP_AssignBitAnd:
            case OP_AssignBitOr:
            case OP_AssignBitXor:
			{
                if(!PerformBitAssignOp(execstack, funccallstack, curoperation)) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - unable to perform op: %s\n",
                                  GetOperationString(curoperation));
                    return false;
                }
				break;
			}

            case OP_UnaryPreInc:
            case OP_UnaryPreDec:
            case OP_UnaryNeg:
            case OP_UnaryPos:
            case OP_UnaryBitInvert:
            case OP_UnaryNot:
            {
                if(!PerformUnaryOp(execstack, funccallstack, curoperation))
                {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                 "Error - unable to perform op: %s\n",
                                 GetOperationString(curoperation));
                    return false;
                }
                break;
            }

			case OP_Push:
			{
				// -- next instruction is the type
				eVarType contenttype = (eVarType)(*instrptr);
				instrptr++;
				assert(contenttype >= 0 && contenttype < TYPE_COUNT);

				// -- push the the value onto the stack, and update the instrptr
				execstack.Push((void*)instrptr, contenttype);
                DebugTrace(curoperation, "%s", DebugPrintVar((void*)instrptr, contenttype));

				// -- advance the instruction pointer
				int32 contentsize = kBytesToWordCount(gRegisteredTypeSize[contenttype]);
				instrptr += contentsize;

				break;
			}

			case OP_PushLocalVar:
			{
				// -- next instruction is the variable hash followed by the function context hash
				execstack.Push((void*)instrptr, TYPE__stackvar);
                DebugTrace(curoperation, "StackVar [%s : %d]",
                           GetRegisteredTypeName((eVarType)instrptr[0]), instrptr[1]);

				// -- advance the instruction pointer
				int32 contentsize = kBytesToWordCount(gRegisteredTypeSize[TYPE__stackvar]);
				instrptr += contentsize;

				break;
			}

			case OP_PushLocalValue:
			{
                // -- next instruction is the type
				eVarType valtype = (eVarType)(*instrptr++);

				// -- next instruction is the stack offset
                int32 stackoffset = (int32)*instrptr++;

                // -- get the stack top for this function call
                void* stackvaraddr = GetStackVarAddr(execstack, funccallstack, stackoffset);
                if(!stackvaraddr) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - Unable to get StackVarAddr()\n");
                    return false;
                }

				execstack.Push(stackvaraddr, valtype);
                DebugTrace(curoperation, "StackVar [%d]: %s", stackoffset,
                           DebugPrintVar(stackvaraddr, valtype));
				break;
			}

			case OP_PushGlobalVar:
			{
				// -- next instruction is the variable hash followed by the function context hash
				execstack.Push((void*)instrptr, TYPE__var);
                DebugTrace(curoperation, "Var: %s", UnHash(instrptr[2]));

				// -- advance the instruction pointer
				int32 contentsize = kBytesToWordCount(gRegisteredTypeSize[TYPE__var]);
				instrptr += contentsize;

				break;
			}

			case OP_PushGlobalValue:
			{
				// -- next instruction is the variable name
                uint32 nshash = *instrptr++;
				uint32 varfunchash = *instrptr++;
				uint32 varhash = *instrptr++;
				CVariableEntry* ve = GetVariable(GetGlobalNamespace()->GetVarTable(), nshash, varfunchash,
                                                 varhash, 0);
				assert(ve != NULL);
				void* val = ve->GetAddr(NULL);
				eVarType valtype = ve->GetType();

				execstack.Push(val, valtype);
                DebugTrace(curoperation, "Var: %s, %s", UnHash(varhash), DebugPrintVar(val, valtype));

				break;
			}

            case OP_PushArrayVar:
            {
				// -- hash value will have already been pushed
				eVarType contenttype;
				void* contentptr = execstack.Pop(contenttype);
                if(contenttype != TYPE_int) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - ExecStack should contain TYPE_int\n");
                    return false;
                }
                uint32 arrayvarhash = *(uint32*)contentptr;

                // -- the next instructions specify the variable representing the hash table
                uint32 nshash = *instrptr++;
				uint32 varfunchash = *instrptr++;
				uint32 varhash = *instrptr++;

                // -- push the hashvar
                uint32 arrayvar[4];
                arrayvar[0] = nshash;
                arrayvar[1] = varfunchash;
                arrayvar[2] = varhash;
                arrayvar[3] = arrayvarhash;

				// -- next instruction is the variable hash followed by the function context hash
				execstack.Push((void*)arrayvar, TYPE__hashvar);

                break;
            }

            case OP_PushArrayValue:
            {
				// -- hash value will have already been pushed
				eVarType contenttype;
				void* contentptr = execstack.Pop(contenttype);
                if(contenttype != TYPE_int) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - ExecStack should contain TYPE_int\n");
                    return false;
                }
                uint32 arrayvarhash = *(uint32*)contentptr;

				// -- next instruction is the variable name
                uint32 nshash = *instrptr++;
				uint32 varfunchash = *instrptr++;
				uint32 varhash = *instrptr++;
				CVariableEntry* ve = GetVariable(GetGlobalNamespace()->GetVarTable(), nshash, varfunchash,
                                                 varhash, arrayvarhash);
				assert(ve != NULL);

				eVarType vetype = ve->GetType();
                void* veaddr = ve->GetAddr(NULL);

				execstack.Push(veaddr, vetype);
                break;
            }

			case OP_PushMember:
			{
				// -- next instruction is the member name
				uint32 varhash = *instrptr++;

                // -- what will previously have been pushed on the stack, is the object ID
				eVarType contenttype;
				void* contentptr = execstack.Pop(contenttype);
                if(contenttype != TYPE_object) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - ExecStack should contain TYPE_object\n");
                    return false;
                }

                // -- TYPE_object is actually just an uint32 ID
                // -- a member, a memberhash followed by the ID of the object
                uint32 member[2];
                member[0] = varhash;
                member[1] = *(uint32*)contentptr;

				// -- next instruction is the variable hash followed by the function context hash
				execstack.Push((void*)member, TYPE__member);

				break;
			}

			case OP_PushMemberVal:
			{
				// -- next instruction is the member name
				uint32 varhash = *instrptr++;

                // -- what will previously have been pushed on the stack, is the object ID
				eVarType contenttype;
				void* contentptr = execstack.Pop(contenttype);
                if(contenttype != TYPE_object) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr), "Error - ExecStack should contain TYPE_object\n");
                    return false;
                }

                // -- TYPE_object is actually just an uint32 ID
                uint32 objectid = *(uint32*)contentptr;

                // -- find the object
                CObjectEntry* oe = CNamespace::FindObject(objectid);
                if(!oe) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr), "Error - Unable to find object %d\n", objectid);
                    return false;
                }

                // -- find the variable entry from the object's namespace variable table
                CVariableEntry* ve = oe->GetVariableEntry(varhash);
                if(!ve) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr), "Error - Unable to find member %s for object %d\n",
                                  UnHash(varhash), objectid);
                    return false;
                }
				assert(ve != NULL);
                void* val = ve->GetAddr(oe->GetAddr());
				eVarType valtype = ve->GetType();

                // -- push the value of the member
				execstack.Push(val, valtype);

				break;
			}

            case OP_PushSelf:
            {
                CObjectEntry* oe = NULL;
                CFunctionEntry* fe = funccallstack.GetTopMethod(oe);
                if(!fe || !oe) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr), "Error - PushSelf from outside a method\n");
                    return false;
                }

                // -- need to push the variable
                uint32 objid = oe->GetID();
				execstack.Push((void*)&objid, TYPE_object);

                break;
            }

			// -- normally popping the stack is part of performing an operation...
            // -- this can happen when we ignore the return value of a function call
			case OP_Pop:
			{
				eVarType contenttype;
				void* contentptr = execstack.Pop(contenttype);
				break;
			}

			case OP_Add:
			case OP_Sub:
			case OP_Mult:
			case OP_Div:
			case OP_Mod:
			{
                float32 result = 0.0f;
                if(!PerformNumericalBinOp(execstack, funccallstack, curoperation, result)) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - unable to perform op: %s\n",
                                  GetOperationString(curoperation));
                    return false;
                }
				execstack.Push((void*)&result, TYPE_float);
                DebugTrace(curoperation, "%.2f", result);
				break;
			}

			case OP_BooleanAnd:
			case OP_BooleanOr:
			{
                float32 result = 0.0f;
                if(!PerformNumericalBinOp(execstack, funccallstack, curoperation, result)) {
                    return false;
                }
                bool8 boolresult = (result != 0.0f);
				execstack.Push((void*)&boolresult, TYPE_bool);
				break;
			}

			case OP_CompareEqual:
			{
                float32 result = 0.0f;
                if(!PerformNumericalBinOp(execstack, funccallstack, curoperation, result)) {
                    return false;
                }
                bool8 boolresult = (result == 0.0f);
				execstack.Push((void*)&boolresult, TYPE_bool);
                DebugTrace(curoperation, "%s", boolresult ? "true" : "false");
				break;
			}

			case OP_CompareNotEqual:
			{
                float32 result = 0.0f;
                if(!PerformNumericalBinOp(execstack, funccallstack, curoperation, result)) {
                    return false;
                }
                bool8 boolresult = (result != 0.0f);
				execstack.Push((void*)&boolresult, TYPE_bool);
                DebugTrace(curoperation, "%s", boolresult ? "true" : "false");
				break;
			}

			case OP_CompareLess:
			{
                float32 result = 0.0f;
                if(!PerformNumericalBinOp(execstack, funccallstack, curoperation, result)) {
                    return false;
                }
                bool8 boolresult = (result < 0.0f);
				execstack.Push((void*)&boolresult, TYPE_bool);
                DebugTrace(curoperation, "%s", boolresult ? "true" : "false");
				break;
			}

			case OP_CompareLessEqual:
			{
                float32 result = 0.0f;
                if(!PerformNumericalBinOp(execstack, funccallstack, curoperation, result)) {
                    return false;
                }
                bool8 boolresult = (result <= 0.0f);
				execstack.Push((void*)&boolresult, TYPE_bool);
                DebugTrace(curoperation, "%s", boolresult ? "true" : "false");
				break;
			}

			case OP_CompareGreater:
			{
                float32 result = 0.0f;
                if(!PerformNumericalBinOp(execstack, funccallstack, curoperation, result)) {
                    return false;
                }
                bool8 boolresult = (result > 0.0f);
				execstack.Push((void*)&boolresult, TYPE_bool);
                DebugTrace(curoperation, "%s", boolresult ? "true" : "false");
				break;
			}

			case OP_CompareGreaterEqual:
			{
                float32 result = 0.0f;
                if(!PerformNumericalBinOp(execstack, funccallstack, curoperation, result)) {
		            ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - Operation failed: %s\n",
                                  GetOperationString(curoperation));
                    return false;
                }
                bool8 boolresult = (result >= 0.0f);
				execstack.Push((void*)&boolresult, TYPE_bool);
                DebugTrace(curoperation, "%s", boolresult ? "true" : "false");
				break;
			}

            case OP_BitLeftShift:
            case OP_BitRightShift:
            case OP_BitAnd:
            case OP_BitOr:
            case OP_BitXor:
            {
                int32 result = 0;
                if(!PerformIntegerBinOp(execstack, funccallstack, curoperation, result)) {
		            ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - Operation failed: %s\n",
                                  GetOperationString(curoperation));
                    return false;
                }
				execstack.Push((void*)&result, TYPE_int);
                DebugTrace(curoperation, "%d", result);
				break;
            }

			case OP_Branch:
			{
				int32 jumpcount = *instrptr++;
				instrptr += jumpcount;
                DebugTrace(curoperation, "");
				break;
			}

			case OP_BranchTrue:
			{
				int32 jumpcount = *instrptr++;

				// -- top of the stack had better be a bool8
				eVarType valtype;
				void* valueraw = execstack.Pop(valtype);
				assert(valtype == TYPE_bool);

				// -- branch
				bool8 value = *(bool8*)valueraw;
				if(value) {
					instrptr += jumpcount;
				}
                DebugTrace(curoperation, "%s", value ? "true" : "false");

				break;
			}

			case OP_BranchFalse:
			{
				int32 jumpcount = *instrptr++;

				// -- top of the stack had better be a bool8
				eVarType valtype;
				void* valueraw = execstack.Pop(valtype);
				assert(valtype == TYPE_bool);

				// -- branch
				bool8 value = *(bool8*)valueraw;
				if(!value) {
					instrptr += jumpcount;
				}
                DebugTrace(curoperation, "%s", !value ? "true" : "false");

				break;
			}

            case OP_FuncDecl:
            {
				uint32 funchash = *instrptr++;
				uint32 namespacehash = *instrptr++;
                uint32 funcoffset = *instrptr++;
                CFunctionEntry* fe = FuncDeclaration(namespacehash, UnHash(funchash),
                                                     funchash, eFuncTypeScript);
	            if(!fe) {
		            ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - failed to declare function - hash: 0x%08x\n",
                                  funchash);
		            return false;
	            }

                // -- this being a script function, set the offset, and add this function
                // -- to the list of functions this codeblock implements
                fe->SetCodeBlockOffset(this, funcoffset);

                // -- push the function entry onto the call stack, so all var declarations
                // -- will be associated with this function
                funccallstack.Push(fe, NULL, execstack.GetStackTop());
                DebugTrace(curoperation, "%s", UnHash(fe->GetHash()));

                break;
            }

            case OP_FuncDeclEnd:
            {
                // -- push the function stack
                CObjectEntry* oe = NULL;
                CFunctionEntry* fe = funccallstack.Pop(oe);
                fe->GetContext()->InitStackVarOffsets();
                DebugTrace(curoperation, "%s", UnHash(fe->GetHash()));

                break;
            }

			case OP_FuncCallArgs:
			{
				// -- get the hash of the function name
				uint32 nshash = *instrptr++;
				uint32 funchash = *instrptr++;
                tFuncTable* functable = CNamespace::FindNamespace(nshash)->GetFuncTable();
	            CFunctionEntry* fe = functable->FindItem(funchash);
	            if(!fe) {
                    if(nshash != 0) {
		                ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                      "Error - undefined function: %s::%s()\n",
                                      UnHash(nshash), UnHash(funchash));
                    }
                    else
                    {
		                ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                      "Error - undefined function: %s()\n",
                                      UnHash(funchash));
                    }
		            return false;
	            }

                // -- push the function entry onto the call stack
                funccallstack.Push(fe, NULL, execstack.GetStackTop());
                DebugTrace(curoperation, "%s", UnHash(fe->GetHash()));

                // -- create space on the execstack, if this is a script function
                if(fe->GetType() != eFuncTypeGlobal) {
                    int32 localvarcount = fe->GetLocalVarTable()->Used();
                    execstack.Reserve(localvarcount * MAX_TYPE_SIZE);
                }

                break;
            }

            case OP_PushParam:
            {
                // -- the next word is the parameter index for the current function we're calling
                uint32 paramindex = *instrptr++;

                // -- get the function about to be called
                int32 stackoffset = 0;
                CObjectEntry* oe = NULL;
            	CFunctionEntry* fe = funccallstack.GetTop(oe, stackoffset);
                if(!fe) {
		            ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - assigning parameters outside a function call\n");
		            return false;
                }
                uint32 paramcount = fe->GetContext()->GetParameterCount();
                if(paramindex >= paramcount) {
		            ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - too many parameters calling function: %s\n",
                                  UnHash(fe->GetHash()));
		            return false;
                }

                // -- get the parameter
                CVariableEntry* ve = fe->GetContext()->GetParameter(paramindex);

                // -- push the variable onto the stack
                uint32 varbuf[3];
                varbuf[0] = fe->GetNamespaceHash();
                varbuf[1] = fe->GetHash();
                varbuf[2] = ve->GetHash();
				execstack.Push((void*)varbuf, TYPE__var);

                DebugTrace(curoperation, "%s, param %d", UnHash(fe->GetHash()), paramindex);

                break;
            }

   			case OP_MethodCallArgs:
			{
				// -- get the hash of the namespace, in case we want a specific one
				uint32 nshash = *instrptr++;

				// -- get the hash of the method name
				uint32 methodhash = *instrptr++;

                // -- what will previously have been pushed on the stack, is the object ID
				eVarType contenttype;
				void* contentptr = execstack.Pop(contenttype);
                if(contenttype != TYPE_object) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr), "Error - ExecStack should contain TYPE_object\n");
                    return false;
                }

                // -- TYPE_object is actually just an uint32 ID
                uint32 objectid = *(uint32*)contentptr;

                // -- find the object
                CObjectEntry* oe = CNamespace::FindObject(objectid);
                if(!oe) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - Unable to find object %d\n", objectid);
                    return false;
                }

                // -- find the method entry from the object's namespace hierarachy
                // -- if nshash is 0, then it's from the top of the hierarchy
                CFunctionEntry* fe = oe->GetFunctionEntry(nshash, methodhash);
                if(!fe) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - Unable to find method %s for object %d\n",
                                  UnHash(methodhash), objectid);
                    return false;
                }

                // -- push the function entry onto the call stack
                funccallstack.Push(fe, oe, execstack.GetStackTop());

                // -- create space on the execstack, if this is a script function
                if(fe->GetType() != eFuncTypeGlobal) {
                    int32 localvarcount = fe->GetLocalVarTable()->Used();
                    execstack.Reserve(localvarcount * MAX_TYPE_SIZE);
                }

                DebugTrace(curoperation, "obj: %d, ns: %s, func: %s", objectid, UnHash(nshash),
                           UnHash(fe->GetHash()));
                break;
            }

            case OP_FuncCall:
            {
                int32 stackoffset = 0;
                CObjectEntry* oe = NULL;
                CFunctionEntry* fe = funccallstack.GetTop(oe, stackoffset);
                assert(fe != NULL);

                // -- notify the stack that we're now actually executing the top function
                // -- this is to ensure that stack variables now reference this function's
                // -- reserved space on the stack.
                funccallstack.BeginExecution();

                DebugTrace(curoperation, "func: %s", UnHash(fe->GetHash()));

                bool8 result = CodeBlockCallFunction(fe, oe, execstack, funccallstack);
                if(!result) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - Unable to call function: %s()\n",
                                  UnHash(fe->GetHash()));
                    return false;
                }
                break;
            }

            case OP_FuncReturn:
            {
                // -- pop the function entry from the stack
                CObjectEntry* oe = NULL;
                CFunctionEntry* fe = funccallstack.Pop(oe);
                if(!fe) {
		            ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - return with no function\n");
                    return false;
                }

                // -- pop the return value while we unreserve the local var space on the stack
                uint32 stacktopcontent[MAX_TYPE_SIZE];

	            // -- pop the value of the next string to append to the hash
	            eVarType contenttype;
	            void* content = execstack.Pop(contenttype);
                memcpy(stacktopcontent, content, MAX_TYPE_SIZE * sizeof(uint32));

                // -- unreserve space from the exec stack
                int32 localvarcount = fe->GetLocalVarTable()->Used();
                execstack.UnReserve(localvarcount * MAX_TYPE_SIZE);

                // -- re-push the stack top contents
                execstack.Push((void*)stacktopcontent, contenttype);

                DebugTrace(curoperation, "func: %s, val: %s", UnHash(fe->GetHash()),
                           DebugPrintVar(stacktopcontent, contenttype));

                // -- return from this execution loop
                return true;
            }

            case OP_ArrayHash:
            {
	            // -- pop the value of the next string to append to the hash
                CVariableEntry* ve1 = NULL;
                CObjectEntry* oe1 = NULL;
	            eVarType val1type;
	            void* val1 = execstack.Pop(val1type);
                if(!GetStackValue(execstack, funccallstack, val1, val1type, ve1, oe1)) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - Failed to pop string to hash\n");
                    return false;
                }
                // -- ensure it actually is a string
                void* val1addr = TypeConvert(val1type, val1, TYPE_string);

                // -- get the current hash
				eVarType contenttype;
				void* contentptr = execstack.Pop(contenttype);
                if(contenttype != TYPE_int) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - ExecStack should contain TYPE_object\n");
                    return false;
                }

                // -- calculate the updated hash
                uint32 hash = *(uint32*)contentptr;
                hash = HashAppend(hash, "_");
                hash = HashAppend(hash, (const char*)val1addr);

                // -- push the result
                execstack.Push((void*)&hash, TYPE_int);

                break;
            }

            case OP_ArrayVarDecl:
            {
                // -- next instruction is the type
				eVarType vartype = (eVarType)(*instrptr++);

                // -- pull the hash value for the hash table entry
				eVarType contenttype;
				void* contentptr = execstack.Pop(contenttype);
                if(contenttype != TYPE_int) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - ExecStack should contain TYPE_int\n");
                    return false;
                }
                uint32 hashvalue = *(uint32*)contentptr;

                // -- pull the hashtable variable off the stack
                CVariableEntry* ve0 = NULL;
                CObjectEntry* oe0 = NULL;
                eVarType val0type;
	            void* val0 = execstack.Pop(val0type);
                if(!GetStackValue(execstack, funccallstack, val0, val0type, ve0, oe0)) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - ExecStack should contain a hashtable variable\n");
                    return false;
                }
                if(val0type != TYPE_hashtable) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - ExecStack should contain hashtable variable\n");
                    return false;
                }

                // -- now that we've got our hashtable var, and the type, create (or verify)
                // -- the hash table entry
                tVarTable* hashtable = (tVarTable*)ve0->GetAddr(oe0 ? oe0->GetAddr() : NULL);
                CVariableEntry* hte = hashtable->FindItem(hashvalue);
                // -- if the entry already exists, ensure it's the same type
                if(hte && hte->GetType() != vartype) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                        "Error - HashTable variable: %s already has an entry of type: %s\n",
                        UnHash(ve0->GetHash()), GetRegisteredTypeName(hte->GetType()));
                    return false;
                }
                // -- otherwise add the variable entry to the hash table
                else if(! hte) {
                    CVariableEntry* hte = TinAlloc(ALLOC_VarEntry, CVariableEntry,
                                                   UnHash(hashvalue), hashvalue, vartype,
                                                   false, 0, false);
                    hashtable->AddItem(*hte, hashvalue);
                }

                break;
            }

            case OP_SelfVarDecl:
            {
                // -- next instruction is the variable hash
                uint32 varhash = *instrptr++;

                // -- followed by the type
				eVarType vartype = (eVarType)(*instrptr++);

                // -- get the object from the stack
                CObjectEntry* oe = NULL;
                CFunctionEntry* fe = funccallstack.GetTopMethod(oe);
                if(!fe || !oe) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - Unable to declare a self.var from outside a method\n");
                    return false;
                }

                // -- add the dynamic variable
                CNamespace::AddDynamicVariable(oe->GetID(), varhash, vartype);
                break;
            }

            case OP_ScheduleBegin:
            {
                // -- ensure we're not in the middle of a schedule construction already
                if(currentschedule != NULL) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - A schedule() is already being processed\n");
                    return false;
                }

                // -- get the function hash
                uint32 funchash = *instrptr++;

                // -- get the delay time
                int32 delaytime = *instrptr++;

                // -- pull the object ID off the stack
				eVarType contenttype;
				void* contentptr = execstack.Pop(contenttype);
                if(contenttype != TYPE_object) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr), "Error - ExecStack should contain TYPE_object\n");
                    return false;
                }

                // -- TYPE_object is actually just an uint32 ID
                uint32 objectid = *(uint32*)contentptr;

                // -- create the schedule command
                currentschedule = CScheduler::ScheduleCreate(objectid, delaytime, funchash);

                break;
            }

            case OP_ScheduleParam:
            {
                // -- ensure we are in the middle of a schedule construction
                if(currentschedule == NULL) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - There is no schedule() being processed\n");
                    return false;
                }

                // -- get the parameter index
                int32 paramindex = *instrptr++;

                // -- pull the parameter value off the stack
				eVarType contenttype;
				void* contentptr = execstack.Pop(contenttype);

                // -- add the parameter to the function context, inheriting the type from whatever
                // -- was pushed
                char varnamebuf[32];
                sprintf_s(varnamebuf, 32, "_%d", paramindex);
                currentschedule->funccontext->AddParameter(varnamebuf, Hash(varnamebuf), contenttype, paramindex);

                // -- assign the value
                CVariableEntry* ve = currentschedule->funccontext->GetParameter(paramindex);
                ve->SetValue(NULL, contentptr);

                // -- copy the value 

                break;
            }

            case OP_ScheduleEnd:
            {
                // -- ensure we are in the middle of a schedule construction
                if(currentschedule == NULL) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr),
                                  "Error - There is no schedule() being processed\n");
                    return false;
                }

                // -- push the schedule request ID onto the stack
                int32 reqid = currentschedule->reqid;
                currentschedule = NULL;

				execstack.Push(&reqid, TYPE_int);

                break;
            }

            case OP_CreateObject:
            {
                uint32 classhash = *instrptr++;
                uint32 objnamehash = *instrptr++;
                uint32 objid = CNamespace::CreateObject(classhash, objnamehash);

   				// -- push the objid onto the stack, and update the instrptr
				execstack.Push(&objid, TYPE_object);

                break;
            }

            case OP_DestroyObject:
            {
                // -- what will previously have been pushed on the stack, is the object ID
				eVarType contenttype;
				void* contentptr = execstack.Pop(contenttype);
                if(contenttype != TYPE_object) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr), "Error - ExecStack should contain TYPE_object\n");
                    return false;
                }

                // -- TYPE_object is actually just an uint32 ID
                uint32 objectid = *(uint32*)contentptr;

                // -- find the object
                CObjectEntry* oe = CNamespace::FindObject(objectid);
                if(!oe) {
                    ScriptAssert_(0, GetFileName(), CalcLineNumber(instrptr), "Error - Unable to find object %d\n", objectid);
                    return false;
                }

                // $$$TZA possible opportunity to ensure that if the current object on the function call stack
                // is this object, there are no further instructions referencing it...
                CNamespace::DestroyObject(objectid);

                break;
            }

			case OP_EOF:
				// -- we're done
				return true;

			default:
				// -- unhandle instruction
				printf("Unhandled instruction: 0x%08x: %s\n", curoperation,
					   GetOperationString(curoperation));
				assert(0);
				return false;
		};
	}

	// -- ran out of instructions, without a legitimate OP_EOF
	return false;
}


// ------------------------------------------------------------------------------------------------
// Debug helper functions
void SetDebugTrace(bool8 torf) {
    gDebugTrace = torf;
}

bool8 GetDebugParseTree() {
    return gDebugTrace;
}

REGISTER_FUNCTION_P1(SetDebugTrace, SetDebugTrace, void, bool8);

}  // TinScript

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
