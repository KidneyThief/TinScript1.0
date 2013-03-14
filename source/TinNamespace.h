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
// tinnamespace.h
// ------------------------------------------------------------------------------------------------

#ifndef __TINNAMESPACE_H
#define __TINNAMESPACE_H

#include "TinHash.h"
#include "TinScript.h"

// ------------------------------------------------------------------------------------------------
// -- These macros are included by DECLARE_SCRIPT_CLASS and IMPLEMENT_SCRIPT_CLASS...
// -- not to be used independently (or publicly unless you know what you're doing!)
#define SCRIPT_DEFAULT_METHODS(classname)                                                           \
    static uint32 classname##GetObjectID(classname* obj);                                           \
    static const char* classname##GetObjectName(classname* obj);                                    \
    static uint32 classname##GetGroupID(classname* obj);                                            \
    static void classname##ListMembers(classname* obj);                                             \
    static void classname##ListMethods(classname* obj);                                             \

#define IMPLEMENT_DEFAULT_METHODS(classname)                                                        \
    static uint32 classname##GetObjectID(::TinScript::CScriptContext* script_context,               \
                                         classname* obj) {                                          \
        return script_context->FindIDByAddress((void*)obj);                                         \
    }                                                                                               \
    static ::TinScript::CRegContextMethodP0<classname, uint32>                                      \
        _reg_##classname##GetObjectID                                                               \
        ("GetObjectID", classname##GetObjectID);                                                    \
                                                                                                    \
    static const char* classname##GetObjectName(::TinScript::CScriptContext* script_context,        \
                                                classname* obj) {                                   \
        ::TinScript::CObjectEntry* oe =                                                             \
            script_context->FindObjectByAddress((void*)obj);                                        \
        return oe ? oe->GetName() : "";                                                             \
    }                                                                                               \
    static ::TinScript::CRegContextMethodP0<classname, const char*>                                 \
        _reg_##classname##GetObjectName                                                             \
        ("GetObjectName", classname##GetObjectName);                                                \
                                                                                                    \
    static uint32 classname##GetGroupID(::TinScript::CScriptContext* script_context,                \
                                         classname* obj) {                                          \
        ::TinScript::CObjectEntry* oe =                                                             \
            script_context->FindObjectByAddress((void*)obj);                                        \
        ::TinScript::CObjectEntry* group_oe =                                                       \
            oe ? script_context->FindObjectByAddress((void*)oe->GetObjectGroup()) : NULL;           \
        return (group_oe ? group_oe->GetID() : 0);                                                  \
    }                                                                                               \
    static ::TinScript::CRegContextMethodP0<classname, uint32>                                      \
        _reg_##classname##GetGroupID                                                                \
        ("GetGroupID", classname##GetGroupID);                                                      \
                                                                                                    \
    static void classname##ListMembers(::TinScript::CScriptContext* script_context, classname* obj) { \
        ::TinScript::CObjectEntry* oe =                                                             \
            script_context->FindObjectByAddress((void*)obj);                                        \
        ::TinScript::DumpVarTable(oe);                                                              \
    }                                                                                               \
    static TinScript::CRegContextMethodP0<classname, void>                                          \
        _reg_##classname##ListMembers                                                               \
        ("ListMembers", classname##ListMembers);                                                    \
                                                                                                    \
    static void classname##ListMethods(::TinScript::CScriptContext* script_context, classname* obj) { \
        ::TinScript::CObjectEntry* oe =                                                             \
            script_context->FindObjectByAddress((void*)obj);                                        \
        ::TinScript::DumpFuncTable(oe);                                                             \
    }                                                                                               \
    static ::TinScript::CRegContextMethodP0<classname, void>                                        \
        _reg_##classname##ListMethods                                                               \
        ("ListMethods", classname##ListMethods);                                                    \


namespace TinScript {

class CScriptContext;
class CVariableEntry;
class CFunctionEntry;
class CNamespace;
class CNamespaceReg;
class CObjectGroup;

typedef CHashTable<CVariableEntry> tVarTable;
typedef CHashTable<CFunctionEntry> tFuncTable;

// ------------------------------------------------------------------------------------------------
class CObjectEntry {
    public:
        CObjectEntry(CScriptContext* script_context, uint32 _objid, uint32 _namehash,
                     CNamespace* _objnamespace, void* _objaddr);
        virtual ~CObjectEntry();

        CScriptContext* GetScriptContext() {
            return (mContextOwner);
        }

        uint32 GetID() const {
            return mObjectID;
        }

        const char* GetName() const {
            return UnHash(mNameHash);
        }

        uint32 GetNameHash() const {
            return mNameHash;
        }

        CNamespace* GetNamespace() const {
            return mObjectNamespace;
        }

        void* GetAddr() const {
            return mObjectAddr;
        }

        CObjectGroup* GetObjectGroup() const {
            return (mGroupOwner);
        }
        void SetObjectGroup(CObjectGroup* group_owner) {
            mGroupOwner = group_owner;
        }

        CVariableEntry* GetVariableEntry(uint32 varhash);
        CFunctionEntry* GetFunctionEntry(uint32 nshash, uint32 funchash);

        bool AddDynamicVariable(uint32 varhash, eVarType vartype);

    private:
        CScriptContext* mContextOwner;

        uint32 mObjectID;
        uint32 mNameHash;
        CNamespace* mObjectNamespace;
        void* mObjectAddr;
        CObjectGroup* mGroupOwner;
        CHashTable<CVariableEntry>* mDynamicVariables;
};

// ------------------------------------------------------------------------------------------------
class CNamespace {
    public:
        typedef void* (*CreateInstance)();
        typedef void (*DestroyInstance)(void* addr);
        typedef void (*Register)(CScriptContext* script_context, CNamespace* reg);

        CNamespace(CScriptContext* script_context, const char* name,
                   CreateInstance _createinstance = NULL, DestroyInstance _destroyinstance = NULL);
        virtual ~CNamespace();

        CScriptContext* GetScriptContext() {
            return (mContextOwner);
        }

        const char* GetName() {
            return (mName);
        }

        uint32 GetHash() {
            return (mHash);
        }

        CNamespace* GetNext() const {
            return (mNext);
        }
        void SetNext(CNamespace* _next) {
            mNext = _next;
        }

        CreateInstance GetCreateInstance() const {
            return (mCreateFuncptr);
        }

        // -- it's possible that this is a script-derived namespace...
        // -- find the highest level child with a proper destructor
        DestroyInstance GetDestroyInstance() const {
            const CNamespace* ns = this;
            while(ns && ns->mDestroyFuncptr == NULL)
                ns = ns->mNext;
            if(ns)
                return (ns->mDestroyFuncptr);
            else
                return (NULL);
        }

        CVariableEntry* GetVarEntry(uint32 varhash);

        tVarTable* GetVarTable() {
            return (mMemberTable);
        }

        tFuncTable* GetFuncTable() {
            return (mMethodTable);
        }

    private:
        CNamespace() { }

        CScriptContext* mContextOwner;

        const char* mName;
        uint32 mHash;
        CNamespace* mNext;

        CreateInstance mCreateFuncptr;
        DestroyInstance mDestroyFuncptr;

        tVarTable* mMemberTable;
        tFuncTable* mMethodTable;
};

class CNamespaceReg {
    public:
        CNamespaceReg(const char* _name, const char* _parentname,
                      void* _createfuncptr, void* _destroyfuncptr,
                      void* _regfuncptr) {
            mName = _name;
            mHash = Hash(mName);
            mParentName = _parentname;
            mParentHash = Hash(mParentName);
            mCreateFuncptr = (CNamespace::CreateInstance)_createfuncptr;
            mDestroyFuncptr = (CNamespace::DestroyInstance)_destroyfuncptr;
            mRegFuncptr = (CNamespace::Register)_regfuncptr;

            next = head;
            head = this;
        }

        // -- head of the linked list so we can register (build hashtables) each namespace
        static CNamespaceReg* head;
        CNamespaceReg* next;

        const char* GetName() const {
            return mName;
        }

        const char* GetParentName() const {
            return mParentName;
        }

        uint32 GetHash() const {
            return mHash;
        }

        uint32 GetParentHash() const {
            return mParentHash;
        }

        void SetRegistered(bool8 torf) {
            mRegistered = torf;
        }

        bool8 GetRegistered() const {
            return (mRegistered);
        }

        CNamespaceReg* GetNext() const {
            return next;
        }

        CNamespace::CreateInstance GetCreateFunction() const {
            return mCreateFuncptr;
        }

        CNamespace::DestroyInstance GetDestroyFunction() const {
            return mDestroyFuncptr;
        }

        void RegisterNamespace(CScriptContext* script_context, CNamespace* _namespace) {
            mRegFuncptr(script_context, _namespace);
        }

    private:
        const char* mName;
        uint32 mHash;
        const char* mParentName;
        uint32 mParentHash;
        bool8 mRegistered;

        CNamespace::CreateInstance mCreateFuncptr;
        CNamespace::DestroyInstance mDestroyFuncptr;
        CNamespace::Register mRegFuncptr;
        CNamespace** mClassNamespace;

        CNamespaceReg() { }
};

};

#endif // __TINNAMESPACE_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
