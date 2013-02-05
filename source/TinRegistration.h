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
		CVariableEntry(const char* _name = NULL, eVarType _type = TYPE_NULL, void* _addr = NULL);
		CVariableEntry(const char* _name, unsigned int _hash, eVarType _type, bool isoffset,
                       unsigned int _offset, bool _isdynamic = false);

		virtual ~CVariableEntry();

		const char* GetName() const {
			return name;
		}

		eVarType GetType() const {
			return type;
		}

		unsigned int GetHash() const {
			return hash;
		}

        int GetStackOffset() const {
            return stackoffset;
        }

        void SetStackOffset(int _stackoffset) {
            stackoffset = _stackoffset;
        }

        // -- added to accomodate converting StringTableEntry hash values back into
        // -- const char*, before calling dispatch
		void* GetValueAddr(void* objaddr) const {
            void* valueaddr = NULL;
            // -- if we're providing an object address, this var is a member
            // -- if it's a dynamic var, it belongs to the object,
            // -- but lives in a local dyanmic hashtable
            if(objaddr && !isdynamic)
                valueaddr = (void*)((char*)objaddr + offset);
            else
			    valueaddr = addr;
            if(type == TYPE_string)
                return (void*)CStringTable::FindString(*(unsigned int*)valueaddr);
            else
                return valueaddr;
		}

		void* GetAddr(void* objaddr) const {
            // -- if we're providing an object address, this var is a member
            if(objaddr && !isdynamic)
                return (void*)((char*)objaddr + offset);
            else
			    return addr;
		}

        unsigned int GetOffset() const {
            return offset;
        }

		void SetValue(void* objaddr, void* value);
        void SetValueAddr(void* objaddr, void* value);

        void SetFunctionEntry(CFunctionEntry* _funcentry) {
            assert(funcentry == NULL || funcentry == _funcentry);
            funcentry = _funcentry;
        }
        CFunctionEntry* GetFunctionEntry() {
            return funcentry;
        }

	private:
		char name[kMaxNameLength];
		unsigned int hash;
		eVarType type;
		void* addr;
        unsigned int offset;
        bool isdynamic;
		bool scriptvar;
        int stackoffset;
        CFunctionEntry* funcentry;
};

// ------------------------------------------------------------------------------------------------
// Function Entry
class CFunctionContext {

    public:
        CFunctionContext();
        virtual ~CFunctionContext();

        bool AddParameter(const char* varname, unsigned int varhash, eVarType type);
        bool AddParameter(const char* varname, unsigned int varhash, eVarType type, int paramindex);
        CVariableEntry* AddLocalVar(const char* varname, unsigned int varhash,
                                    eVarType type);
        int GetParameterCount();
        CVariableEntry* GetParameter(int index);
        CVariableEntry* GetLocalVar(unsigned int varhash);
        tVarTable* GetLocalVarTable();
        bool IsParameter(CVariableEntry* ve);
        void InitStackVarOffsets();

    private:

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

        virtual void Register() = 0;
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
		CFunctionEntry(unsigned int _nshash, const char* _name, unsigned int _hash,
                       EFunctionType _type, void* _addr);
		CFunctionEntry(unsigned int _nshash, const char* _name, unsigned int _hash,
                       EFunctionType _type, CRegFunctionBase* _func);
		virtual ~CFunctionEntry();

		const char* GetName() const {
			return name;
		}

		EFunctionType GetType() const {
			return type;
		}

		unsigned int GetNamespaceHash() const {
			return namespacehash;
		}

		unsigned int GetHash() const {
			return hash;
		}

		void* GetAddr() const;

        void SetCodeBlockOffset(CCodeBlock* _codeblock, unsigned int _offset);
        unsigned int GetCodeBlockOffset(CCodeBlock*& _codeblock) const;
        CFunctionContext* GetContext();

        eVarType GetReturnType();
        tVarTable* GetLocalVarTable();
        CRegFunctionBase* GetRegObject();

	private:
		char name[kMaxNameLength];
		unsigned int hash;
		EFunctionType type;
        unsigned int namespacehash;

        void* addr;
		unsigned int instroffset;
        CCodeBlock* codeblock;

        CFunctionContext context;

        CRegFunctionBase* regobject;
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
