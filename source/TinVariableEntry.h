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
// TinInterface.h
// ------------------------------------------------------------------------------------------------

#pragma once

namespace TinScript {

// ------------------------------------------------------------------------------------------------
// variable table
class CVariableEntry {

public:
    CVariableEntry(CScriptContext* script_context, const char* _name = NULL,
                    eVarType _type = TYPE_NULL, void* _addr = NULL);
    CVariableEntry(CScriptContext* script_context, const char* _name, uint32 _hash,
                    eVarType _type, bool isoffset, uint32 _offset,
                    bool _isdynamic = false);

    virtual ~CVariableEntry();

    CScriptContext* GetScriptContext() {
        return (mContextOwner);
    }

    const char* GetName() const {
        return mName;
    }

    eVarType GetType() const {
        return mType;
    }

    uint32 GetHash() const {
        return mHash;
    }

    int GetStackOffset() const {
        return mStackOffset;
    }

    void SetStackOffset(int _stackoffset) {
        mStackOffset = _stackoffset;
    }

    // -- added to accomodate converting StringTableEntry hash values back into
    // -- const char*, before calling dispatch
    void* GetValueAddr(void* objaddr) const {
        void* valueaddr = NULL;
        // -- if we're providing an object address, this var is a member
        // -- if it's a dynamic var, it belongs to the object,
        // -- but lives in a local dyanmic hashtable
        if(objaddr && !mIsDynamic)
            valueaddr = (void*)((char*)objaddr + mOffset);
        else
            valueaddr = mAddr;
        if(mType == TYPE_string)
            return (void*)mContextOwner->GetStringTable()->FindString(*(uint32*)valueaddr);
        else
            return valueaddr;
    }

    void* GetAddr(void* objaddr) const {
        // -- if we're providing an object address, this var is a member
        if(objaddr && !mIsDynamic)
            return (void*)((char*)objaddr + mOffset);
        else
            return mAddr;
    }

    uint32 GetOffset() const {
        return mOffset;
    }

    void SetValue(void* objaddr, void* value);
    void SetValueAddr(void* objaddr, void* value);

    void SetFunctionEntry(CFunctionEntry* _funcentry) {
        mFuncEntry = _funcentry;
    }
    CFunctionEntry* GetFunctionEntry() {
        return mFuncEntry;
    }

    // -- this is used to pass return values from scheduled functions
    void ResolveValueType(eVarType new_type, void* value);

private:

    CScriptContext* mContextOwner;

    char mName[kMaxNameLength];
    uint32 mHash;
    eVarType mType;
    void* mAddr;
    uint32 mOffset;
    int32 mStackOffset;
    bool8 mIsDynamic;
    bool8 mScriptVar;
    CFunctionEntry* mFuncEntry;
};

} // TinScript

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------

