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

// -- lib includes
#include "stdafx.h"
#include "stdio.h"

#include "TinScript.h"
#include "TinCompile.h"
#include "TinRegistration.h"

namespace TinScript {

// ------------------------------------------------------------------------------------------------
// variable entry

CVariableEntry::CVariableEntry(CScriptContext* script_context, const char* _name, eVarType _type,
                               void* _addr) {
    mContextOwner = script_context;
	SafeStrcpy(mName, _name, kMaxNameLength);
	mType = _type;
	mHash = Hash(_name);
    mOffset = 0;
    mAddr = _addr;
    mIsDynamic = false;
    mScriptVar = false;
    mStringValueHash = 0;
    mStackOffset = -1;
    mDispatchConvertFromObject = 0;
    mFuncEntry = NULL;
	mBreakOnWrite = false;
    mWatchRequestID = 0;
}

CVariableEntry::CVariableEntry(CScriptContext* script_context, const char* _name, uint32 _hash,
                               eVarType _type, bool8 isoffset, uint32 _offset, bool8 _isdynamic) {
    mContextOwner = script_context;
	SafeStrcpy(mName, _name, kMaxNameLength);
	mType = _type;
	mHash = _hash;
    mOffset = 0;
    mIsDynamic = _isdynamic;
    mScriptVar = false;
    mStringValueHash = 0;
    mStackOffset = -1;
    mDispatchConvertFromObject = 0;
    mFuncEntry = NULL;
	mBreakOnWrite = false;
    mWatchRequestID = 0;

    // -- hashtables are tables of variable entries...
    // -- they can only be created from script
    if(mType == TYPE_hashtable) {
        mScriptVar = true;
        // -- setting allocation type as a VarTable, although this may be an exception:
        // -- since it's actually a script variable allocation...  it's size is not
        // -- consistent with the normal size of variable storage
        mAddr = (void*)TinAlloc(ALLOC_VarTable, tVarTable, kLocalVarTableSize);
    }
    else if(isoffset) {
        mAddr = NULL;
        mOffset = _offset;
    }

    // -- not an offset (e.g not a class member)
    // -- globals are constructed above, so this is a script var, requiring us to allocate
    else {
		mScriptVar = true;
		mAddr = (void*)TinAllocArray(ALLOC_VarStorage, char, gRegisteredTypeSize[_type]);
		memset(mAddr, 0, gRegisteredTypeSize[_type]);
    }
}

CVariableEntry::~CVariableEntry() {
    // -- if the value is a string, update the string table
    if (mType == TYPE_string)
    {
        // -- keep the string table up to date
        GetScriptContext()->GetStringTable()->RefCountDecrement(mStringValueHash);
    }

	if(mScriptVar) {
        if(mType != TYPE_hashtable) {
		    TinFreeArray((char*)mAddr);
        }
        // -- if this is a hashtable, need to destroy all of its entries
        else {
            tVarTable* ht = static_cast<tVarTable*>(mAddr);
            ht->DestroyAll();

            // -- now delete the hashtable itself
            TinFree(ht);
        }
	}
}

// -- if the value type is a TYPE_string, then the void* value contains a hash value
void CVariableEntry::SetValue(void* objaddr, void* value)
{
	assert(value);
	int32 size = gRegisteredTypeSize[mType];

    // -- if we're providing an objaddr, this variable is actually a member
    void* varaddr = GetAddr(objaddr);

    // -- if this variable is a string, we need to decrement the current value string table entry
    if (mType == TYPE_string)
    {
        // -- keep the string table up to date
        GetScriptContext()->GetStringTable()->RefCountDecrement(mStringValueHash);

        // -- script variables store strings as hash values, but registered code variables
        // -- of type const char* need to actually point to a valid stirng
        mStringValueHash = *(uint32*)value;
        if (mScriptVar)
            memcpy(varaddr, &mStringValueHash, size);
        else
        {
            const char* string_value = GetScriptContext()->GetStringTable()->FindString(mStringValueHash);

            // - the mAddr member of a registered global string, is the address of the actual const char*
            *(const char**)(mAddr) = string_value;
        }

        GetScriptContext()->GetStringTable()->RefCountIncrement(mStringValueHash);
    }
    else
    {
        // -- copy the new value
	    memcpy(varaddr, value, size);
    }

	// -- if we've been requested to break on write
    NotifyWrite(GetScriptContext());
}

// -- for this method, if the value type is a TYPE_string, then the void* value
// -- contains an actual const char* string...
void CVariableEntry::SetValueAddr(void* objaddr, void* value)
{
    assert(value);
	int32 size = gRegisteredTypeSize[mType];

    void* varaddr = GetAddr(objaddr);
    if(mType == TYPE_string)
    {
        // -- keep the string table up to date
        GetScriptContext()->GetStringTable()->RefCountDecrement(mStringValueHash);

        // -- hash the new value (which includes a ref count)
        mStringValueHash = Hash((const char*)value, -1, true);

        // -- script variables store strings as hash values, but registered const char*
        // -- code variables need to actually point to a valid stirng
        if (mScriptVar)
            memcpy(varaddr, &mStringValueHash, size);
        else
        {
            // - the mAddr member of a registered global string, is the address of the actual const char*
            *(const char**)(mAddr) = GetScriptContext()->GetStringTable()->FindString(mStringValueHash);
        }
    }
    else
        memcpy(varaddr, value, size);

	// -- if we've been requested to break on write (in a current session
    NotifyWrite(GetScriptContext());
}

// -- this is only used to copy the contents of an execstack, to return a value from a
// -- scheduled function
void CVariableEntry::ResolveValueType(eVarType new_type, void* value) {
    if(mType != TYPE__resolve || !value) {
        ScriptAssert_(GetScriptContext(), false, "<internal>", -1,
            "Error - trying to call ResolveValueType() on var: %s\n",
            UnHash(GetHash()));
        return;
    }

    mType = new_type;
    int32 size = gRegisteredTypeSize[mType];
    memcpy(mAddr, value, size);
}

// ------------------------------------------------------------------------------------------------
// CFunctionContext
CFunctionContext::CFunctionContext(CScriptContext* script_context) {
    mContextOwner = script_context;
    localvartable = TinAlloc(ALLOC_VarTable, tVarTable, eMaxLocalVarCount);
    paramcount = 0;
    for(int32 i = 0; i < eMaxParameterCount; ++i) {
        parameterlist[i] = NULL;
    }
}

CFunctionContext::~CFunctionContext() {

    // -- delete all the variable entries
    int32 tablesize = localvartable->Size();
	for(int32 i = 0; i < tablesize; ++i) {
		CVariableEntry* ve = localvartable->FindItemByBucket(i);
		while (ve) {
			uint32 hash = ve->GetHash();
			localvartable->RemoveItem(hash);
			TinFree(ve);
			ve = localvartable->FindItemByBucket(i);
		}
	}

    // -- delete the actual table
    TinFree(localvartable);
}

bool8 CFunctionContext::AddParameter(const char* varname, uint32 varhash, eVarType type,
                                    int32 paramindex, uint32 actual_type_id) {
    assert(varname != NULL);

    // add the entry to the parameter list as well
    assert(paramindex >= 0 && paramindex < eMaxParameterCount);
    if(paramindex >= eMaxParameterCount) {
        printf("Error - Max parameter count %d exceeded, parameter: %s\n",
                eMaxParameterCount, varname);
        return false;
    }
    if(parameterlist[paramindex] != NULL) {
        printf("Error - parameter %d has already been added\n", paramindex);
        return false;
    }

    // -- create the Variable entry
    CVariableEntry* ve = AddLocalVar(varname, varhash, type);
    if(! ve) {
        return false;
    }

    // -- parameters that are registered as TYPE_object, but are actually
    // -- pointers to registered classes, can be automatically
    // -- converted
    if (type == TYPE_object && actual_type_id != 0 && actual_type_id != GetTypeID<uint32>())
    {
        ve->SetDispatchConvertFromObject(actual_type_id);
    }

    // -- bump the count if we need to
    if(paramindex >= paramcount)
        paramcount = paramindex + 1;
    parameterlist[paramindex] = ve;

    return true;
}

bool8 CFunctionContext::AddParameter(const char* varname, uint32 varhash, eVarType type, uint32 actual_type_id) {
    assert(varname != NULL);

    // -- adding automatically increments the paramcount if needed
    AddParameter(varname, varhash, type, paramcount, actual_type_id);
    return true;
}

CVariableEntry* CFunctionContext::AddLocalVar(const char* varname, uint32 varhash,
                                              eVarType type) {

    // -- ensure the variable doesn't already exist
    CVariableEntry* exists = localvartable->FindItem(varhash);
    if(exists != NULL) {
        printf("Error - variable already exists: %s\n", varname);
        return NULL;
    }

    // -- create the Variable entry
    CVariableEntry* ve = TinAlloc(ALLOC_VarEntry, CVariableEntry, GetScriptContext(), varname,
                                                                  varhash, type, false, 0, false);
	uint32 hash = ve->GetHash();
	localvartable->AddItem(*ve, hash);

    return ve;
}

int32 CFunctionContext::GetParameterCount() {
    return paramcount;
}

CVariableEntry* CFunctionContext::GetParameter(int32 index) {
    assert(index >= 0 && index < paramcount);
    return parameterlist[index];
}

CVariableEntry* CFunctionContext::GetLocalVar(uint32 varhash) {
    return localvartable->FindItem(varhash);
}

tVarTable* CFunctionContext::GetLocalVarTable() {
    return localvartable;
}

bool8 CFunctionContext::IsParameter(CVariableEntry* ve) {
    if(!ve)
        return false;
    for(int32 i = 0; i < paramcount; ++i) {
        if(parameterlist[i]->GetHash() == ve->GetHash())
            return true;
    }

    return false;
}

void CFunctionContext::ClearParameters() {
    for(int32 i = 0; i < paramcount; ++i) {
        CVariableEntry* ve = parameterlist[i];
        const int32 max_size = MAX_TYPE_SIZE * (int32)sizeof(uint32);
        char buf[max_size];
        memset(buf, 0, max_size);
        ve->SetValue(NULL, (void*)&buf);
    }
}

// ------------------------------------------------------------------------------------------------
void CFunctionContext::InitStackVarOffsets() {
    int32 stackoffset = 0;

    // -- loop the parameters
    int32 paramcount = GetParameterCount();
    for(int32 i = 0; i < paramcount; ++i) {
        CVariableEntry* ve = GetParameter(i);
        assert(ve);
        // -- set the stackoffset
        if(ve->GetStackOffset() < 0)
            ve->SetStackOffset(stackoffset);
        ++stackoffset;
    }

    // -- now declare the rest of the local vars
    tVarTable* vartable = GetLocalVarTable();
    assert(vartable);
	if(vartable) {
		for(int32 i = 0; i < vartable->Size(); ++i) {
			CVariableEntry* ve = vartable->FindItemByBucket(i);
			while (ve) {
                if(!IsParameter(ve)) {
                    // -- set the stackoffset
                    if(ve->GetStackOffset() < 0)
                        ve->SetStackOffset(stackoffset);
                    ++stackoffset;
                }
				ve = vartable->GetNextItemInBucket(i);
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
// CFunctionEntry implementation
CFunctionEntry::CFunctionEntry(CScriptContext* script_context, uint32 _nshash, const char* _name,
                               uint32 _hash, EFunctionType _type, void* _addr) :
                               mContext(script_context) {
    mContextOwner = script_context;
	SafeStrcpy(mName, _name, kMaxNameLength);
	mType = _type;
	mHash = _hash;
    mNamespaceHash = _nshash;
	mAddr = _addr;
    mCodeblock = NULL;
    mInstrOffset = 0;
    mRegObject = NULL;
}

CFunctionEntry::CFunctionEntry(CScriptContext* script_context, uint32 _nshash, const char* _name,
                               uint32 _hash, EFunctionType _type, CRegFunctionBase* _func) :
                               mContext(script_context) {
    mContextOwner = script_context;
	SafeStrcpy(mName, _name, kMaxNameLength);
	mType = _type;
	mHash = _hash;
    mNamespaceHash = _nshash;
    mCodeblock = NULL;
	mInstrOffset = 0;
    mRegObject = _func;
}

CFunctionEntry::~CFunctionEntry() {
    // -- notify the codeblock that this entry no longer exists
    if(mCodeblock)
        mCodeblock->RemoveFunction(this);
}

void* CFunctionEntry::GetAddr() const {
    assert(mType != eFuncTypeScript);
	return mAddr;
}

void CFunctionEntry::SetCodeBlockOffset(CCodeBlock* _codeblock, uint32 _offset)
{
    // -- if we're switching codeblocks (recompiling...) change owners
    if(mCodeblock && mCodeblock != _codeblock) {
        mCodeblock->RemoveFunction(this);
    }
    mCodeblock = _codeblock;
    mInstrOffset = _offset;
    if(mCodeblock)
        mCodeblock->AddFunction(this);
}

uint32 CFunctionEntry::GetCodeBlockOffset(CCodeBlock*& _codeblock) const {
    assert(mType == eFuncTypeScript);
    _codeblock = mCodeblock;
    return mInstrOffset;
}

CFunctionContext* CFunctionEntry::GetContext() {
    return &mContext;
}

eVarType CFunctionEntry::GetReturnType() {
    // -- return value is always the first var entry in the array
    assert(mContext.GetParameterCount() > 0);
    return (mContext.GetParameter(0)->GetType());
}

tVarTable* CFunctionEntry::GetLocalVarTable() {
    return mContext.GetLocalVarTable();
}

CRegFunctionBase* CFunctionEntry::GetRegObject() {
    return mRegObject;
}

}  // TinScript

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
