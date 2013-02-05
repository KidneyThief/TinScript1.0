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
#define SCRIPT_DEFAULT_METHODS(classname)                                                    \
    static unsigned int classname##GetObjectID(classname* obj);                              \
    static const char* classname##GetObjectName(classname* obj);                             \
    static void classname##ListMembers(classname* obj);                                      \
    static void classname##ListMethods(classname* obj);

#define IMPLEMENT_DEFAULT_METHODS(classname)                                                 \
    static unsigned int classname##GetObjectID(classname* obj) {                             \
        return TinScript::CNamespace::FindIDByAddress((void*)obj);                           \
    }                                                                                        \
    static TinScript::CRegMethodP0<classname, unsigned int>                                  \
        _reg_##classname##GetObjectID                                                        \
        ("GetObjectID", classname##GetObjectID);                                             \
                                                                                             \
    static const char* classname##GetObjectName(classname* obj) {                            \
        TinScript::CObjectEntry* oe =                                                        \
            TinScript::CNamespace::FindObjectByAddress((void*)obj);                          \
        return oe ? oe->GetName() : "";                                                      \
    }                                                                                        \
    static TinScript::CRegMethodP0<classname, const char*>                                   \
        _reg_##classname##GetObjectName                                                      \
        ("GetObjectName", classname##GetObjectName);                                         \
                                                                                             \
    static void classname##ListMembers(classname* obj) {                                     \
        TinScript::CObjectEntry* oe =                                                        \
            TinScript::CNamespace::FindObjectByAddress((void*)obj);                          \
        TinScript::DumpVarTable(oe);                                                         \
    }                                                                                        \
    static TinScript::CRegMethodP0<classname, void>                                          \
        _reg_##classname##ListMembers                                                        \
        ("ListMembers", classname##ListMembers);                                             \
                                                                                             \
    static void classname##ListMethods(classname* obj) {                                     \
        TinScript::CObjectEntry* oe =                                                        \
            TinScript::CNamespace::FindObjectByAddress((void*)obj);                          \
        TinScript::DumpFuncTable(oe);                                                        \
    }                                                                                        \
    static TinScript::CRegMethodP0<classname, void>                                          \
        _reg_##classname##ListMethods                                                        \
        ("ListMethods", classname##ListMethods);                                             \


namespace TinScript {

class CVariableEntry;
class CFunctionEntry;
class CNamespace;
class CNamespaceReg;

typedef CHashTable<CVariableEntry> tVarTable;
typedef CHashTable<CFunctionEntry> tFuncTable;

// ------------------------------------------------------------------------------------------------
class CObjectEntry {
    public:
        CObjectEntry(unsigned int _objid, unsigned int _namehash, CNamespace* _objnamespace,
                     void* _objaddr);
        virtual ~CObjectEntry();

        unsigned int GetID() const {
            return objectid;
        }

        const char* GetName() const {
            return UnHash(namehash);
        }

        unsigned int GetNameHash() const {
            return namehash;
        }

        CNamespace* GetNamespace() const {
            return objectnamespace;
        }

        void* GetAddr() const {
            return objectaddr;
        }

        CVariableEntry* GetVariableEntry(unsigned int varhash);
        CFunctionEntry* GetFunctionEntry(unsigned int nshash, unsigned int funchash);

        bool AddDynamicVariable(unsigned int varhash, eVarType vartype);

    private:

        unsigned int objectid;
        unsigned int namehash;
        CNamespace* objectnamespace;
        void* objectaddr;
        CHashTable<CVariableEntry>* dynamicvariables;
};

// ------------------------------------------------------------------------------------------------
class CNamespace {
    public:
        typedef void* (*CreateInstance)();
        typedef void (*DestroyInstance)(void* addr);
        typedef void (*Register)(CNamespace* reg);

        CNamespace(const char* name, CreateInstance _createinstance = NULL,
                   DestroyInstance _destroyinstance = NULL);
        virtual ~CNamespace();

        const char* GetName() {
            return name;
        }

        unsigned int GetHash() {
            return hash;
        }

        CNamespace* GetNext() const {
            return next;
        }

        CreateInstance GetCreateInstance() const {
            return createfuncptr;
        }

        // -- it's possible that this is a script-derived namespace...
        // -- find the highest level child with a proper destructor
        DestroyInstance GetDestroyInstance() const {
            const CNamespace* ns = this;
            while(ns && ns->destroyfuncptr == NULL)
                ns = ns->next;
            if(ns)
                return ns->destroyfuncptr;
            else
                return NULL;
        }

        CVariableEntry* GetVarEntry(unsigned int varhash);

        tVarTable* GetVarTable() {
            return membertable;
        }

        tFuncTable* GetFuncTable() {
            return methodtable;
        }

        static void Initialize();
        static void Shutdown();

        static CNamespace* FindOrCreateNamespace(const char* _nsname, bool create);
        static CNamespace* FindNamespace(unsigned int nshash);
        static void LinkNamespaces(const char* parentnsname, const char* childnsname);
        static void LinkNamespaces(CNamespace* parentns, CNamespace* childns);

        static unsigned int GetNextObjectID();
        static unsigned int CreateObject(unsigned int classhash, unsigned int objnamehash);
        static unsigned int RegisterObject(void* objaddr, const char* classname,
                                           const char* objectname);
        static void DestroyObject(unsigned int objectid);

        static CObjectEntry* FindObjectByAddress(void* addr);
        static CObjectEntry* FindObjectByName(const char* objname);
        static CObjectEntry* FindObject(unsigned int objectid);
        static unsigned int FindIDByAddress(void* addr);

        static void* FindObjectAddr(unsigned int objectid);

        static void AddDynamicVariable(unsigned int objectid, unsigned int varhash,
                                       eVarType vartype);
        static void AddDynamicVariable(unsigned int objectid, const char* varname,
                                       const char* vartypename);
        static void ListObjects();

    private:
        CNamespace() { }

        const char* name;
        unsigned int hash;
        CNamespace* next;

        CreateInstance createfuncptr;
        DestroyInstance destroyfuncptr;

        tVarTable* membertable;
        tFuncTable* methodtable;

        // -- need to keep a list of all the current codeblocks, hashed by filename
        static CHashTable<CNamespace>* gNamespaceDictionary;

        // -- an array of all objects that were created from script
        static CHashTable<CObjectEntry>* gObjectDictionary;
        static CHashTable<CObjectEntry>* gAddressDictionary;
        static CHashTable<CObjectEntry>* gNameDictionary;
};

class CNamespaceReg {
    public:
        CNamespaceReg(const char* _name, const char* _parentname, void* _createfuncptr,
                      void* _destroyfuncptr, void* _regfuncptr, CNamespace** _classnamespace) {
            name = _name;
            hash = Hash(name);
            parentname = _parentname;
            parenthash = Hash(parentname);
            createfuncptr = (CNamespace::CreateInstance)_createfuncptr;
            destroyfuncptr = (CNamespace::DestroyInstance)_destroyfuncptr;
            regfuncptr = (CNamespace::Register)_regfuncptr;
            classnamespace = _classnamespace;

            next = head;
            head = this;
        }

        // -- head of the linked list so we can register (build hashtables) each namespace
        static CNamespaceReg* head;
        CNamespaceReg* next;

        const char* GetName() const {
            return name;
        }

        const char* GetParentName() const {
            return parentname;
        }

        unsigned int GetHash() const {
            return hash;
        }

        unsigned int GetParentHash() const {
            return parenthash;
        }

        CNamespaceReg* GetNext() const {
            return next;
        }

        CNamespace* GetClassNamespace() const {
            return *classnamespace;
        }

        CNamespace::CreateInstance GetCreateFunction() const {
            return createfuncptr;
        }

        CNamespace::DestroyInstance GetDestroyFunction() const {
            return destroyfuncptr;
        }

        void SetClassNamespace(CNamespace* _namespace) {
            *classnamespace = _namespace;
        }

        void RegisterNamespace() {
            regfuncptr(*classnamespace);
        }

    private:
        const char* name;
        unsigned int hash;
        const char* parentname;
        unsigned int parenthash;

        CNamespace::CreateInstance createfuncptr;
        CNamespace::DestroyInstance destroyfuncptr;
        CNamespace::Register regfuncptr;
        CNamespace** classnamespace;

        CNamespaceReg() { }
};

};

#endif // __TINNAMESPACE_H