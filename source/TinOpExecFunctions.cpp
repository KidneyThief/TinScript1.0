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
// TinOpExecFunctions.cpp : Implementation of the virtual machine
// ------------------------------------------------------------------------------------------------

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "TinScript.h"
#include "TinCompile.h"
#include "TinNamespace.h"
#include "TinScheduler.h"
#include "TinExecute.h"
#include "TinOpExecFunctions.h"

namespace TinScript {

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

void* GetStackVarAddr(CScriptContext* script_context, CExecStack& execstack,
                      CFunctionCallStack& funccallstack, int32 stackvaroffset) {
    int32 stacktop = 0;
    CObjectEntry* oe = NULL;
    CFunctionEntry* fe = funccallstack.GetExecuting(oe, stacktop);
    if(!fe || stackvaroffset < 0) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                      "Error - GetStackVarAddr() failed\n");
        return NULL;
    }

    void* varaddr = execstack.GetStackVarAddr(stacktop, stackvaroffset);
    return varaddr;
}

bool8 GetStackValue(CScriptContext* script_context, CExecStack& execstack,
                    CFunctionCallStack& funccallstack, void*& valaddr, eVarType& valtype,
                    CVariableEntry*& ve, CObjectEntry*& oe) {
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

		ve = GetVariable(script_context, script_context->GetGlobalNamespace()->GetVarTable(),
                         val1ns, val1func, val1hash, val1hashvar);
        if(!ve) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - Unable to find variable %d\n", UnHash(val1hash));
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
        oe = script_context->FindObjectEntry(varsource);
        if(!oe) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - Unable to find object %d\n", varsource);
            return false;
        }

        // -- find the variable entry from the object's namespace variable table
        ve = oe->GetVariableEntry(varhash);
        if(!ve)
            return (false);
		//assert(ve != NULL);
        valaddr = ve->GetAddr(oe->GetAddr());
		valtype = ve->GetType();
    }
    // -- if a stack variable was pushed...
    else if(valtype == TYPE__stackvar) {
        // -- we already know to do a stackvar lookup - replace the var with the actual value type
        valtype = (eVarType)((uint32*)valaddr)[0];

        int32 stackvaroffset = ((uint32*)valaddr)[1];
        valaddr = GetStackVarAddr(script_context, execstack, funccallstack, stackvaroffset);
        if(!valaddr) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - Unable to find stack var\n");
            return false;
        }
    }

    // -- if a POD member was pushed...
    else if (valtype == TYPE__podmember)
    {
        // -- the type and address of the variable/value has already been pushed
        valtype = (eVarType)((uint32*)valaddr)[0];
        valaddr = (void*)((uint32*)valaddr)[1];
    }

    // -- if the valtype wasn't either a var or a member, they remain unchanged
    return true;
}

bool8 GetBinOpValues(CScriptContext* script_context, CExecStack& execstack,
                     CFunctionCallStack& funccallstack,
                     void*& val0, eVarType& val0type,
					 void*& val1, eVarType& val1type) {

	// -- Note:  values come off the stack in reverse order
	// -- get the 2nd value
    CVariableEntry* ve1 = NULL;
    CObjectEntry* oe1 = NULL;
	val1 = execstack.Pop(val1type);
    if(!GetStackValue(script_context, execstack, funccallstack, val1, val1type, ve1, oe1))
        return false;

	// -- get the 1st value
    CVariableEntry* ve0 = NULL;
    CObjectEntry* oe0 = NULL;
	val0 = execstack.Pop(val0type);
    if(!GetStackValue(script_context, execstack, funccallstack, val0, val0type, ve0, oe0))
        return false;

	return true;
}

bool8 ObjectNumericalBinOp(CScriptContext* script_context, eOpCode op, eVarType val0type,
                           void* val0addr, eVarType val1type, void* val1addr, int32& result) {
    // -- both types must be TYPE_object
    if(val0type != TYPE_object || val1type != TYPE_object) {
        ScriptAssert_(script_context, 0, "<internal>", -1, "Error - BinOp of non-object types");
        return (false);
    }

    uint32 object_id_0 = *(uint32*)val0addr;
    uint32 object_id_1 = *(uint32*)val1addr;

    switch(op) {
        // -- comparisons are normally a subtraction, then compare the result to 0.0f
        case OP_CompareEqual:
        case OP_CompareNotEqual:
            result = object_id_0 == object_id_1 ? 0 : 1;
            return (true);

        // -- to have reached this point, we already know both objects exist
        case OP_BooleanAnd:
        {
            CObjectEntry* oe0 = script_context->FindObjectEntry(object_id_0);
            CObjectEntry* oe1 = script_context->FindObjectEntry(object_id_1);
            result = (oe0 != NULL && oe1 != NULL) ? 0 : 1;
            return (true);
        }
        case OP_BooleanOr:
        {
            CObjectEntry* oe0 = script_context->FindObjectEntry(object_id_0);
            CObjectEntry* oe1 = script_context->FindObjectEntry(object_id_1);
            result = (oe0 != NULL || oe1 != NULL) ? 0 : 1;
            return (true);
        }
        default:
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - unable to perform op: %s\n",
                          GetOperationString(op));
    }

    // -- unhandled
    return (false);
}

// ------------------------------------------------------------------------------------------------
bool8 IntegerNumericalBinOp(CScriptContext* script_context, eOpCode op, eVarType val0type,
                            void* val0, eVarType val1type, void* val1, int32& int_result) {
    void* val0addr = TypeConvert(val0type, val0, TYPE_int);
    void* val1addr = TypeConvert(val1type, val1, TYPE_int);
    int32 val0int = *(int32*)val0addr;
    int32 val1int = *(int32*)val1addr;

    // -- now perform the op
    switch(op) {
        case OP_Add:
            int_result = val0int + val1int;
            break;
        case OP_Sub:
            int_result = val0int - val1int;
            break;
        case OP_Mult:
            int_result = val0int * val1int;
            break;
        case OP_Div:
            ScriptAssert_(script_context, val1int != 0, "<internal>", -1,
                          "Error - Divide by 0\n");
            int_result = val0int / val1int;
            break;
        case OP_Mod:
        {
            ScriptAssert_(script_context, val1int != 0, "<internal>", -1,
                          "Error - Mod Divide by 0\n");
            val1int = val1int < 0 ? -val1int : val1int;
            while(val0int < 0)
                val0int += val1int;
            int_result = val0int % val1int;
            break;
        }

        case OP_CompareEqual:
        case OP_CompareNotEqual:
        case OP_CompareLess:
        case OP_CompareLessEqual:
        case OP_CompareGreater:
        case OP_CompareGreaterEqual:
            int_result = val0int - val1int;
            break;

        case OP_BooleanAnd:
            int_result = (val0int != 0 && val1int != 0) ? 1 : 0;
            break;

        case OP_BooleanOr:
            int_result = (val0int != 0 || val1int != 0) ? 1 : 0;
            break;

        default:
            return false;
    }

    // -- success 
    return true;
}

// ------------------------------------------------------------------------------------------------
bool8 PerformIntegerBinOp(CScriptContext* script_context, CExecStack& execstack,
                            CFunctionCallStack& funccallstack, eOpCode op, int32& int_result) {
    // -- when we're doing a comparison operation, or if none of the args are float32
	// -- Get both args from the stacks
	eVarType val0type;
	void* val0 = NULL;
	eVarType val1type;
	void* val1 = NULL;
	if(!GetBinOpValues(script_context, execstack, funccallstack, val0, val0type, val1, val1type)) {
		printf("Error - failed GetBinopValues() for operation: %s\n",
				GetOperationString(op));
		return false;
	}

    // -- initialize the results
	// -- there are only two valid types to compare, floats, and ints
	// -- any other form of "add" must be done through a function call
	// $$$TZA expand the TypeToString tables to include operations
    if((val0type != TYPE_int && val0type != TYPE_bool) ||
        (val1type != TYPE_int && val1type != TYPE_bool)) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - trying to compare non-integer types\n");
		return false;
    }

    return (IntegerNumericalBinOp(script_context, op, val0type, val0, val1type, val1, int_result));
}

// -- this is to consolidate all the math operations that pop two values from the stack
// -- and combine them... the operation is still responsible for handling pushing the result
bool8 PerformNumericalBinOp(CScriptContext* script_context, CExecStack& execstack,
                            CFunctionCallStack& funccallstack, eOpCode op, int32& int_result,
							float32& float_result) {

	// -- Get both args from the stacks
	eVarType val0type;
	void* val0 = NULL;
	eVarType val1type;
	void* val1 = NULL;
	if(!GetBinOpValues(script_context, execstack, funccallstack, val0, val0type, val1, val1type)) {
		printf("Error - failed GetBinopValues() for operation: %s\n",
				GetOperationString(op));
		return false;
	}

    // -- initialize the results
    int_result = 0x7fffffff;
    float_result = 1e8f;

    // -- if either argument is an object
    if(val0type == TYPE_object || val1type == TYPE_object) {
        bool8 result = ObjectNumericalBinOp(script_context, op, val0type, val0, val1type, val1, int_result);
        // -- copy the int_result into the float_result, for ops like boolean, comparison, etc...
        float_result = static_cast<float32>(int_result);
        return (result);
    }

    // -- if both args are int and/or bool, use integer operations
    if ((val0type == TYPE_int || val0type == TYPE_bool) &&
        (val1type == TYPE_int || val1type == TYPE_bool)) {
        bool8 result = IntegerNumericalBinOp(script_context, op, val0type, val0, val1type, val1,
                                             int_result);
        // -- copy the int_result into the float_result, for ops like boolean, comparison, etc...
        float_result = static_cast<float32>(int_result);
        return (result);
    }
 
	// -- there are only two valid types to compare, floats, and ints
	// -- any other form of "add" must be done through a function call
	// $$$TZA expand the TypeToString tables to include operations
    if((val0type != TYPE_int && val0type != TYPE_float && val0type != TYPE_bool) ||
        (val1type != TYPE_int && val1type != TYPE_float && val1type != TYPE_bool)) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - trying to compare non-numeric types\n");
		return false;
    }
    void* val0addr = TypeConvert(val0type, val0, TYPE_float);
    void* val1addr = TypeConvert(val1type, val1, TYPE_float);
    float32 val0float = *(float32*)val0addr;
    float32 val1float = *(float32*)val1addr;

    // -- now perform the op
    switch(op) {
        case OP_Add:
            float_result = val0float + val1float;
            break;
        case OP_Sub:
            float_result = val0float - val1float;
            break;
        case OP_Mult:
            float_result = val0float * val1float;
            break;
        case OP_Div:
            ScriptAssert_(script_context, val1float != 0.0f, "<internal>", -1,
                          "Error - Divide by 0\n");
            float_result = val0float / val1float;
            break;
        case OP_Mod:
        {
            ScriptAssert_(script_context, val1float != 0.0f, "<internal>", -1,
                          "Error - Mod Divide by 0\n");
            int32 val0int = (int32)val0float;
            int32 val1int = val1float < 0.0f ? -(int32)val1float : (int32)val1float;
            while(val0int < 0)
                val0int += val1int;
            float_result = (float32)(val0int % val1int);
            break;
        }

        case OP_CompareEqual:
        case OP_CompareNotEqual:
        case OP_CompareLess:
        case OP_CompareLessEqual:
        case OP_CompareGreater:
        case OP_CompareGreaterEqual:
            float_result = val0float - val1float;
            break;

        case OP_BooleanAnd:
            float_result = (val0float != 0.0f && val1float != 0.0f) ? 1.0f : 0.0f;
            break;

        case OP_BooleanOr:
            float_result = (val0float != 0.0f || val1float != 0.0f) ? 1.0f : 0.0f;
            break;

        default:
            return false;
    }

    // -- success 
    return true;
}

// ------------------------------------------------------------------------------------------------
// -- this is to consolidate all the math operations that pop two values from the stack
// -- and compbine them... the operation is still responsible for handling pushing the result
bool8 PerformIntegerBitwiseOp(CScriptContext* script_context, CExecStack& execstack,
                              CFunctionCallStack& funccallstack, eOpCode op, int32& result) {

	// -- Get both args from the stacks
	eVarType val0type;
	void* val0 = NULL;
	eVarType val1type;
	void* val1 = NULL;
	if(!GetBinOpValues(script_context, execstack, funccallstack, val0, val0type, val1, val1type)) {
		printf("Error - failed GetBinopValues() for operation: %s\n",
				GetOperationString(op));
		return false;
	}
 
	// -- there are only two valid types to compare, floats, andints
	// -- any other form of "add" must be done through a function call
	// $$$TZA expand the TypeToString tables to include operations
    if((val0type != TYPE_int && val0type != TYPE_float) ||
       (val1type != TYPE_int && val1type != TYPE_float)) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - trying to compare non-int32 types\n");
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

bool8 PerformAssignOp(CScriptContext* script_context, CExecStack& execstack,
                      CFunctionCallStack& funccallstack, eOpCode op) {

	// -- pop the value
    CVariableEntry* ve1 = NULL;
    CObjectEntry* oe1 = NULL;
	eVarType val1type;
	void* val1addr = execstack.Pop(val1type);
    if(!GetStackValue(script_context, execstack, funccallstack, val1addr, val1type, ve1, oe1)) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                      "Error - Failed to pop assignment value\n");
        return false;
    }

	// -- pop the (hash) name of the var
    CVariableEntry* ve0 = NULL;
    CObjectEntry* oe0 = NULL;
	eVarType varhashtype;
	void* var = execstack.Pop(varhashtype);
    bool8 is_stack_var = (varhashtype == TYPE__stackvar);
    bool8 is_pod_member = (varhashtype == TYPE__podmember);
    bool8 use_var_addr = (is_stack_var || is_pod_member);
    if(!GetStackValue(script_context, execstack, funccallstack, var, varhashtype, ve0, oe0)) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                      "Error - Failed to pop assignment variable\n");
        return (false);
    }

    // -- ensure we're assigning to a variable, an object member, or a local stack variable
    if(!ve0 && !use_var_addr) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                      "Error - Attempting to assign to a non-variable\n");
        return (false);
    }

    // -- if we're doing a straight up assignment, don't convert to float32
    if(op == OP_Assign )
    {
        // -- if we've been given the actual address of the var, copy directly to it
        if (use_var_addr) {
            val1addr = TypeConvert(val1type, val1addr, varhashtype);
            memcpy(var, val1addr, gRegisteredTypeSize[varhashtype]);
            DebugTrace(op, is_stack_var ? "StackVar: %s" : "PODMember: %s", DebugPrintVar(var, varhashtype));
        }

        // -- else set the value through the variable entry
        else
        {
            val1addr = TypeConvert(val1type, val1addr, ve0->GetType());
    	    ve0->SetValue(oe0 ? oe0->GetAddr() : NULL, val1addr);
            DebugTrace(op, "Var %s: %s", UnHash(ve0->GetHash()),
                       DebugPrintVar(val1addr, ve0->GetType()));
        }
        return true;
    }

    void* ve0addr = use_var_addr ? TypeConvert(varhashtype, var, TYPE_float)
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
            ScriptAssert_(script_context, val1float != 0.0f, "<internal>", -1,
                          "Error - Divide by 0\n");
            result = vefloat / val1float;
            break;
        case OP_AssignMod:
        {
            ScriptAssert_(script_context, val1float != 0.0f, "<internal>", -1,
                          "Error - Mod Divide by 0\n");
            int32 val0int = (int32)vefloat;
            int32 val1int = val1float < 0.0f ? -(int32)val1float : (int32)val1float;
            while(val0int < 0)
                val0int += val1int;
            result = (float32)(val0int % val1int);
            break;
        }

        default:
        {
            ScriptAssert_(script_context, false, "<internal>", -1,
                          "Error - Unhandled operation: %s\n", GetOperationString(op));
            return false;
        }
    }

    // -- convert back to our variable type
    if(use_var_addr) {
        void* convertptr = TypeConvert(TYPE_float, &result, varhashtype);
        if(!convertptr) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - Unable to convert from type %s to %s\n",
                          GetRegisteredTypeName(TYPE_float),
                          GetRegisteredTypeName(varhashtype));
            return false;
        }

        // -- if we've been given the actual address of the var, copy directly to it
        if (use_var_addr) {
            memcpy(var, convertptr, gRegisteredTypeSize[varhashtype]);
            DebugTrace(op, is_stack_var ? "StackVar: %s" : "PODMember: %s", DebugPrintVar(val1addr, varhashtype));
        }
    }
    else {
        void* convertptr = TypeConvert(TYPE_float, &result, ve0->GetType());
        if(!convertptr) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - Unable to convert from type %s to %s\n",
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

bool8 PerformBitAssignOp(CScriptContext* script_context, CExecStack& execstack,
                         CFunctionCallStack& funccallstack, eOpCode op) {
	// -- pop the value
    CVariableEntry* ve1 = NULL;
    CObjectEntry* oe1 = NULL;
	eVarType val1type;
	void* val1addr = execstack.Pop(val1type);
    if(!GetStackValue(script_context, execstack, funccallstack, val1addr, val1type, ve1, oe1)) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                      "Error - Failed to pop assignment value\n");
        return false;
    }

	// -- pop the (hash) name of the var
    CVariableEntry* ve0 = NULL;
    CObjectEntry* oe0 = NULL;
	eVarType varhashtype;
	void* var = execstack.Pop(varhashtype);
    bool8 is_stack_var = (varhashtype == TYPE__stackvar);
    bool8 is_pod_member = (varhashtype == TYPE__podmember);
    bool8 use_var_addr = (is_stack_var || is_pod_member);
    if(!GetStackValue(script_context, execstack, funccallstack, var, varhashtype, ve0, oe0)) {
        ScriptAssert_(script_context, false, "<internal>", -1,
                      "Error - Failed to pop assignment variable\n");
        return false;
    }

    // -- ensure we're assigning to a variable, an object member, or a local stack variable
    if(!ve0 && !use_var_addr) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                      "Error - Attempting to assign to a non-variable\n");
        return false;
    }

    void* ve0addr = use_var_addr ? TypeConvert(varhashtype, var, TYPE_int)
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

        default:
        {
            ScriptAssert_(script_context, false, "<internal>", -1,
                "Error - Unhandled operation: %s\n", GetOperationString(op));
            return false;
        }
    }

    // -- convert back to our variable type
    if(use_var_addr) {
        void* convertptr = TypeConvert(TYPE_int, &result, varhashtype);
        if(!convertptr) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - Unable to convert from type %s to %s\n",
                          GetRegisteredTypeName(TYPE_int),
                          GetRegisteredTypeName(varhashtype));
            return false;
        }

        memcpy(var, convertptr, gRegisteredTypeSize[varhashtype]);
        DebugTrace(op, is_stack_var ? "StackVar: %s" : "PODMember: %s", DebugPrintVar(var, varhashtype));
    }
    else {
        void* convertptr = TypeConvert(TYPE_int, &result, ve0->GetType());
        if(!convertptr) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - Unable to convert from type %s to %s\n",
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

bool8 PerformUnaryOp(CScriptContext* script_context, CExecStack& execstack,
                     CFunctionCallStack& funccallstack, eOpCode op) {
	// -- pop the value
    CVariableEntry* ve = NULL;
    CObjectEntry* oe = NULL;
	eVarType valtype;
	void* valaddr = execstack.Pop(valtype);
    if(!GetStackValue(script_context, execstack, funccallstack, valaddr, valtype, ve, oe)) {
        ScriptAssert_(script_context, 0, "<internal>", -1,
                      "Error - Failed to pop unary op value\n");
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
                ScriptAssert_(script_context, 0, "<internal>", -1,
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
                ScriptAssert_(script_context, 0, "<internal>", -1,
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
                ScriptAssert_(script_context, 0, "<internal>", -1,
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
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - unsupported (non unary?) op: %s\n",
                          GetOperationString(op));
            return false;
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Specific Operation Execution Functions begin here

bool8 OpExecNULL(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                 CFunctionCallStack& funccallstack) {
    ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                  "Error - OP_NULL is not a valid op, indicating an error in this codeblock: %s\n");
    return (false);
}

bool8 OpExecNOP(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                CFunctionCallStack& funccallstack) {
    DebugTrace(op, "");
    return (true);
}

bool8 OpExecVarDecl(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                    CFunctionCallStack& funccallstack) {
    uint32 varhash = *instrptr++;
    eVarType vartype = (eVarType)(*instrptr++);

    // -- if we're in the middle of a function definition, the var is local
    // -- otherwise it's global... there are no nested function definitions allowed
    int32 stacktop = 0;
    CObjectEntry* oe = NULL;
    AddVariable(cb->GetScriptContext(), cb->GetScriptContext()->GetGlobalNamespace()->GetVarTable(),
        funccallstack.GetTop(oe, stacktop), UnHash(varhash), varhash, vartype);
    DebugTrace(op, "Var: %s", UnHash(varhash));
    return (true);
}

bool8 OpExecParamDecl(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    uint32 varhash = *instrptr++;
    eVarType vartype = (eVarType)(*instrptr++);

    int32 stacktop = 0;
    CObjectEntry* oe = NULL;
    CFunctionEntry* fe = funccallstack.GetTop(oe, stacktop);
    assert(fe != NULL);
    fe->GetContext()->AddParameter(UnHash(varhash), varhash, vartype);
    DebugTrace(op, "Var: %s", UnHash(varhash));
    return (true);
}

// ------------------------------------------------------------------------------------------------
// -- Function used for all basic assign ops
bool8 OpExecAssignVal(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    if(!PerformAssignOp(cb->GetScriptContext(), execstack, funccallstack, op)) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
            "Error - unable to perform op: %s\n",
            GetOperationString(op));
        return false;
    }
    return (true);
}

bool8 OpExecAssign(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecAssignVal(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecAssignAdd(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                   CFunctionCallStack& funccallstack) {
    return (OpExecAssignVal(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecAssignSub(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                   CFunctionCallStack& funccallstack) {
    return (OpExecAssignVal(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecAssignMult(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                   CFunctionCallStack& funccallstack) {
    return (OpExecAssignVal(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecAssignDiv(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                   CFunctionCallStack& funccallstack) {
    return (OpExecAssignVal(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecAssignMod(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                   CFunctionCallStack& funccallstack) {
    return (OpExecAssignVal(cb, op, instrptr, execstack, funccallstack));
}

// ------------------------------------------------------------------------------------------------
// -- Function used for all bitwise assign ops
bool8 OpExecAssignBitwise(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    if(!PerformBitAssignOp(cb->GetScriptContext(), execstack, funccallstack, op)) {
            ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                "Error - unable to perform op: %s\n",
                GetOperationString(op));
            return false;
    }
    return (true);
}

bool8 OpExecAssignLeftShift(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecAssignBitwise(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecAssignRightShift(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecAssignBitwise(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecAssignBitAnd(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecAssignBitwise(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecAssignBitOr(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecAssignBitwise(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecAssignBitXor(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecAssignBitwise(cb, op, instrptr, execstack, funccallstack));
}

// ------------------------------------------------------------------------------------------------
// -- used for all unary ops
bool8 OpExecUnary(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                  CFunctionCallStack& funccallstack) {
    if(!PerformUnaryOp(cb->GetScriptContext(), execstack, funccallstack, op)) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - unable to perform op: %s\n", GetOperationString(op));
        return false;
    }
    return (true);
}

bool8 OpExecUnaryPreInc(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                        CFunctionCallStack& funccallstack) {
    return (OpExecUnary(cb, op, instrptr, execstack, funccallstack));
}
bool8 OpExecUnaryPreDec(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                        CFunctionCallStack& funccallstack) {
    return (OpExecUnary(cb, op, instrptr, execstack, funccallstack));
}
bool8 OpExecUnaryNeg(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                     CFunctionCallStack& funccallstack) {
    return (OpExecUnary(cb, op, instrptr, execstack, funccallstack));
}
bool8 OpExecUnaryPos(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                     CFunctionCallStack& funccallstack) {
    return (OpExecUnary(cb, op, instrptr, execstack, funccallstack));
}
bool8 OpExecUnaryBitInvert(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                           CFunctionCallStack& funccallstack) {
    return (OpExecUnary(cb, op, instrptr, execstack, funccallstack));
}
bool8 OpExecUnaryNot(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                     CFunctionCallStack& funccallstack) {
    return (OpExecUnary(cb, op, instrptr, execstack, funccallstack));
}

// ------------------------------------------------------------------------------------------------
bool8 OpExecPush(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                 CFunctionCallStack& funccallstack) {
    // -- next instruction is the type
    eVarType contenttype = (eVarType)(*instrptr);
    instrptr++;
    assert(contenttype >= 0 && contenttype < TYPE_COUNT);

    // -- push the the value onto the stack, and update the instrptr
    execstack.Push((void*)instrptr, contenttype);
    DebugTrace(op, "%s", DebugPrintVar((void*)instrptr, contenttype));

    // -- advance the instruction pointer
    int32 contentsize = kBytesToWordCount(gRegisteredTypeSize[contenttype]);
    instrptr += contentsize;
    return (true);
}

bool8 OpExecPushLocalVar(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
    // -- next instruction is the variable hash followed by the function context hash
    execstack.Push((void*)instrptr, TYPE__stackvar);
    DebugTrace(op, "StackVar [%s : %d]",
               GetRegisteredTypeName((eVarType)instrptr[0]), instrptr[1]);

    // -- advance the instruction pointer
    int32 contentsize = kBytesToWordCount(gRegisteredTypeSize[TYPE__stackvar]);
    instrptr += contentsize;
    return (true);
}

bool8 OpExecPushLocalValue(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
    // -- next instruction is the type
    eVarType valtype = (eVarType)(*instrptr++);

    // -- next instruction is the stack offset
    int32 stackoffset = (int32)*instrptr++;

    // -- get the stack top for this function call
    void* stackvaraddr = GetStackVarAddr(cb->GetScriptContext(), execstack, funccallstack,
                                         stackoffset);
    if(!stackvaraddr) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - Unable to get StackVarAddr()\n");
        return false;
    }

    execstack.Push(stackvaraddr, valtype);
    DebugTrace(op, "StackVar [%d]: %s", stackoffset, DebugPrintVar(stackvaraddr, valtype));
    return (true);
}

bool8 OpExecPushGlobalVar(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    // -- next instruction is the variable hash followed by the function context hash
    execstack.Push((void*)instrptr, TYPE__var);
    DebugTrace(op, "Var: %s", UnHash(instrptr[2]));

    // -- advance the instruction pointer
    int32 contentsize = kBytesToWordCount(gRegisteredTypeSize[TYPE__var]);
    instrptr += contentsize;
    return (true);
}

bool8 OpExecPushGlobalValue(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                        CFunctionCallStack& funccallstack) {
    // -- next instruction is the variable name
    uint32 nshash = *instrptr++;
    uint32 varfunchash = *instrptr++;
    uint32 varhash = *instrptr++;
    CVariableEntry* ve =
        GetVariable(cb->GetScriptContext(),
                    cb->GetScriptContext()->GetGlobalNamespace()->GetVarTable(), nshash,
                    varfunchash, varhash, 0);
    assert(ve != NULL);
    void* val = ve->GetAddr(NULL);
    eVarType valtype = ve->GetType();

    execstack.Push(val, valtype);
    DebugTrace(op, "Var: %s, %s", UnHash(varhash), DebugPrintVar(val, valtype));
    return (true);
}

bool8 OpExecPushArrayVar(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
    // -- hash value will have already been pushed
    eVarType contenttype;
    void* contentptr = execstack.Pop(contenttype);
    if(contenttype != TYPE_int) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
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
    return (true);
}

bool8 OpExecPushArrayValue(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                           CFunctionCallStack& funccallstack) {
    // -- hash value will have already been pushed
    eVarType contenttype;
    void* contentptr = execstack.Pop(contenttype);
    if(contenttype != TYPE_int) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - ExecStack should contain TYPE_int\n");
        return false;
    }
    uint32 arrayvarhash = *(uint32*)contentptr;

    // -- next instruction is the variable name
    uint32 nshash = *instrptr++;
    uint32 varfunchash = *instrptr++;
    uint32 varhash = *instrptr++;
    CVariableEntry* ve =
        GetVariable(cb->GetScriptContext(),
        cb->GetScriptContext()->GetGlobalNamespace()->GetVarTable(), nshash,
        varfunchash, varhash, arrayvarhash);
    if(!ve) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - OP_PushArrayValue failed\n");
        return false;
    }

    eVarType vetype = ve->GetType();
    void* veaddr = ve->GetAddr(NULL);

    execstack.Push(veaddr, vetype);
    return (true);
}

bool8 OpExecPushMember(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                       CFunctionCallStack& funccallstack) {
    // -- next instruction is the member name
    uint32 varhash = *instrptr++;

    // -- what will previously have been pushed on the stack, is the object ID
    eVarType contenttype;
    void* contentptr = execstack.Pop(contenttype);
    if(contenttype != TYPE_object) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
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
    return (true);
}

bool8 OpExecPushMemberVal(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                          CFunctionCallStack& funccallstack) {
    // -- next instruction is the member name
    uint32 varhash = *instrptr++;

    // -- what will previously have been pushed on the stack, is the object ID
    eVarType contenttype;
    void* contentptr = execstack.Pop(contenttype);
    if(contenttype != TYPE_object) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - ExecStack should contain TYPE_object\n");
        return false;
    }

    // -- TYPE_object is actually just an uint32 ID
    uint32 objectid = *(uint32*)contentptr;

    // -- find the object
    CObjectEntry* oe = cb->GetScriptContext()->FindObjectEntry(objectid);
    if(!oe) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - Unable to find object %d\n", objectid);
        return false;
    }

    // -- find the variable entry from the object's namespace variable table
    CVariableEntry* ve = oe->GetVariableEntry(varhash);
    if(!ve) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - Unable to find member %s for object %d\n",
                      UnHash(varhash), objectid);
        return false;
    }
    assert(ve != NULL);
    void* val = ve->GetAddr(oe->GetAddr());
    eVarType valtype = ve->GetType();

    // -- push the value of the member
    execstack.Push(val, valtype);
    return (true);
}

bool8 OpExecPushPODMember(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                          CFunctionCallStack& funccallstack) {
    // -- next instruction is the POD member name
    uint32 varhash = *instrptr++;

    // -- what will previously have been pushed on the stack, is a variable, member, or stack var
    // -- that is of a registered POD type
    CVariableEntry* ve0 = NULL;
    CObjectEntry* oe0 = NULL;
	eVarType vartype;
	void* varaddr = execstack.Pop(vartype);
    if(!GetStackValue(cb->GetScriptContext(), execstack, funccallstack, varaddr, vartype, ve0, oe0)) {
        ScriptAssert_(cb->GetScriptContext(), 0, "<internal>", -1,
                      "Error - Failed to pop assignment variable\n");
        return (false);
    }

    // -- the var and vartype will be set to the actual type and physical address of the
    // -- POD variable we're about to dereference
    eVarType pod_member_type;
    void* pod_member_addr = NULL;
    if (!GetRegisteredPODMember(vartype, varaddr, varhash, pod_member_type, pod_member_addr))
    {
        // -- push the value of the POD member
        execstack.Push(pod_member_addr, pod_member_type);
        return (true);
    }

    // -- the new type we're going to push is a TYPE__podmember
    // -- which is of the format:  TYPE__podmember vartype, varaddr
    uint32 varbuf[2];
    varbuf[0] = pod_member_type;
    varbuf[1] = (uint32)pod_member_addr;
    execstack.Push((void*)varbuf, TYPE__podmember);

    return (true);
}

bool8 OpExecPushPODMemberVal(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                          CFunctionCallStack& funccallstack) {
    // -- next instruction is the POD member name
    uint32 varhash = *instrptr++;

    // -- what will previously have been pushed on the stack, is a variable, member, or stack var
    // -- that is of a registered POD type
    eVarType contenttype;
    void* contentptr = execstack.Pop(contenttype);

    // -- see if we popped a value of a registered POD type
    eVarType pod_member_type;
    void* pod_member_addr = NULL;
    if (GetRegisteredPODMember(contenttype, contentptr, varhash, pod_member_type, pod_member_addr))
    {
        // -- push the value of the POD member
        execstack.Push(pod_member_addr, pod_member_type);
        return (true);
    }

    // -- push the value of the POD member
    execstack.Push(pod_member_addr, pod_member_type);
    return (true);
}

bool8 OpExecPushSelf(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                     CFunctionCallStack& funccallstack) {
    int32 stacktop = 0;
    CObjectEntry* oe = NULL;
    CFunctionEntry* fe = funccallstack.GetExecuting(oe, stacktop);
    if(!fe || !oe) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - PushSelf from outside a method\n");
        return false;
    }

    // -- need to push the variable
    uint32 objid = oe->GetID();
    execstack.Push((void*)&objid, TYPE_object);
    return (true);
}

bool8 OpExecPop(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                CFunctionCallStack& funccallstack) {
    eVarType contenttype;
    execstack.Pop(contenttype);
    return (true);
}

// ------------------------------------------------------------------------------------------------
// -- Function to execute basic math ops
bool8 OpExecNumericalOp(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                        CFunctionCallStack& funccallstack) {
    int32 int_result = 0x7fffffff;
    float32 float_result = 1e8f;
    if(!PerformNumericalBinOp(cb->GetScriptContext(), execstack, funccallstack,
                              op, int_result, float_result)) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - unable to perform op: %s\n",
                      GetOperationString(op));
        return false;
    }

    // -- if we have an integer result, push that, otherwise push a float result
    if(int_result != 0x7fffffff) {
        execstack.Push((void*)&int_result, TYPE_int);
        DebugTrace(op, "%d", int_result);
    }
    else {
        execstack.Push((void*)&float_result, TYPE_float);
        DebugTrace(op, "%.2f", float_result);
    }
    return (true);
}

bool8 OpExecAdd(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                CFunctionCallStack& funccallstack) {
    return (OpExecNumericalOp(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecSub(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                CFunctionCallStack& funccallstack) {
    return (OpExecNumericalOp(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecMult(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                CFunctionCallStack& funccallstack) {
    return (OpExecNumericalOp(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecDiv(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                CFunctionCallStack& funccallstack) {
    return (OpExecNumericalOp(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecMod(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                CFunctionCallStack& funccallstack) {
    return (OpExecNumericalOp(cb, op, instrptr, execstack, funccallstack));
}

// ------------------------------------------------------------------------------------------------
// -- Function to execute boolean ops
bool8 OpExecBooleanOp(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    int32 int_result = 0x7fffffff;
    float32 float_result = 1e8f;
    if(!PerformNumericalBinOp(cb->GetScriptContext(), execstack, funccallstack,
                              op, int_result, float_result)) {
        return false;
    }
    bool8 boolresult = (float_result != 0.0f);
    execstack.Push((void*)&boolresult, TYPE_bool);
    return (true);
}

bool8 OpExecBooleanAnd(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecBooleanOp(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecBooleanOr(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecBooleanOp(cb, op, instrptr, execstack, funccallstack));
}

// ------------------------------------------------------------------------------------------------
bool8 OpExecCompareEqual(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
    int32 int_result = 0x7fffffff;
    float32 float_result = 1e8f;
    if(!PerformNumericalBinOp(cb->GetScriptContext(), execstack, funccallstack,
                              op, int_result, float_result)) {
        return false;
    }
    bool8 boolresult = (float_result == 0.0f);
    execstack.Push((void*)&boolresult, TYPE_bool);
    DebugTrace(op, "%s", boolresult ? "true" : "false");
    return (true);
}

bool8 OpExecCompareNotEqual(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
    int32 int_result = 0x7fffffff;
    float32 float_result = 1e8f;
    if(!PerformNumericalBinOp(cb->GetScriptContext(), execstack, funccallstack,
                              op, int_result, float_result)) {
        return false;
    }
    bool8 boolresult = (float_result != 0.0f);
    execstack.Push((void*)&boolresult, TYPE_bool);
    DebugTrace(op, "%s", boolresult ? "true" : "false");
    return (true);
}

bool8 OpExecCompareLess(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
    int32 int_result = 0x7fffffff;
    float32 float_result = 1e8f;
    if(!PerformNumericalBinOp(cb->GetScriptContext(), execstack, funccallstack,
                              op, int_result, float_result)) {
        return false;
    }
    bool8 boolresult = (float_result < 0.0f);
    execstack.Push((void*)&boolresult, TYPE_bool);
    DebugTrace(op, "%s", boolresult ? "true" : "false");
    return (true);
}

bool8 OpExecCompareLessEqual(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                             CFunctionCallStack& funccallstack) {
    int32 int_result = 0x7fffffff;
    float32 float_result = 1e8f;
    if(!PerformNumericalBinOp(cb->GetScriptContext(), execstack, funccallstack,
                              op, int_result, float_result)) {
        return false;
    }
    bool8 boolresult = (float_result <= 0.0f);
    execstack.Push((void*)&boolresult, TYPE_bool);
    DebugTrace(op, "%s", boolresult ? "true" : "false");
    return (true);
}

bool8 OpExecCompareGreater(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
    int32 int_result = 0x7fffffff;
    float32 float_result = 1e8f;
    if(!PerformNumericalBinOp(cb->GetScriptContext(), execstack, funccallstack,
                              op, int_result, float_result)) {
        return false;
    }
    bool8 boolresult = (float_result > 0.0f);
    execstack.Push((void*)&boolresult, TYPE_bool);
    DebugTrace(op, "%s", boolresult ? "true" : "false");
    return (true);
}

bool8 OpExecCompareGreaterEqual(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                             CFunctionCallStack& funccallstack) {
    int32 int_result = 0x7fffffff;
    float32 float_result = 1e8f;
    if(!PerformNumericalBinOp(cb->GetScriptContext(), execstack, funccallstack,
                              op, int_result, float_result)) {
        return false;
    }
    bool8 boolresult = (float_result >= 0.0f);
    execstack.Push((void*)&boolresult, TYPE_bool);
    DebugTrace(op, "%s", boolresult ? "true" : "false");
    return (true);
}

// ------------------------------------------------------------------------------------------------
// -- function to execute bitwise ops
bool8 OpExecBitwiseOp(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    int32 result = 0;
    if(!PerformIntegerBitwiseOp(cb->GetScriptContext(), execstack, funccallstack, op, result)) {
            ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                "Error - Operation failed: %s\n", GetOperationString(op));
            return false;
    }
    execstack.Push((void*)&result, TYPE_int);
    DebugTrace(op, "%d", result);
    return (true);
}

bool8 OpExecBitLeftShift(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecBitwiseOp(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecBitRightShift(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecBitwiseOp(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecBitAnd(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecBitwiseOp(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecBitOr(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecBitwiseOp(cb, op, instrptr, execstack, funccallstack));
}

bool8 OpExecBitXor(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    return (OpExecBitwiseOp(cb, op, instrptr, execstack, funccallstack));
}

// ------------------------------------------------------------------------------------------------
bool8 OpExecBranch(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                   CFunctionCallStack& funccallstack) {
    int32 jumpcount = *instrptr++;
    instrptr += jumpcount;
    DebugTrace(op, "");
    return (true);
}

bool8 OpExecBranchTrue(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                       CFunctionCallStack& funccallstack) {
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
    DebugTrace(op, "%s", value ? "true" : "false");

    return (true);
}

bool8 OpExecBranchFalse(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                        CFunctionCallStack& funccallstack) {
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
    DebugTrace(op, "%s", !value ? "true" : "false");
    return (true);
}

bool8 OpExecFuncDecl(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                     CFunctionCallStack& funccallstack) {
    uint32 funchash = *instrptr++;
    uint32 namespacehash = *instrptr++;
    uint32 funcoffset = *instrptr++;
    CFunctionEntry* fe = FuncDeclaration(cb->GetScriptContext(), namespacehash, UnHash(funchash),
                                         funchash, eFuncTypeScript);
    if(!fe) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - failed to declare function - hash: 0x%08x\n", funchash);
        return false;
    }

    // -- this being a script function, set the offset, and add this function
    // -- to the list of functions this codeblock implements
    fe->SetCodeBlockOffset(cb, funcoffset);

    // -- push the function entry onto the call stack, so all var declarations
    // -- will be associated with this function
    funccallstack.Push(fe, NULL, execstack.GetStackTop());
    DebugTrace(op, "%s", UnHash(fe->GetHash()));
    return (true);
}

bool8 OpExecFuncDeclEnd(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                        CFunctionCallStack& funccallstack) {
    // -- push the function stack
    CObjectEntry* oe = NULL;
    CFunctionEntry* fe = funccallstack.Pop(oe);
    fe->GetContext()->InitStackVarOffsets();
    DebugTrace(op, "%s", UnHash(fe->GetHash()));
    return (true);
}

bool8 OpExecFuncCallArgs(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
    // -- we're about to call a new function - next will be however many assign ops
    // -- to set the parameter values, finally OP_FuncCall will actually execute

    // -- get the hash of the function name
    uint32 nshash = *instrptr++;
    uint32 funchash = *instrptr++;
    tFuncTable* functable = cb->GetScriptContext()->FindNamespace(nshash)->GetFuncTable();
    CFunctionEntry* fe = functable->FindItem(funchash);
    if(!fe) {
        if(nshash != 0) {
            ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                          "Error - undefined function: %s::%s()\n",
                          UnHash(nshash), UnHash(funchash));
        }
        else
        {
            ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                "Error - undefined function: %s()\n", UnHash(funchash));
        }
        return false;
    }

    // -- push the function entry onto the call stack
    // -- we're also going to zero out all parameters - by default, calling
    // -- a function without passing a parameter value is the same as that
    // -- passing 0
    fe->GetContext()->ClearParameters();

    funccallstack.Push(fe, NULL, execstack.GetStackTop());
    DebugTrace(op, "%s", UnHash(fe->GetHash()));

    // -- create space on the execstack, if this is a script function
    if(fe->GetType() != eFuncTypeGlobal) {
        int32 localvarcount = fe->GetLocalVarTable()->Used();
        execstack.Reserve(localvarcount * MAX_TYPE_SIZE);
    }

    return (true);
}

bool8 OpExecPushParam(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    // -- the next word is the parameter index for the current function we're calling
    uint32 paramindex = *instrptr++;

    // -- get the function about to be called
    int32 stackoffset = 0;
    CObjectEntry* oe = NULL;
    CFunctionEntry* fe = funccallstack.GetTop(oe, stackoffset);
    if(!fe) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - assigning parameters outside a function call\n");
        return false;
    }
    uint32 paramcount = fe->GetContext()->GetParameterCount();
    if(paramindex >= paramcount) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
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

    DebugTrace(op, "%s, param %d", UnHash(fe->GetHash()), paramindex);

    return (true);
}

bool8 OpExecMethodCallArgs(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                           CFunctionCallStack& funccallstack) {
    // -- get the hash of the namespace, in case we want a specific one
    uint32 nshash = *instrptr++;

    // -- get the hash of the method name
    uint32 methodhash = *instrptr++;

    // -- what will previously have been pushed on the stack, is the object ID
    eVarType contenttype;
    void* contentptr = execstack.Pop(contenttype);
    if(contenttype != TYPE_object) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - ExecStack should contain TYPE_object\n");
        return false;
    }

    // -- TYPE_object is actually just an uint32 ID
    uint32 objectid = *(uint32*)contentptr;

    // -- find the object
    CObjectEntry* oe = cb->GetScriptContext()->FindObjectEntry(objectid);
    if(!oe) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - Unable to find object %d\n", objectid);
        return false;
    }

    // -- find the method entry from the object's namespace hierarachy
    // -- if nshash is 0, then it's from the top of the hierarchy
    CFunctionEntry* fe = oe->GetFunctionEntry(nshash, methodhash);
    if(!fe) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - Unable to find method %s for object %d\n",
                      UnHash(methodhash), objectid);
        return false;
    }

    // -- push the function entry onto the call stack
    // -- we're also going to zero out all parameters - by default, calling
    // -- a function without passing a parameter value is the same as that
    // -- passing 0
    fe->GetContext()->ClearParameters();

    // -- push the function entry onto the call stack
    funccallstack.Push(fe, oe, execstack.GetStackTop());

    // -- create space on the execstack, if this is a script function
    if(fe->GetType() != eFuncTypeGlobal) {
        int32 localvarcount = fe->GetLocalVarTable()->Used();
        execstack.Reserve(localvarcount * MAX_TYPE_SIZE);
    }

    DebugTrace(op, "obj: %d, ns: %s, func: %s", objectid, UnHash(nshash),
               UnHash(fe->GetHash()));
    return (true);
}

// $$$TZA Why is the compiler complaining about not finding this, when it's defined in TinExecute.h
extern bool8 CodeBlockCallFunction(CFunctionEntry* fe, CObjectEntry* oe, CExecStack& execstack,
                            CFunctionCallStack& funccallstack);

bool8 OpExecFuncCall(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                     CFunctionCallStack& funccallstack) {
    int32 stackoffset = 0;
    CObjectEntry* oe = NULL;
    CFunctionEntry* fe = funccallstack.GetTop(oe, stackoffset);
    assert(fe != NULL);

    // -- notify the stack that we're now actually executing the top function
    // -- this is to ensure that stack variables now reference this function's
    // -- reserved space on the stack.
    funccallstack.BeginExecution(instrptr - 1);

    DebugTrace(op, "func: %s", UnHash(fe->GetHash()));

    bool8 result = CodeBlockCallFunction(fe, oe, execstack, funccallstack);
    if(!result) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - Unable to call function: %s()\n",
                      UnHash(fe->GetHash()));
        return false;
    }
    return (true);
}

bool8 OpExecFuncReturn(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                       CFunctionCallStack& funccallstack) {
    // -- pop the function entry from the stack
    CObjectEntry* oe = NULL;
    CFunctionEntry* fe = funccallstack.Pop(oe);
    if(!fe) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
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

    // -- clear all parameters for the function - this will ensure all
    // -- strings are decremented, keeping the string table clear of unassigned values
    fe->GetContext()->ClearParameters();

    DebugTrace(op, "func: %s, val: %s", UnHash(fe->GetHash()),
               DebugPrintVar(stacktopcontent, contenttype));

    // Note:  THIS ACTUALLY BREAKS THE EXECUTION LOOP
    return (true);
}

bool8 OpExecArrayHash(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                      CFunctionCallStack& funccallstack) {
    // -- pop the value of the next string to append to the hash
    CVariableEntry* ve1 = NULL;
    CObjectEntry* oe1 = NULL;
    eVarType val1type;
    void* val1 = execstack.Pop(val1type);
    if(!GetStackValue(cb->GetScriptContext(), execstack, funccallstack, val1, val1type, ve1, oe1)) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - Failed to pop string to hash\n");
        return false;
    }
    // -- ensure it actually is a string
    void* val1addr = TypeConvert(val1type, val1, TYPE_string);

    // -- get the current hash
    eVarType contenttype;
    void* contentptr = execstack.Pop(contenttype);
    if(contenttype != TYPE_int) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - ExecStack should contain TYPE_object\n");
        return false;
    }

    // -- calculate the updated hash
    uint32 hash = *(uint32*)contentptr;
    hash = HashAppend(hash, "_");
    hash = HashAppend(hash, (const char*)val1addr);

    // -- push the result
    execstack.Push((void*)&hash, TYPE_int);

    return (true);
}

bool8 OpExecArrayVarDecl(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
    // -- next instruction is the type
    eVarType vartype = (eVarType)(*instrptr++);

    // -- pull the hash value for the hash table entry
    eVarType contenttype;
    void* contentptr = execstack.Pop(contenttype);
    if(contenttype != TYPE_int) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - ExecStack should contain TYPE_int\n");
        return false;
    }
    uint32 hashvalue = *(uint32*)contentptr;

    // -- pull the hashtable variable off the stack
    CVariableEntry* ve0 = NULL;
    CObjectEntry* oe0 = NULL;
    eVarType val0type;
    void* val0 = execstack.Pop(val0type);
    if(!GetStackValue(cb->GetScriptContext(), execstack, funccallstack, val0, val0type, ve0, oe0)) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - ExecStack should contain a hashtable variable\n");
        return false;
    }
    if(val0type != TYPE_hashtable) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - ExecStack should contain hashtable variable\n");
        return false;
    }

    // -- now that we've got our hashtable var, and the type, create (or verify)
    // -- the hash table entry
    tVarTable* hashtable = (tVarTable*)ve0->GetAddr(oe0 ? oe0->GetAddr() : NULL);
    CVariableEntry* hte = hashtable->FindItem(hashvalue);
    // -- if the entry already exists, ensure it's the same type
    if(hte && hte->GetType() != vartype) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - HashTable variable: %s already has an entry of type: %s\n",
                      UnHash(ve0->GetHash()), GetRegisteredTypeName(hte->GetType()));
        return false;
    }
    // -- otherwise add the variable entry to the hash table
    else if(! hte) {
        CVariableEntry* hte = TinAlloc(ALLOC_VarEntry, CVariableEntry,
                                       cb->GetScriptContext(), UnHash(hashvalue),
                                       hashvalue, vartype, false, 0, false);
        hashtable->AddItem(*hte, hashvalue);
    }

    return (true);
}

bool8 OpExecSelfVarDecl(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                        CFunctionCallStack& funccallstack) {
    // -- next instruction is the variable hash
    uint32 varhash = *instrptr++;

    // -- followed by the type
    eVarType vartype = (eVarType)(*instrptr++);

    // -- get the object from the stack
    CObjectEntry* oe = NULL;
    CFunctionEntry* fe = funccallstack.GetTopMethod(oe);
    if(!fe || !oe) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - Unable to declare a self.var from outside a method\n");
        return false;
    }

    // -- add the dynamic variable
    cb->GetScriptContext()->AddDynamicVariable(oe->GetID(), varhash, vartype);
    return (true);
}

bool8 OpExecScheduleBegin(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                          CFunctionCallStack& funccallstack) {
    // -- ensure we're not in the middle of a schedule construction already
    if(cb->GetScriptContext()->GetScheduler()->mCurrentSchedule != NULL) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - A schedule() is already being processed\n");
        return false;
    }

    // -- read the next instruction - see if this is an immediate execution call
    uint32 immediate_execution = *instrptr++;

    // -- the function hash will have been pushed most recently
    eVarType contenttype;
    void* contentptr = execstack.Pop(contenttype);
    if(contenttype != TYPE_int) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - ExecStack should contain TYPE_int\n");
        return false;
    }
    uint32 funchash = *(uint32*)(contentptr);

    // -- next pull the object ID off the stack
    contenttype;
    contentptr = execstack.Pop(contenttype);
    if(contenttype != TYPE_object) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - ExecStack should contain TYPE_object\n");
        return false;
    }

    // -- TYPE_object is actually just an uint32 ID
    uint32 objectid = *(uint32*)contentptr;

    // -- finally pull the delay time off the stack
    contentptr = execstack.Pop(contenttype);
    if(contenttype != TYPE_int) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - ExecStack should contain TYPE_int\n");
        return false;
    }
    int32 delaytime = *(int32*)(contentptr);

    // -- create the schedule 
    cb->GetScriptContext()->GetScheduler()->mCurrentSchedule =
        cb->GetScriptContext()->GetScheduler()->ScheduleCreate(objectid, delaytime, funchash,
        immediate_execution != 0 ? true : false);

    return (true);
}

bool8 OpExecScheduleParam(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                          CFunctionCallStack& funccallstack) {
    // -- ensure we are in the middle of a schedule construction
    if(cb->GetScriptContext()->GetScheduler()->mCurrentSchedule == NULL) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
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
    cb->GetScriptContext()->GetScheduler()->mCurrentSchedule->mFuncContext->
        AddParameter(varnamebuf, Hash(varnamebuf), contenttype, paramindex);

    // -- assign the value
    CVariableEntry* ve = cb->GetScriptContext()->GetScheduler()->mCurrentSchedule->
                         mFuncContext->GetParameter(paramindex);
    ve->SetValue(NULL, contentptr);
    return (true);
}

bool8 OpExecScheduleEnd(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                        CFunctionCallStack& funccallstack) {
    // -- ensure we are in the middle of a schedule construction
    if(cb->GetScriptContext()->GetScheduler()->mCurrentSchedule == NULL) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - There is no schedule() being processed\n");
        return false;
    }

    // -- now that the schedule has been completely constructed, we need to determine
    // -- if it's scheduled for immediate execution
    CScheduler::CCommand* curcommand = cb->GetScriptContext()->GetScheduler()->mCurrentSchedule;
    if(curcommand->mImmediateExec) {
        ExecuteScheduledFunction(cb->GetScriptContext(), curcommand->mObjectID, curcommand->mFuncHash,
                                 curcommand->mFuncContext);

        // -- see if we have a return result
        CVariableEntry* return_ve = curcommand->mFuncContext->GetParameter(0);
        if(!return_ve) {
            ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                          "Error - There is no return value available from schedule()\n");
            return false;
        }

        // -- Push the contents of the return_ve onto *this* execstack
        execstack.Push(return_ve->GetAddr(NULL), return_ve->GetType());

        // -- now remove the current command from the scheduler
        cb->GetScriptContext()->GetScheduler()->CancelRequest(curcommand->mReqID);
    }

    // -- not immediate execution - therefore, push the schedule request ID instead
    else {
        int32 reqid =  cb->GetScriptContext()->GetScheduler()->mCurrentSchedule->mReqID;
        execstack.Push(&reqid, TYPE_int);
    }

    // -- clear the current schedule
    cb->GetScriptContext()->GetScheduler()->mCurrentSchedule = NULL;
    return (true);
}

bool8 OpExecCreateObject(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                         CFunctionCallStack& funccallstack) {
    uint32 classhash = *instrptr++;
    uint32 objnamehash = *instrptr++;
    uint32 objid = cb->GetScriptContext()->CreateObject(classhash, objnamehash);

    // -- if we failed to create the object, assert
    if (objid == 0)
    {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - Failed to create object of class:  %s\n", UnHash(classhash));
        return false;
    }

    // -- push the objid onto the stack, and update the instrptr
    execstack.Push(&objid, TYPE_object);
    return (true);
}

bool8 OpExecDestroyObject(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                          CFunctionCallStack& funccallstack) {
    // -- what will previously have been pushed on the stack, is the object ID
    eVarType contenttype;
    void* contentptr = execstack.Pop(contenttype);
    if(contenttype != TYPE_object) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - ExecStack should contain TYPE_object\n");
        return false;
    }

    // -- TYPE_object is actually just an uint32 ID
    uint32 objectid = *(uint32*)contentptr;

    // -- find the object
    CObjectEntry* oe = cb->GetScriptContext()->FindObjectEntry(objectid);
    if(!oe) {
        ScriptAssert_(cb->GetScriptContext(), 0, cb->GetFileName(), cb->CalcLineNumber(instrptr),
                      "Error - Unable to find object %d\n", objectid);
        return false;
    }

    // $$$TZA possible opportunity to ensure that if the current object on the function call stack
    // is this object, there are no further instructions referencing it...
    cb->GetScriptContext()->DestroyObject(objectid);
    return (true);
}

bool8 OpExecEOF(CCodeBlock* cb, eOpCode op, const uint32*& instrptr, CExecStack& execstack,
                CFunctionCallStack& funccallstack) {
    // $$$TZA EOF - END THE CURRENT EXECUTION LOOP
    return (true);
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
