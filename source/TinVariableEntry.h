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

    // -- this method is called by registered methods (dispatch templated implementation),
    // -- and as it is used to cross into cpp, it returns a const char* for strings
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
            return (void*)mContextOwner->GetStringTable()->FindString(mStringValueHash);
        else
            return valueaddr;
    }

    // -- this method is used only on the script side.  The address it returns must *never*
    // -- be written to - instead, the SetValue() method for variables is used.
    // -- for strings, this returns the address of the hash value found in the string dictionary
    void* GetAddr(void* objaddr) const
    {
        // -- find the value address
        void* valueaddr = NULL;

        // -- if we're providing an object address, this var is a member
        if(objaddr && !mIsDynamic)
            valueaddr = (void*)((char*)objaddr + mOffset);
        else
            valueaddr = mAddr;

        // -- if the value is not a string, then simply return it
        if (mType != TYPE_string)
            return (valueaddr);
        else
        {
            // -- if the variable is not a global registered const char*, then it
            // -- lives in the string table and is already a hash value
            if (mScriptVar)
                return (valueaddr);

            // -- and here's where things get sticky - global registered strings contain
            // -- const char* values, *however*, as they're global, they could be modified
            // -- in code... so we need to re-hash the string every time we access it
            else
            {
                const char* global_string_value = *(const char**)(valueaddr);
                mContextOwner->GetStringTable()->RefCountDecrement(mStringValueHash);
                mStringValueHash = Hash(global_string_value, -1, true);
                return (void*)(&mStringValueHash);
            }
        }
    }

    uint32 GetOffset() const {
        return mOffset;
    }

    void SetValue(void* objaddr, void* value);
    void SetValueAddr(void* objaddr, void* value);

	void SetBreakOnWrite(bool torf, int32 varWatchRequestID, int32 debugger_session)
	{
		mBreakOnWrite = torf;
		mWatchRequestID = varWatchRequestID;
		mDebuggerSession = debugger_session;
	}

    void NotifyWrite(CScriptContext* script_context)
    {
	    if (mBreakOnWrite)
	    {
		    int32 cur_debugger_session = 0;
		    bool is_debugger_connected = script_context->IsDebuggerConnected(cur_debugger_session);
		    if (is_debugger_connected && mDebuggerSession >= cur_debugger_session)
			    script_context->SetForceBreak(mWatchRequestID);
	    }
    }

    void SetFunctionEntry(CFunctionEntry* _funcentry) {
        mFuncEntry = _funcentry;
    }
    CFunctionEntry* GetFunctionEntry() {
        return mFuncEntry;
    }

    // -- this is used to pass return values from scheduled functions
    void ResolveValueType(eVarType new_type, void* value);

    // -- if true, and this is the parameter of a registered function,
    // -- then instead of passing a uint32 to code, we'll
    // -- look up the object, verify it exists, verify it's namespace type matches
    // -- and convert to a pointer directly
    void SetDispatchConvertFromObject(uint32 convert_to_type_id)
    {
        mDispatchConvertFromObject = convert_to_type_id;
    }
    uint32 GetDispatchConvertFromObject()
    {
        return (mDispatchConvertFromObject);
    }

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
    mutable uint32 mStringValueHash;
    uint32 mDispatchConvertFromObject;
    CFunctionEntry* mFuncEntry;

	// -- a debugger hook to break if the variable changes
	bool8 mBreakOnWrite;
	int32 mWatchRequestID;
	int32 mDebuggerSession;
};

// ------------------------------------------------------------------------------------------------
// A special case, where a registered parameter is an actual class pointer, not a uint32
// -- we'll look up the object, and if it exists, ensure it's namespace hierarchy
// -- contains the pointer type we're converting to... then we'll do the conversion
template <typename T>
T ConvertVariableForDispatch(CVariableEntry* ve)
{
    T return_value;
    uint32 conversion_type_id = ve->GetDispatchConvertFromObject();
    if (conversion_type_id != 0)
    {
        uint32 obj_id = convert_from_void_ptr<uint32>::Convert(ve->GetValueAddr(NULL));
        CObjectEntry* oe = GetContext()->FindObjectEntry(obj_id);
        if (oe)
        {
            // -- validate that the object is actually derived from the parameter expected
            bool ns_type_found = false;
            CNamespace* ns_entry = oe->GetNamespace();
            while (ns_entry)
            {
                if (ns_entry->GetTypeID() == conversion_type_id)
                {
                    ns_type_found = true;
                    break;
                }
                else
                {
                    ns_entry = ns_entry->GetNext();
                }
            }

            if (!ns_type_found)
            {
                ScriptAssert_(::TinScript::GetContext(), false, "<internal>", -1,
                              "Error - object %d cannot be passed - invalid type\n", oe->GetID());
            }

            return_value = convert_from_void_ptr<T>::Convert(oe->GetAddr());
            return (return_value);
        }

        // -- invalid or not found - return NULL
        return_value = convert_from_void_ptr<T>::Convert((void*)NULL);
    }
    else
    {
        return_value = convert_from_void_ptr<T>::Convert(ve->GetValueAddr(NULL));
    }

    // -- return the value
    return (return_value);
}

} // TinScript

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------

