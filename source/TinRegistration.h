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

#ifndef __TINREGISTRATION_H
#define __TINREGISTRATION_H

#include "TinNamespace.h"
#include "TinStringTable.h"
#include "TinInterface.h"

namespace TinScript {

class CVariableEntry;
class CFunctionEntry;
class CCodeBlock;

typedef CHashTable<CVariableEntry> tVarTable;
typedef CHashTable<CFunctionEntry> tFuncTable;

// ------------------------------------------------------------------------------------------------
// registration macros
// ------------------------------------------------------------------------------------------------
#include "registrationmacros.h"

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

	private:
        CScriptContext* mContextOwner;

		char mName[kMaxNameLength];
		uint32 mHash;
		eVarType mType;
		void* mAddr;
        uint32 mOffset;
        bool mIsDynamic;
		bool mScriptVar;
        int mStackOffset;
        CFunctionEntry* mFuncEntry;
};

// ------------------------------------------------------------------------------------------------
// Function Entry
class CFunctionContext {

    public:
        CFunctionContext(CScriptContext* script_context);
        virtual ~CFunctionContext();

        CScriptContext* GetScriptContext() {
            return mContextOwner;
        }

        bool AddParameter(const char* varname, uint32 varhash, eVarType type);
        bool AddParameter(const char* varname, uint32 varhash, eVarType type, int paramindex);
        CVariableEntry* AddLocalVar(const char* varname, uint32 varhash,
                                    eVarType type);
        int GetParameterCount();
        CVariableEntry* GetParameter(int index);
        CVariableEntry* GetLocalVar(uint32 varhash);
        tVarTable* GetLocalVarTable();
        bool IsParameter(CVariableEntry* ve);
        void InitStackVarOffsets();

    private:
        CScriptContext* mContextOwner;

        enum { eMaxParameterCount = 16, eMaxLocalVarCount = 37 };

        tVarTable* localvartable;

        // -- note:  the first parameter in the list is the return value
        // -- we're using an array to ensure the list stays ordered
        int paramcount;
        CVariableEntry* parameterlist[eMaxParameterCount];
};

class CRegFunctionBase {
    public:

        CRegFunctionBase(const char* _funcname = "") {
            funcname = _funcname;
            next = gRegistrationList;
            gRegistrationList = this;
        }

        virtual ~CRegFunctionBase() {
        }

        const char* GetName() {
            return funcname;
        }

        void SetContext(CFunctionContext* fe) {
            funccontext = fe;
        }

        CFunctionContext* GetContext() {
            return funccontext;
        }

        virtual void DispatchFunction(void* objaddr) {
        }

        virtual void Register(CScriptContext* script_context) = 0;
        CRegFunctionBase* GetNext() {
            return next;
        }

        static CRegFunctionBase* gRegistrationList;

    private:
        const char* funcname;
        CFunctionContext* funccontext;

        CRegFunctionBase* next;
};

// ------------------------------------------------------------------------------------------------
// Function Entry
class CFunctionEntry {
	public:
		CFunctionEntry(CScriptContext* script_context, uint32 _nshash, const char* _name,
                       uint32 _hash, EFunctionType _type, void* _addr);
		CFunctionEntry(CScriptContext* script_context, uint32 _nshash, const char* _name,
                       uint32 _hash, EFunctionType _type, CRegFunctionBase* _func);
		virtual ~CFunctionEntry();

        CScriptContext* GetScriptContext() {
            return (mContextOwner);
        }

		const char* GetName() const {
			return (mName);
		}

		EFunctionType GetType() const {
			return (mType);
		}

		uint32 GetNamespaceHash() const {
			return (mNamespaceHash);
		}

		uint32 GetHash() const {
			return (mHash);
		}

		void* GetAddr() const;

        void SetCodeBlockOffset(CCodeBlock* _codeblock, uint32 _offset);
        uint32 GetCodeBlockOffset(CCodeBlock*& _codeblock) const;
        CFunctionContext* GetContext();

        eVarType GetReturnType();
        tVarTable* GetLocalVarTable();
        CRegFunctionBase* GetRegObject();

	private:
        CScriptContext* mContextOwner;

		char mName[kMaxNameLength];
		uint32 mHash;
		EFunctionType mType;
        uint32 mNamespaceHash;

        void* mAddr;
		uint32 mInstrOffset;
        CCodeBlock* mCodeblock;

        CFunctionContext mContext;

        CRegFunctionBase* mRegObject;
};

// ------------------------------------------------------------------------------------------------
// registration classes
// ------------------------------------------------------------------------------------------------
#include "registrationclasses.h"
#include "TinInterface.h"

}  // TinScript

#endif // __TINREGISTRATION_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
