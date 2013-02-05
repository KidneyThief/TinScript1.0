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
// variable table

CVariableEntry::CVariableEntry(const char* _name, eVarType _type, void* _addr) {
	SafeStrcpy(name, _name, kMaxNameLength);
	type = _type;
	hash = Hash(_name);
    offset = 0;
    addr = _addr;
    isdynamic = false;
    scriptvar = false;
    stackoffset = -1;
    funcentry = NULL;
}

CVariableEntry::CVariableEntry(const char* _name, unsigned int _hash, eVarType _type,
                               bool isoffset, unsigned int _offset, bool _isdynamic) {
	SafeStrcpy(name, _name, kMaxNameLength);
	type = _type;
	hash = _hash;
    offset = 0;
    isdynamic = _isdynamic;
    scriptvar = false;
    stackoffset = -1;
    funcentry = NULL;

    // -- hashtables are tables of variable entries...
    // -- they can only be created from script
    if(type == TYPE_hashtable) {
        scriptvar = true;
        addr = (void*)new tVarTable(kLocalVarTableSize);
    }
    else if(isoffset) {
        addr = NULL;
        offset = _offset;
    }

    // -- not an offset (e.g not a class member)
    // -- globals are constructed above, so this is a script var, requiring us to allocate
    else {
		scriptvar = true;
		addr = (void*)(new char[gRegisteredTypeSize[_type]]);
		memset(addr, 0, gRegisteredTypeSize[_type]);
    }
}

CVariableEntry::~CVariableEntry() {
	if(scriptvar) {
        if(type != TYPE_hashtable) {
		    delete addr;
        }
        // -- if this is a hashtable, need to destroy all of its entries
        else {
            tVarTable* ht = static_cast<tVarTable*>(addr);
            ht->DestroyAll();

            // -- now delete the hashtable itself
            delete ht;
        }
	}
}

void CVariableEntry::SetValue(void* objaddr, void* value) {
	assert(value);
	int size = gRegisteredTypeSize[type];

    // -- if we're providing an objaddr, this variable is actually a member
    void* varaddr = GetAddr(objaddr);
	memcpy(varaddr, value, size);
}

// -- added to accomodate converting StringTableEntry hash values back into
// -- const char*, before calling dispatch
void CVariableEntry::SetValueAddr(void* objaddr, void* value) {
    assert(value);
	int size = gRegisteredTypeSize[type];

    void* varaddr = GetAddr(objaddr);
    if(type == TYPE_string) {
        unsigned int hash = Hash((const char*)value);
        memcpy(varaddr, &hash, size);
    }
    else
        memcpy(varaddr, value, size);
}

// ------------------------------------------------------------------------------------------------
// CFunctionContext
CFunctionContext::CFunctionContext() {
    localvartable = new tVarTable(eMaxLocalVarCount);
    paramcount = 0;
    for(int i = 0; i < eMaxParameterCount; ++i) {
        parameterlist[i] = NULL;
    }
}

CFunctionContext::~CFunctionContext() {

    // -- delete all the variable entries
    int tablesize = localvartable->Size();
	for(int i = 0; i < tablesize; ++i) {
		CVariableEntry* ve = localvartable->FindItemByBucket(i);
		while (ve) {
			unsigned int hash = ve->GetHash();
			localvartable->RemoveItem(hash);
			delete ve;
			ve = localvartable->FindItemByBucket(i);
		}
	}

    // -- delete the actual table
    delete localvartable;
}

bool CFunctionContext::AddParameter(const char* varname, unsigned int varhash, eVarType type,
                                    int paramindex) {
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

    // -- bump the count if we need to
    if(paramindex >= paramcount)
        paramcount = paramindex + 1;
    parameterlist[paramindex] = ve;

    return true;
}

bool CFunctionContext::AddParameter(const char* varname, unsigned int varhash, eVarType type) {
    assert(varname != NULL);

    // -- adding automatically increments the paramcount if needed
    AddParameter(varname, varhash, type, paramcount);
    return true;
}

CVariableEntry* CFunctionContext::AddLocalVar(const char* varname, unsigned int varhash,
                                              eVarType type) {

    // -- ensure the variable doesn't already exist
    CVariableEntry* exists = localvartable->FindItem(varhash);
    if(exists != NULL) {
        printf("Error - variable already exists: %s\n", varname);
        return NULL;
    }

    // -- create the Variable entry
    CVariableEntry* ve = new CVariableEntry(varname, varhash, type, false, 0, false);
	unsigned int hash = ve->GetHash();
	localvartable->AddItem(*ve, hash);

    return ve;
}

int CFunctionContext::GetParameterCount() {
    return paramcount;
}

CVariableEntry* CFunctionContext::GetParameter(int index) {
    assert(index >= 0 && index < paramcount);
    return parameterlist[index];
}

CVariableEntry* CFunctionContext::GetLocalVar(unsigned int varhash) {
    return localvartable->FindItem(varhash);
}

tVarTable* CFunctionContext::GetLocalVarTable() {
    return localvartable;
}

bool CFunctionContext::IsParameter(CVariableEntry* ve) {
    if(!ve)
        return false;
    for(int i = 0; i < paramcount; ++i) {
        if(parameterlist[i]->GetHash() == ve->GetHash())
            return true;
    }

    return false;
}

// ------------------------------------------------------------------------------------------------
void CFunctionContext::InitStackVarOffsets() {
    int stackoffset = 0;

    // -- loop the parameters
    int paramcount = GetParameterCount();
    for(int i = 0; i < paramcount; ++i) {
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
		for(unsigned int i = 0; i < vartable->Size(); ++i) {
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
CFunctionEntry::CFunctionEntry(unsigned int _nshash, const char* _name, unsigned int _hash,
                               EFunctionType _type, void* _addr) {
	SafeStrcpy(name, _name, kMaxNameLength);
	type = _type;
	hash = _hash;
    namespacehash = _nshash;
	addr = _addr;
    codeblock = NULL;
    instroffset = 0;
    regobject = NULL;
}

CFunctionEntry::CFunctionEntry(unsigned int _nshash, const char* _name, unsigned int _hash,
                               EFunctionType _type, CRegFunctionBase* _func) {
	SafeStrcpy(name, _name, kMaxNameLength);
	type = _type;
	hash = _hash;
    namespacehash = _nshash;
    codeblock = NULL;
	instroffset = 0;
    regobject = _func;
}

CFunctionEntry::~CFunctionEntry() {
    // -- notify the codeblock that this entry no longer exists
    if(codeblock)
        codeblock->RemoveFunction(this);
}

void* CFunctionEntry::GetAddr() const {
    assert(type != eFuncTypeScript);
	return addr;
}

void CFunctionEntry::SetCodeBlockOffset(CCodeBlock* _codeblock, unsigned int _offset) {
    assert(type == eFuncTypeScript);

    // -- if we're switching codeblocks (recompiling...) change owners
    if(codeblock && codeblock != _codeblock) {
        codeblock->RemoveFunction(this);
    }
    codeblock = _codeblock;
    instroffset = _offset;
    if(codeblock)
        codeblock->AddFunction(this);
}

unsigned int CFunctionEntry::GetCodeBlockOffset(CCodeBlock*& _codeblock) const {
    assert(type == eFuncTypeScript);
    _codeblock = codeblock;
    return instroffset;
}

CFunctionContext* CFunctionEntry::GetContext() {
    return &context;
}

eVarType CFunctionEntry::GetReturnType() {
    // -- return value is always the first var entry in the array
    assert(context.GetParameterCount() > 0);
    return context.GetParameter(0)->GetType();
}

tVarTable* CFunctionEntry::GetLocalVarTable() {
    return context.GetLocalVarTable();
}

CRegFunctionBase* CFunctionEntry::GetRegObject() {
    return regobject;
}

}  // TinScript

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
