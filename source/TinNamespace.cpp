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
// tinnamespace.cpp
// ------------------------------------------------------------------------------------------------

// -- lib includes
#include "stdafx.h"
#include "stdio.h"

#include "TinNamespace.h"
#include "TinParse.h"
#include "TinScheduler.h"
#include "TinStringTable.h"
#include "TinRegistration.h"

namespace TinScript {

CHashTable<CNamespace>* CNamespace::gNamespaceDictionary;
CHashTable<CObjectEntry>* CNamespace::gObjectDictionary;
CHashTable<CObjectEntry>* CNamespace::gAddressDictionary;
CHashTable<CObjectEntry>* CNamespace::gNameDictionary;

CNamespaceReg* CNamespaceReg::head = NULL;

static const char* kGlobalNamespace = "_global";

// ------------------------------------------------------------------------------------------------
// CObjectEntry implementation

CObjectEntry::CObjectEntry(uint32 _objid, uint32 _namehash, CNamespace* _objnamespace,
    void* _objaddr) {
    objectid = _objid;
    namehash = _namehash;
    objectnamespace = _objnamespace;
    objectaddr = _objaddr;
    dynamicvariables = NULL;
}

CObjectEntry::~CObjectEntry() {
    if(dynamicvariables) {
        dynamicvariables->DestroyAll();
        delete dynamicvariables;
    }
}

// ------------------------------------------------------------------------------------------------
CVariableEntry* CObjectEntry::GetVariableEntry(uint32 varhash) {
    CVariableEntry* ve = NULL;
    CNamespace* objns = GetNamespace();
    while(objns && !ve) {
        ve = objns->GetVarTable()->FindItem(varhash);
        objns = objns->GetNext();
    }

    // -- if we weren't able to find the variable in the legitimate namespace hierarchy,
    // -- check the dynamic variables
    if(!ve && dynamicvariables)
        ve = dynamicvariables->FindItem(varhash);

    return ve;
}

CFunctionEntry* CObjectEntry::GetFunctionEntry(uint32 nshash, uint32 funchash) {
    CFunctionEntry* fe = NULL;
    CNamespace* objns = GetNamespace();
    while(!fe && objns) {
        if(nshash == 0 || objns->GetHash() == nshash)
            fe = objns->GetFuncTable()->FindItem(funchash);
        objns = objns->GetNext();
    }

    return fe;
}

nflag CObjectEntry::AddDynamicVariable(uint32 varhash, eVarType vartype) {
    // -- sanity check
    if(varhash == 0 || vartype < FIRST_VALID_TYPE)
        return false;

    // -- see if the variable already exists
    CVariableEntry* ve = GetVariableEntry(varhash);

    // -- if we do, it had better be the same type
    if(ve) {
        if(ve->GetType() != vartype) {
            ScriptAssert_(0, "<internal>", -1, "Error - Variable already exists: %s, type: %s\n",
                          UnHash(varhash), GetRegisteredTypeName(ve->GetType()));
            return false;
        }
        return true;
    }

    // -- ensure we have a dictionary to hold the dynamic tags
    if(!dynamicvariables) {
        dynamicvariables = new CHashTable<CVariableEntry>(kLocalVarTableSize);
    }
	ve = new CVariableEntry(UnHash(varhash), varhash, vartype, false, 0, true);
	dynamicvariables->AddItem(*ve, varhash);

    return (ve != NULL);
}

// ------------------------------------------------------------------------------------------------
// CNamespace implementation

void CNamespace::Initialize() {

    // -- allocate the dictinary to store creation functions
    gNamespaceDictionary = new CHashTable<CNamespace>(kGlobalFuncTableSize);

    // -- allocate the dictionary to store the address of all objects created from script.
    gObjectDictionary = new CHashTable<CObjectEntry>(kObjectTableSize);
    gAddressDictionary = new CHashTable<CObjectEntry>(kObjectTableSize);
    gNameDictionary = new CHashTable<CObjectEntry>(kObjectTableSize);

    // -- register the namespace - these are the namespaces
    // -- registered from code, so we need to populate the NamespaceDictionary,
    // -- and register the members/methods
    // -- note, because we register class derived from parent, we need to
    // -- iterate and ensure parents are always registered before children
    while(CNamespaceReg::head != NULL) {
        nflag abletoregister = false;
        CNamespaceReg* regptr = CNamespaceReg::head;
        CNamespaceReg** prevptr = &CNamespaceReg::head;
        while(regptr) {

            // -- see if this namespace is already registered
            if(regptr->GetClassNamespace() != NULL) {
                prevptr = &regptr->next;
                regptr = regptr->GetNext();
                continue;
            }

            // -- see if this namespace still requires its parent to be registered
            static const uint32 nullparenthash = Hash("VOID");
            CNamespace* parentnamespace = NULL;
            if(regptr->GetParentHash() != nullparenthash)
            {
                parentnamespace = gNamespaceDictionary->FindItem(regptr->GetParentHash());
                if(!parentnamespace) {
                    // -- skip this one, and wait until the parent is registered
                    prevptr = &regptr->next;
                    regptr = regptr->GetNext();
                    continue;
                }
            }

            // -- unhook this object from the linked list awaiting registration
            *prevptr = regptr->GetNext();

            // -- set the nflag to track that we're actually making progress
            abletoregister = true;

            // -- ensure the namespace doesn't already exist
            CNamespace* namespaceentry = gNamespaceDictionary->FindItem(regptr->GetHash());
            if(namespaceentry == NULL) {
                // -- create the namespace
                CNamespace* newnamespace = new CNamespace(regptr->GetName(), regptr->GetCreateFunction(),
                                                          regptr->GetDestroyFunction());

                // -- add the creation method to the hash dictionary
                gNamespaceDictionary->AddItem(*newnamespace, regptr->GetHash());

                // -- create the namespace - note, this actually sets the static
                // -- namespace member, defined in the DECLARE_SCRIPT_CLASS macro
                regptr->SetClassNamespace(newnamespace);

                // -- link this namespace to its parent
                if(parentnamespace) {
                    LinkNamespaces(newnamespace, parentnamespace);
                }

                // -- call the class registration method, to register members/methods
                regptr->RegisterNamespace();
            }
            else {
                ScriptAssert_(0, "<internal>", -1, "Error - Namespace already created: %s\n", UnHash(regptr->GetHash()));
                return;
            }

            prevptr = &regptr->next;
            regptr = regptr->GetNext();
        }

        // -- we'd better have registered at least one namespace, otherwise we're stuck
        if(CNamespaceReg::head != NULL && !abletoregister) {
            ScriptAssert_(0, "<internal>", -1, "Error - Unable to register Namespace: %s\n", UnHash(CNamespaceReg::head->GetHash()));
            return;
        }
    }
}

void CNamespace::Shutdown() {

    // -- delete the Namespace dictionary
    if(gNamespaceDictionary) {
        gNamespaceDictionary->DestroyAll();
        delete gNamespaceDictionary;
        gNamespaceDictionary = NULL;
    }

    // -- delete the Object dictionaries
    if(gObjectDictionary) {
        gObjectDictionary->DestroyAll();
        delete gObjectDictionary;
        gObjectDictionary = NULL;
    }

    // -- objects will have been destroyed above, so simply clear this hash table
    if(gAddressDictionary) {
        gAddressDictionary->RemoveAll();
        delete gAddressDictionary;
        gAddressDictionary = NULL;
    }
    if(gNameDictionary) {
        gNameDictionary->RemoveAll();
        delete gNameDictionary;
        gNameDictionary = NULL;
    }
}

CVariableEntry* CNamespace::GetVarEntry(uint32 varhash) {
    CNamespace* curnamespace = this;
    while(curnamespace) {
        CVariableEntry* ve = GetVarTable()->FindItem(varhash);
        if(ve)
            return ve;
        else
            curnamespace = curnamespace->GetNext();
    }

    // -- not found
    return NULL;
}

CNamespace* CNamespace::FindOrCreateNamespace(const char* _nsname, nflag create) {
    // $$$TZA if we didn't give a name, use a global namespace... ideally we should verify
    // -- ensure the name lives in the string table
    const char* nsname = _nsname && _nsname[0] ? CStringTable::AddString(_nsname)
                                               : CStringTable::AddString(kGlobalNamespace);
    uint32 nshash = Hash(nsname);
    CNamespace* namespaceentry = gNamespaceDictionary->FindItem(nshash);
    if(!namespaceentry && create) {
        namespaceentry = new CNamespace(nsname, NULL);

        // -- add the namespace to the dictionary
        gNamespaceDictionary->AddItem(*namespaceentry, nshash);
    }

    return namespaceentry;
}

CNamespace* CNamespace::FindNamespace(uint32 nshash) {
    // $$$TZA Any way to verify that a hash of '0' is 100% guaranteed to mean the global namespace?
    if(nshash == 0)
        nshash = Hash(kGlobalNamespace);
    CNamespace* namespaceentry = gNamespaceDictionary->FindItem(nshash);
    return (namespaceentry);
}

void CNamespace::LinkNamespaces(const char* childnsname, const char* parentnsname) {

    // sanity check
    if(!childnsname || !childnsname[0] || !parentnsname || !parentnsname[0])
        return;

    // -- ensure the child exists and the parent exists
    TinScript::CNamespace* childns = TinScript::CNamespace::FindOrCreateNamespace(childnsname,
                                                                                  true);
    TinScript::CNamespace* parentns = TinScript::CNamespace::FindOrCreateNamespace(parentnsname,
                                                                                   true);
    LinkNamespaces(childns, parentns);
}

void CNamespace::LinkNamespaces(CNamespace* childns, CNamespace* parentns) {
    // -- sanity check
    if(!childns || !parentns || parentns == childns)
        return;

    if(childns->next == NULL) {
        // -- verify the parent is not already in the hierarchy, or we'll have a circular list
        CNamespace* tempns = parentns;
        while(tempns) {
            if(tempns == childns) {
                ScriptAssert_(0, "<internal>", -1,
                    "Error - attempting to link namespace %s to %s, which is already its child\n",
                    UnHash(childns->GetHash()), UnHash(parentns->GetHash()));
                return;
            }
            tempns = tempns->GetNext();
        }

        // -- nothing found in the hierarchy - go ahead and link
        childns->next = parentns;
        return;
    }

    // -- child is already linked - see if the new parent is already in the hierarchy
    else {
        CNamespace* tempns = childns->next;
        while(tempns) {
            if(tempns == parentns)
                return;
            else
                tempns = tempns->GetNext();
        }

        // -- not found in the hierarchy - we can insert if it doesn't have any children
        // -- or if it's child is the same child as the child we're trying to link
        if(parentns->next == NULL || parentns->next == childns->next) {
            parentns->next = childns->next;
            childns->next = parentns;
            return;
        }
    }

    // -- not found in the hierarchy - assert
    ScriptAssert_(0, "<internal>", -1,
        "Error - attempting to link namespace %s to %s, already linked to %s\n",
        UnHash(childns->GetHash()), UnHash(parentns->GetHash()),
        UnHash(childns->GetNext()->GetHash()));
}

uint32 CNamespace::GetNextObjectID() {
    // -- every object created gets a unique ID, so we can find it in the object dictionary
    // -- providing a way to register code-instantiated objects
    static uint32 objectid = 0;
    return ++objectid;
}

uint32 CNamespace::CreateObject(uint32 classhash, uint32 objnamehash) {
    uint32 objectid = GetNextObjectID();

    // -- find the creation function
    CNamespace* namespaceentry = gNamespaceDictionary->FindItem(classhash);
    if(namespaceentry != NULL) {
        CNamespace::CreateInstance funcptr = namespaceentry->GetCreateInstance();
        if(funcptr == NULL) {
            ScriptAssert_(0, "<internal>", -1, "Error - Class is not registered: %s\n", UnHash(classhash));
            return 0;
        }

        // -- create the object
        void* newobj = (*funcptr)();

        // -- see if we can hook this object up to the namespace for it's object name
        CNamespace* objnamens = namespaceentry;
        if(objnamehash != 0) {
            objnamens = gNamespaceDictionary->FindItem(objnamehash);
            if(!objnamens) {
                objnamens = namespaceentry;
            }
            else {
                // -- link the namespaces
                LinkNamespaces(objnamens, namespaceentry);
            }
        }

        // -- need to verify that if we're using an objnamens (scripted), that the namespaceentry
        // -- is the highest level registered class
        if(objnamens != namespaceentry) {
            CNamespace* tempns = objnamens;
            while(tempns && tempns->GetCreateInstance() == NULL) {
                tempns = tempns->GetNext();
            }
            // -- if we run out of namespaces... how'd we create this object?
            if(!tempns) {
                ScriptAssert_(0, "<internal>", -1, "Error - Unable to verify hierarchy for namespace: %s\n",
                              UnHash(objnamens->GetHash()));
                // $$$TZA find a way to delete the newly created, but non-registered object
                //delete newobj;
                return 0;
            }
            else if(tempns != namespaceentry) {
                ScriptAssert_(0, "<internal>", -1,
                    "Error - Unable to create an instance of base class: %s, using object namespace: %s.  Use derived class: %s\n",
                    UnHash(classhash), UnHash(objnamehash), UnHash(tempns->GetHash()));
                // $$$TZA find a way to delete the newly created, but non-registered object
                //delete newobj;
                return 0;
            }
        }

        // -- add this object to the dictionary of all objects created from script
        CObjectEntry* newobjectentry = new CObjectEntry(objectid, objnamehash, objnamens, newobj);
        gObjectDictionary->AddItem(*newobjectentry, objectid);

        // -- add the object to the dictionary by address
        gAddressDictionary->AddItem(*newobjectentry, (uint32)newobj);

        // -- if the item is named, add it to the name dictionary
        // $$$TZA Note:  names are not guaranteed unique...  warn?
        if(objnamehash != Hash("")) {
            gNameDictionary->AddItem(*newobjectentry, objnamehash);
        }

        // -- see if the "OnCreate" has been defined - it's not required to
        CFunctionEntry* createfunc = newobjectentry->GetFunctionEntry(0, Hash("OnCreate"));
        if(createfunc) {
            // -- call the script "OnInit" for the object
            int32 dummy = 0;
            ObjExecF(objectid, dummy, "OnCreate();");
        }

        return objectid;
    }
    else
    {
        ScriptAssert_(0, "<internal>", -1, "Error - Class is not registered: %s\n", UnHash(classhash));
        return 0;
    }
}

uint32 CNamespace::RegisterObject(void* objaddr, const char* classname,
                                        const char* objectname) {

    // -- sanity check
    if(!objaddr || !classname || !classname[0])
        return 0;

    // -- ensure the classname is for an existing registered class
    uint32 nshash = Hash(classname);
    CNamespace* namespaceentry = FindNamespace(nshash);
    if(!namespaceentry) {
        ScriptAssert_(0, "<internal>", -1, "Error - Class is not registered: %s\n", classname);
        return 0;
    }

    uint32 objectid = GetNextObjectID();

        // -- see if we can hook this object up to the namespace for it's object name
    uint32 objnamehash = objectname ? Hash(objectname) : 0;
    CNamespace* objnamens = namespaceentry;
    if(objnamehash != 0) {
        objnamens = gNamespaceDictionary->FindItem(objnamehash);
        if(!objnamens)
            objnamens = namespaceentry;
        else {
            // -- link the namespaces
            LinkNamespaces(objnamens, namespaceentry);
        }
    }

    // -- add this object to the dictionary of all objects created from script
    CObjectEntry* newobjectentry = new CObjectEntry(objectid, objnamehash, objnamens, objaddr);
    gObjectDictionary->AddItem(*newobjectentry, objectid);

    // -- add the object to the dictionary by address
    gAddressDictionary->AddItem(*newobjectentry, (uint32)objaddr);

    // -- see if the "OnCreate" has been defined - it's not required to
    CFunctionEntry* createfunc = newobjectentry->GetFunctionEntry(0, Hash("OnCreate"));
    if(createfunc) {
        // -- call the script "OnInit" for the object
        int32 dummy = 0;
        ObjExecF(objectid, dummy, "OnCreate(%d);", 57);
    }

    return objectid;
}

void CNamespace::DestroyObject(uint32 objectid) {
    // -- find this object in the dictionary of all objects created from script
    CObjectEntry* oe = gObjectDictionary->FindItem(objectid);
    if(!oe) {
        ScriptAssert_(0, "<internal>", -1, "Error - Unable to find object: %d\n", objectid);
        return;
    }

    // -- get the namespace entry for the object
    CNamespace* namespaceentry = oe->GetNamespace();
    if(!namespaceentry) {
        ScriptAssert_(0, "<internal>", -1, "Error - Unable to find the namespace for object: %d\n",
                      objectid);
        return;
    }

    // -- get the Destroy function
    CNamespace::DestroyInstance destroyptr = namespaceentry->GetDestroyInstance();
    if(destroyptr == NULL) {
        ScriptAssert_(0, "<internal>", -1,
                      "Error - no Destroy() function registered for class: %s\n",
                      UnHash(namespaceentry->GetHash()));
        return;
    }

    // -- see if the "OnDestroy" has been defined - it's not required to
    CFunctionEntry* destroyfunc = oe->GetFunctionEntry(0, Hash("OnDestroy"));
    if(destroyfunc) {
        // -- call the script "OnInit" for the object
        int32 dummy = 0;
        ObjExecF(objectid, dummy, "OnDestroy();");
    }

    // -- get the address of the object
    void* objaddr = oe->GetAddr();
    if(!objaddr) {
        ScriptAssert_(0, "<internal>", -1, "Error - no address for object: %d\n", objectid);
        return;
    }

    // -- remove the object from the dictionary, and delete the entry
    gObjectDictionary->RemoveItem(objectid);
    gAddressDictionary->RemoveItem((uint32)objaddr);
    gNameDictionary->RemoveItem(oe->GetNameHash());

    // -- cancel all pending schedules related to this object
    CScheduler::CancelObject(objectid);

    // -- delete the object entry
    delete oe;

    // -- delete the actual object
    (*destroyptr)(objaddr);
}

CObjectEntry* CNamespace::FindObject(uint32 objectid) {
    CObjectEntry* oe = gObjectDictionary->FindItem(objectid);
    return oe;
}

CObjectEntry* CNamespace::FindObjectByAddress(void* addr) {
    CObjectEntry* oe = gAddressDictionary->FindItem((uint32)addr);
    return oe;
}

CObjectEntry* CNamespace::FindObjectByName(const char* objname) {
    if(!objname || !objname[0])
        return NULL;
    CObjectEntry* oe = gNameDictionary->FindItem(Hash(objname));
    return oe;
}

uint32 CNamespace::FindIDByAddress(void* addr) {
    CObjectEntry* oe = gAddressDictionary->FindItem((uint32)addr);
    return oe ? oe->GetID() : 0;
}

void* CNamespace::FindObjectAddr(uint32 objectid) {
    CObjectEntry* oe = gObjectDictionary->FindItem(objectid);
    return oe ? oe->GetAddr() : NULL;
}

void CNamespace::AddDynamicVariable(uint32 objectid, uint32 varhash,
                                    eVarType vartype) {
    CObjectEntry* oe = gObjectDictionary->FindItem(objectid);
    if(!oe) {
        ScriptAssert_(0, "<internal>", -1, "Error - Unable to find object: %d\n", objectid);
        return;
    }
    oe->AddDynamicVariable(varhash, vartype);
}

void CNamespace::AddDynamicVariable(uint32 objectid, const char* varname,
                                    const char* vartypename) {
    if(!varname || !vartypename) {
        ScriptAssert_(0, "<internal>", -1, "Error - AddDynamicVariable with no var name/type\n");
        return;
    }

    uint32 varhash = Hash(varname);
    eVarType vartype = GetRegisteredType(vartypename, strlen(vartypename));
    return AddDynamicVariable(objectid, varhash, vartype);
}

void CNamespace::ListObjects() {
    for(int32 i = 0; i < kObjectTableSize; ++i) {
        CObjectEntry* oe = gObjectDictionary->FindItemByBucket(i);
        while(oe) {
            printf("%d: %s\n", oe->GetID(), UnHash(oe->GetNamespace()->GetHash()));
            oe = gObjectDictionary->GetNextItemInBucket(i);
        }
    }
}

// ------------------------------------------------------------------------------------------------
CNamespace::CNamespace(const char* _name, CreateInstance _createinstance,
                       DestroyInstance _destroyinstance) {
    // -- ensure the name lives in the string table
    name = _name && _name[0] ? CStringTable::AddString(_name)
                             : CStringTable::AddString(kGlobalNamespace);
    hash = Hash(name);
    next = NULL;
    createfuncptr = _createinstance;
    destroyfuncptr = _destroyinstance;
    membertable = new tVarTable(kLocalVarTableSize);
    methodtable = new tFuncTable(kLocalFuncTableSize);
}

CNamespace::~CNamespace() {
    membertable->DestroyAll();
    delete membertable;
    methodtable->DestroyAll();
    delete methodtable;
}

};

// ------------------------------------------------------------------------------------------------
void ListVariables(uint32 objectid) {
    if(objectid > 0) {
        TinScript::CObjectEntry* oe = TinScript::CNamespace::FindObject(objectid);
        if(!oe) {
            ScriptAssert_(0, "<internal>", -1, "Error - Unable to find object: %d\n", objectid);
        }
        else {
            TinScript::DumpVarTable(oe);
        }
    }
    else {
        TinScript::DumpVarTable(NULL, TinScript::GetGlobalNamespace()->GetVarTable());
    }
}

void ListFunctions(uint32 objectid) {
    if(objectid > 0) {
        TinScript::CObjectEntry* oe = TinScript::CNamespace::FindObject(objectid);
        if(!oe) {
            ScriptAssert_(0, "<internal>", -1, "Error - Unable to find object: %d\n", objectid);
        }
        else {
            TinScript::DumpFuncTable(oe);
        }
    }
    else {
        TinScript::DumpFuncTable(TinScript::GetGlobalNamespace()->GetFuncTable());
    }
}

const char* GetObjectNamespace(uint32 objectid) {
    if(objectid > 0) {
        TinScript::CObjectEntry* oe = TinScript::CNamespace::FindObject(objectid);
        if(!oe) {
            return "";
        }
        else {
            return TinScript::UnHash(oe->GetNamespace()->GetHash());
        }
    }
    else {
        return "";
    }
}

REGISTER_FUNCTION_P1(ListVariables, ListVariables, void, uint32);
REGISTER_FUNCTION_P1(ListFunctions, ListFunctions, void, uint32);
REGISTER_FUNCTION_P1(GetObjectNamespace, GetObjectNamespace, const char*, uint32);

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
