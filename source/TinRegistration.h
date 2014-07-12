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
// Function Entry
class CFunctionContext {

    public:
        CFunctionContext(CScriptContext* script_context);
        virtual ~CFunctionContext();

        CScriptContext* GetScriptContext() {
            return mContextOwner;
        }

        bool8 AddParameter(const char* varname, uint32 varhash, eVarType type, uint32 convert_type_from_object = 0);
        bool8 AddParameter(const char* varname, uint32 varhash, eVarType type, int32 paramindex, uint32 convert_type_from_object = 0);
        CVariableEntry* AddLocalVar(const char* varname, uint32 varhash,
                                    eVarType type);
        int32 GetParameterCount();
        CVariableEntry* GetParameter(int index);
        CVariableEntry* GetLocalVar(uint32 varhash);
        tVarTable* GetLocalVarTable();
        bool8 IsParameter(CVariableEntry* ve);
        void ClearParameters();
        void InitStackVarOffsets();

    private:
        CScriptContext* mContextOwner;

        enum { eMaxParameterCount = 16, eMaxLocalVarCount = 37 };

        tVarTable* localvartable;

        // -- note:  the first parameter in the list is the return value
        // -- we're using an array to ensure the list stays ordered
        int32 paramcount;
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

        void SetScriptContext(CScriptContext* script_context) {
            mScriptContext = script_context;
        }

        CScriptContext* GetScriptContext() {
            return (mScriptContext);
        }

        void SetContext(CFunctionContext* fe) {
            funccontext = fe;
        }

        CFunctionContext* GetContext() {
            return funccontext;
        }

        virtual void DispatchFunction(void*) {
        }

        virtual void Register(CScriptContext* script_context) = 0;
        CRegFunctionBase* GetNext() {
            return next;
        }

        static CRegFunctionBase* gRegistrationList;

    private:
        const char* funcname;
        CFunctionContext* funccontext;
        CScriptContext* mScriptContext;

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

        CCodeBlock* GetCodeBlock() const
        {
            return (mCodeblock);
        }

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
//#include "registrationexecs.h"

}  // TinScript

#endif // __TINREGISTRATION_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
