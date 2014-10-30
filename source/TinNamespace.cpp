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

// ====================================================================================================================
// TinNamespace.cpp
// ====================================================================================================================

// -- lib includes
#include "stdafx.h"
#include "stdio.h"

// -- includes
#include "TinNamespace.h"
#include "TinParse.h"
#include "TinScheduler.h"
#include "TinObjectGroup.h"
#include "TinStringTable.h"
#include "TinRegistration.h"

// == namespace TinScript =============================================================================================

namespace TinScript
{

// --------------------------------------------------------------------------------------------------------------------
// -- statics
CNamespaceReg* CNamespaceReg::head = NULL;
static const char* kGlobalNamespace = "_global";

// == class CObjectEntry ==============================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CObjectEntry::CObjectEntry(CScriptContext* script_context, uint32 _objid, uint32 _namehash,
                           CNamespace* _objnamespace, void* _objaddr, bool8 register_manual)
{
    mContextOwner = script_context;
    mObjectID = _objid;
    mNameHash = _namehash;
    mObjectNamespace = _objnamespace;
    mObjectAddr = _objaddr;
    mDynamicVariables = NULL;
    mGroupOwner = NULL;
    mManualRegister = register_manual;
}

// ====================================================================================================================
// Destructor
// ====================================================================================================================
CObjectEntry::~CObjectEntry()
{
    if (mDynamicVariables)
    {
        mDynamicVariables->DestroyAll();
        TinFree(mDynamicVariables);
    }
}

// ====================================================================================================================
// GetVariableEntry():  Search the linked list of namespaces, and the dynamically added list for a registered variable.
// ====================================================================================================================
CVariableEntry* CObjectEntry::GetVariableEntry(uint32 varhash)
{
    CVariableEntry* ve = NULL;
    CNamespace* objns = GetNamespace();
    while (objns && !ve) {
        ve = objns->GetVarTable()->FindItem(varhash);
        objns = objns->GetNext();
    }

    // -- if we weren't able to find the variable in the legitimate namespace hierarchy,
    // -- check the dynamic variables
    if (!ve && mDynamicVariables)
        ve = mDynamicVariables->FindItem(varhash);

    return (ve);
}

// ====================================================================================================================
// GetFunctionEntry():  Search the linked list of namespaces looking for a registered method.
// ====================================================================================================================
CFunctionEntry* CObjectEntry::GetFunctionEntry(uint32 nshash, uint32 funchash)
{
    CFunctionEntry* fe = NULL;
    CNamespace* objns = GetNamespace();
    while (!fe && objns) {
        if (nshash == 0 || objns->GetHash() == nshash)
            fe = objns->GetFuncTable()->FindItem(funchash);
        objns = objns->GetNext();
    }

    return fe;
}

// ====================================================================================================================
// HasNamespace():  Search the linked list to see if this object is "derived" from a given namespace.
// ====================================================================================================================
CNamespace* CObjectEntry::HasNamespace(uint32 nshash)
{
    if (nshash == 0)
        return (NULL);
    CNamespace* objns = GetNamespace();
    while (objns && objns->GetHash() != nshash)
        objns = objns->GetNext();
    return objns;
}

// ====================================================================================================================
// AddDynamicVariable():  Add a dynamic variable to this object.
// ====================================================================================================================
bool8 CObjectEntry::AddDynamicVariable(uint32 varhash, eVarType vartype, int32 array_size)
{
    // -- sanity check
    if (varhash == 0 || vartype < FIRST_VALID_TYPE)
        return (false);

    // -- see if the variable already exists
    CVariableEntry* ve = GetVariableEntry(varhash);

    // -- if we do, it had better be the same type
    if (ve)
    {
        if (ve->GetType() != vartype)
        {
            ScriptAssert_(GetScriptContext(), 0, "<internal>", -1,
                          "Error - Variable already exists: %s, type: %s\n",
                          UnHash(varhash), GetRegisteredTypeName(ve->GetType()));
            return (false);
        }
        return (true);
    }

    // -- ensure we have a dictionary to hold the dynamic tags
    if (!mDynamicVariables)
    {
        mDynamicVariables = TinAlloc(ALLOC_HashTable, CHashTable<CVariableEntry>,
                                    kLocalVarTableSize);
    }

	ve = TinAlloc(ALLOC_VarEntry, CVariableEntry, GetScriptContext(), UnHash(varhash), varhash,
                  vartype, array_size, false, 0, true);
	mDynamicVariables->AddItem(*ve, varhash);

    return (ve != NULL);
}

// ====================================================================================================================
// SetMemberVar():  Set the value of an object member.
// ====================================================================================================================
bool8 CObjectEntry::SetMemberVar(uint32 varhash, void* value)
{
    if (!value)
        return (false);

    // -- find the variable
    CVariableEntry* ve = GetVariableEntry(varhash);
    if (!ve)
    {
        ScriptAssert_(GetScriptContext(), 0, "<internal>", -1,
                      "Error - Unable to find variable %s for object %d\n",
                      UnHash(varhash), GetID());
        return (false);
    }

    // -- set the value
    ve->SetValue(GetAddr(), value);
    return (true);
}

// ====================================================================================================================
// FindOrCreateNamespace():  Find a namespace by name, create if required.
// ====================================================================================================================
CNamespace* CScriptContext::FindOrCreateNamespace(const char* _nsname, bool8 create)
{
    // -- ensure the name lives in the string table
    const char* nsname = _nsname && _nsname[0] ? GetStringTable()->AddString(_nsname)
                                               : GetStringTable()->AddString(kGlobalNamespace);
    uint32 nshash = Hash(nsname);
    CNamespace* namespaceentry = mNamespaceDictionary->FindItem(nshash);
    if (!namespaceentry && create) {
        namespaceentry = TinAlloc(ALLOC_Namespace, CNamespace, this, nsname, 0, NULL);

        // -- add the namespace to the dictionary
        mNamespaceDictionary->AddItem(*namespaceentry, nshash);
    }

    return namespaceentry;
}

// ====================================================================================================================
// FindNamespace():  Find a namespace by hash.
// ====================================================================================================================
CNamespace* CScriptContext::FindNamespace(uint32 nshash)
{
    if (nshash == 0)
        nshash = Hash(kGlobalNamespace);
    CNamespace* namespaceentry = mNamespaceDictionary->FindItem(nshash);
    return (namespaceentry);
}

// ====================================================================================================================
// LinkNamespaces():  Link a child namespace to a parent, creating a hierarchy.
// ====================================================================================================================
void CScriptContext::LinkNamespaces(const char* childnsname, const char* parentnsname)
{
    // sanity check
    if (!childnsname || !childnsname[0] || !parentnsname || !parentnsname[0])
        return;

    // -- ensure the child exists and the parent exists
    TinScript::CNamespace* childns = FindOrCreateNamespace(childnsname, true);
    TinScript::CNamespace* parentns = FindOrCreateNamespace(parentnsname, true);
    LinkNamespaces(childns, parentns);
}

// ====================================================================================================================
// LinkNamespaces():  Link a child namespace to a parent, creating a hierarchy.
// ====================================================================================================================
void CScriptContext::LinkNamespaces(CNamespace* childns, CNamespace* parentns)
{
    // -- sanity check
    if (!childns || !parentns || parentns == childns)
        return;

    if (childns->GetNext() == NULL)
    {
        // -- verify the parent is not already in the hierarchy, or we'll have a circular list
        CNamespace* tempns = parentns;
        while (tempns)
        {
            if (tempns == childns)
            {
                ScriptAssert_(this, 0, "<internal>", -1,
                             "Error - attempting to link namespace %s to %s, which is already its child\n",
                             UnHash(childns->GetHash()), UnHash(parentns->GetHash()));
                return;
            }
            tempns = tempns->GetNext();
        }

        // -- nothing found in the hierarchy - go ahead and link
        childns->SetNext(parentns);
        return;
    }

    // -- child is already linked - see if the new parent is already in the hierarchy
    else
    {
        CNamespace* tempns = childns->GetNext();
        while (tempns)
        {
            if (tempns == parentns)
                return;
            else
                tempns = tempns->GetNext();
        }

        // -- not found in the hierarchy already - we need see then if the current parent
        // -- is in the hierarchy of the new parent
        bool8 found = false;
        CNamespace* oldparent = childns->GetNext();
        tempns = parentns;
        while (tempns)
        {
            if (tempns == oldparent)
            {
                found = true;
                break;
            }
            tempns = tempns->GetNext();
        }

        // -- if it was found, link the namespaces and exit
        if (found)
        {
            childns->SetNext(parentns);
            return;
        }

        // -- not found in the hierarchy - we can insert the new parent into the hierarchy
        // -- *only* if the order is deterministic - which means one must be a registered
        // -- class, and one must be a script class
        if (parentns->GetNext() == NULL)
        {
            // -- if the parent is *not* a registered class, then it goes at the end of the
            // -- child's hierarchy, as long as the child's hierarchy is still purely script
            bool found_registered_class = false;
            if (parentns->IsRegisteredClass() && !childns->IsRegisteredClass())
            {
                tempns = childns;
                while (tempns->GetNext())
                {
                    if (tempns->GetNext()->IsRegisteredClass())
                    {
                        found_registered_class = true;
                        break;
                    }

                    // -- get the next
                    tempns = tempns->GetNext();
                }

                // -- see if we can append to the hierarchy
                if (!found_registered_class)
                {
                    tempns->SetNext(parentns);
                    return;
                }
            }

            // -- else if the parent is not a registered class, then it gets inserted at the front of the
            // -- current child hierarchy
            else if (!parentns->IsRegisteredClass())
            {
                parentns->SetNext(childns->GetNext());
                childns->SetNext(parentns);
            }
            return;
        }
    }

    // -- not found in the hierarchy - assert
    ScriptAssert_(this, 0, "<internal>", -1,
                  "Error - attempting to link namespace %s to %s, already linked to %s\n",
                  UnHash(childns->GetHash()), UnHash(parentns->GetHash()),
                  UnHash(childns->GetNext()->GetHash()));
}

// ====================================================================================================================
// GetNextObjectID():  Generate the object ID for the next object registered.
// ====================================================================================================================
uint32 CScriptContext::GetNextObjectID()
{
    // -- every object created gets a unique ID, so we can find it in the object dictionary
    // -- providing a way to register code-instantiated objects
    return ++mObjectIDGenerator;
}

// ====================================================================================================================
// CreateObject():  Create an object instance, given the hash of the class to use, and the hash of the object's name.
// ====================================================================================================================
uint32 CScriptContext::CreateObject(uint32 classhash, uint32 objnamehash)
{
    uint32 objectid = GetNextObjectID();

    // -- find the creation function
    CNamespace* namespaceentry = GetNamespaceDictionary()->FindItem(classhash);
    if (namespaceentry != NULL)
    {
        // -- loop down the hierarchy, until you find the actual registered class namespace
        CNamespace* class_namespace = namespaceentry;
        while (class_namespace && !class_namespace->IsRegisteredClass())
            class_namespace = class_namespace->GetNext();

        // -- if we get all the way down and don't find a registered class, assume the CScriptObject base class,
        // -- and the hierarchy of linked namespaces will uncover any errors
        if (!class_namespace)
        {
            class_namespace = GetNamespaceDictionary()->FindItem(Hash("CScriptObject"));
            if (class_namespace)
            {
                LinkNamespaces(namespaceentry, class_namespace);
                TinPrint(this, "Warning - CreateObject():  Unable to find registered class %s.\n"
                               "Linking to default base class CScriptObject\n", UnHash(classhash));
            }
        }

        CNamespace::CreateInstance funcptr = class_namespace ? class_namespace->GetCreateInstance() : NULL;
        if (funcptr == NULL)
        {
            ScriptAssert_(this, 0, "<internal>", -1,
                          "Error - Class is not registered: %s\n", UnHash(classhash));
            return 0;
        }

        // -- create the object
        void* newobj = (*funcptr)();

        // -- see if we can hook this object up to the namespace for it's object name
        CNamespace* objnamens = namespaceentry;
        if (objnamehash != 0)
        {
            objnamens = GetNamespaceDictionary()->FindItem(objnamehash);
            if (!objnamens)
                objnamens = namespaceentry;
            else
            {
                // -- link the namespaces
                LinkNamespaces(objnamens, namespaceentry);
            }
        }

        // -- need to verify that if we're using an objnamens (scripted), that the namespaceentry
        // -- is the highest level registered class
        if (objnamens != namespaceentry)
        {
            CNamespace* tempns = objnamens;
            while (tempns && tempns->GetCreateInstance() == NULL)
                tempns = tempns->GetNext();

            // -- if we run out of namespaces... how'd we create this object?
            if (!tempns)
            {
                ScriptAssert_(this, 0, "<internal>", -1,
                              "Error - Unable to verify hierarchy for namespace: %s\n",
                              UnHash(objnamens->GetHash()));
                // $$$TZA find a way to delete the newly created, but non-registered object
                //delete newobj;
                return 0;
            }
            else if (tempns != class_namespace)
            {
                ScriptAssert_(this, 0, "<internal>", -1,
                              "Error - Unable to create an instance of base class: %s, using object namespace: %s.\n"
                              "Use derived class: %s\n",
                              UnHash(class_namespace->GetHash()), UnHash(objnamehash), UnHash(tempns->GetHash()));
                // $$$TZA find a way to delete the newly created, but non-registered object
                //delete newobj;
                return 0;
            }
        }

        // -- add this object to the dictionary of all objects created from script
        CObjectEntry* newobjectentry = TinAlloc(ALLOC_ObjEntry, CObjectEntry, this,
                                                objectid, objnamehash, objnamens, newobj, false);
        GetObjectDictionary()->AddItem(*newobjectentry, objectid);

        // -- add the object to the dictionary by address
        GetAddressDictionary()->AddItem(*newobjectentry, kPointerToUInt32(newobj));

        // -- if the item is named, add it to the name dictionary
        // $$$TZA Note:  names are not guaranteed unique...  warn?
        if (objnamehash != Hash(""))
            GetNameDictionary()->AddItem(*newobjectentry, objnamehash);

        // -- notify the debugger of the new object (before we call OnCreate(), as that may add the object to a set)
        DebuggerNotifyCreateObject(newobjectentry);

        // -- see if the "OnCreate" has been defined - it's not required to
        CFunctionEntry* createfunc = newobjectentry->GetFunctionEntry(0, Hash("OnCreate"));
        if (createfunc)
        {
            // -- call the script "OnCreate" for the object
            int32 dummy = 0;
            ObjExecF(objectid, dummy, "OnCreate();");
        }

        return (objectid);
    }
    else
    {
        ScriptAssert_(this, 0, "<internal>", -1,
                      "Error - Class is not registered: %s\n", UnHash(classhash));
        return 0;
    }
}

// ====================================================================================================================
// RegisterObject():  Given the address of an object instantiated outside TinScript, register it independently.
// ====================================================================================================================
uint32 CScriptContext::RegisterObject(void* objaddr, const char* classname, const char* objectname)
{
    // -- sanity check
    if (!objaddr || !classname || !classname[0])
        return 0;

    // -- ensure the classname is for an existing registered class
    uint32 nshash = Hash(classname);
    CNamespace* namespaceentry = FindNamespace(nshash);
    if (!namespaceentry) {
        ScriptAssert_(this, 0, "<internal>", -1,
                       "Error - Class is not registered: %s\n", classname);
        return 0;
    }

    uint32 objectid = GetNextObjectID();

        // -- see if we can hook this object up to the namespace for it's object name
    uint32 objnamehash = objectname ? Hash(objectname) : 0;
    CNamespace* objnamens = namespaceentry;
    if (objnamehash != 0) {
        objnamens = GetNamespaceDictionary()->FindItem(objnamehash);
        if (!objnamens)
            objnamens = namespaceentry;
        else {
            // -- link the namespaces
            LinkNamespaces(objnamens, namespaceentry);
        }
    }

    // -- add this object to the dictionary of all objects created from script
    CObjectEntry* newobjectentry = TinAlloc(ALLOC_ObjEntry, CObjectEntry, this,
                                            objectid, objnamehash, objnamens, objaddr, true);
    GetObjectDictionary()->AddItem(*newobjectentry, objectid);

    // -- add the object to the dictionary by address
    GetAddressDictionary()->AddItem(*newobjectentry, kPointerToUInt32(objaddr));

    // -- if the item is named, add it to the name dictionary
    if (objnamehash != Hash("")) {
        GetNameDictionary()->AddItem(*newobjectentry, objnamehash);
    }

    // -- notify the debugger of the new object (before we call OnCreate(), as that may add the object to a set)
    DebuggerNotifyCreateObject(newobjectentry);

    // -- see if the "OnCreate" has been defined - it's not required to
    CFunctionEntry* createfunc = newobjectentry->GetFunctionEntry(0, Hash("OnCreate"));
    if (createfunc) {
        // -- call the script "OnCreate" for the object
        if (HasMethod(objectid, "OnCreate")) {
            int32 dummy = 0;
            ObjExecF(objectid, dummy, "OnCreate();");
        }
    }

    return objectid;
}

// ====================================================================================================================
// UnregisterObject():  Used to remove all TinScript references to an object instantiated outside the system.
// ====================================================================================================================
void CScriptContext::UnregisterObject(void* objaddr)
{
    // -- find the ID of the registered object
    uint32 objectid = FindIDByAddress(objaddr);
    if (objectid == 0)
        return;

    // -- destroy the object (will only remove from dictionaries and call ::OnDestroy())
    // -- registered objects are new'd from code, and must be delete'd as well
    DestroyObject(objectid);
}

// ====================================================================================================================
// DestroyObject():  Destroy an object, calls the OnDestroy() method, and deletes it.
// ====================================================================================================================
void CScriptContext::DestroyObject(uint32 objectid)
{
    // -- find this object in the dictionary of all objects created from script
    CObjectEntry* oe = GetObjectDictionary()->FindItem(objectid);
    if (!oe)
    {
        ScriptAssert_(this, 0, "<internal>", -1,
                      "Error - Unable to find object: %d\n", objectid);
        return;
    }

    // -- notify the debugger of the new object (before we call OnCreate(), as that may add the object to a set)
    DebuggerNotifyDestroyObject(objectid);

    // -- notify the master membership list to remove it from all groups
    GetMasterMembershipList()->OnDelete(oe);

    // -- get the namespace entry for the object
    CNamespace* namespaceentry = oe->GetNamespace();
    if (!namespaceentry)
    {
        ScriptAssert_(this, 0, "<internal>", -1,
                      "Error - Unable to find the namespace for object: %d\n", objectid);
        return;
    }

    // -- get the Destroy function
    CNamespace::DestroyInstance destroyptr = namespaceentry->GetDestroyInstance();
    if (destroyptr == NULL)
    {
        ScriptAssert_(this, 0, "<internal>", -1,
                      "Error - no Destroy() function registered for class: %s\n",
                      UnHash(namespaceentry->GetHash()));
        return;
    }

    // -- see if the "OnDestroy" has been defined - it's not required to
    CFunctionEntry* destroyfunc = oe->GetFunctionEntry(0, Hash("OnDestroy"));
    if (destroyfunc)
    {
        int32 dummy = 0;
        ObjExecF(objectid, dummy, "OnDestroy();");
    }

    // -- get the address of the object
    void* objaddr = oe->GetAddr();
    if (!objaddr)
    {
        ScriptAssert_(this, 0, "<internal>", -1,
                      "Error - no address for object: %d\n", objectid);
        return;
    }

    // -- cancel all pending schedules related to this object
    GetScheduler()->CancelObject(objectid);

    // -- if the object was not registered externally, delete the actual object
    if (!oe->IsManuallyRegistered())
        (*destroyptr)(objaddr);

    // -- remove the object from the dictionary, and delete the entry
    GetObjectDictionary()->RemoveItem(objectid);
    GetAddressDictionary()->RemoveItem(kPointerToUInt32(objaddr));
    GetNameDictionary()->RemoveItem(oe->GetNameHash());

    // -- delete the object entry *after* the object
    TinFree(oe);
}

// ====================================================================================================================
// FindObjectEntry():  Finds an object, given the ID.
// ====================================================================================================================
CObjectEntry* CScriptContext::FindObjectEntry(uint32 objectid)
{
    CObjectEntry* oe = GetObjectDictionary()->FindItem(objectid);
    return oe;
}

// ====================================================================================================================
// FindObjectByAddress():  Find an entry, given the address of an object registered but instantiated outside TinScript.
// ====================================================================================================================
CObjectEntry* CScriptContext::FindObjectByAddress(void* addr)
{
    CObjectEntry* oe = GetAddressDictionary()->FindItem(kPointerToUInt32(addr));
    return oe;
}

// ====================================================================================================================
// FindObjectByName():  Find an object entry, given it's name.
// ====================================================================================================================
CObjectEntry* CScriptContext::FindObjectByName(const char* objname)
{
    if (!objname || !objname[0])
        return (NULL);
    CObjectEntry* oe = GetNameDictionary()->FindItem(Hash(objname));
    return oe;
}

// ====================================================================================================================
// FindIDByAddress():  Find an object entry, give the address, and return the ID.
// ====================================================================================================================
uint32 CScriptContext::FindIDByAddress(void* addr)
{
    CObjectEntry* oe = GetAddressDictionary()->FindItem(kPointerToUInt32(addr));
    return oe ? oe->GetID() : 0;
}

// ====================================================================================================================
// FindObject():  Find an object by ID, but return NULL if a required namespace is not part of its hierarchy.
// ====================================================================================================================
void* CScriptContext::FindObject(uint32 objectid, const char* required_namespace) {
    CObjectEntry* oe = GetObjectDictionary()->FindItem(objectid);
    if (oe && (!required_namespace || !required_namespace[0] ||
              oe->HasNamespace(Hash(required_namespace))))
    {
        return (oe->GetAddr());
    }
    return (NULL);
}

// ====================================================================================================================
// HasMethod():  Return 'true' if there's a registered or scripted method in the hierarchy of an object.
// ====================================================================================================================
bool8 CScriptContext::HasMethod(void* addr, const char* method_name)
{
    if (!addr || !method_name)
        return (false);

    CObjectEntry* oe = GetAddressDictionary()->FindItem(kPointerToUInt32(addr));
    if (!oe)
        return (false);

    uint32 function_hash = Hash(method_name);
    CFunctionEntry* fe = oe->GetFunctionEntry(0, function_hash);
    return (fe != NULL);
}

// ====================================================================================================================
// HasMethod():  Return 'true' if there's a registered or scripted method in the hierarchy of an object.
// ====================================================================================================================
bool8 CScriptContext::HasMethod(uint32 objectid, const char* method_name)
{
    if (!method_name)
        return (false);

    CObjectEntry* oe = GetObjectDictionary()->FindItem(objectid);
    if (!oe)
        return (false);

    uint32 function_hash = Hash(method_name);
    CFunctionEntry* fe = oe->GetFunctionEntry(0, function_hash);
    return (fe != NULL);
}

// ====================================================================================================================
// AddDynamicVariable():  Given an object id, add a variable (or array) of a given type.
// ====================================================================================================================
bool8 CScriptContext::AddDynamicVariable(uint32 objectid, uint32 varhash, eVarType vartype, int32 array_size)
{
    CObjectEntry* oe = GetObjectDictionary()->FindItem(objectid);
    if (!oe) {
        ScriptAssert_(this, 0, "<internal>", -1,
                      "Error - Unable to find object: %d\n", objectid);
        return (false);
    }
    return (oe->AddDynamicVariable(varhash, vartype, array_size));
}

// ====================================================================================================================
// AddDynamicVariable():  Given an object id, add a dynamic variable by name and type name.
// ====================================================================================================================
bool8 CScriptContext::AddDynamicVariable(uint32 objectid, const char* varname, const char* vartypename,
                                         int32 array_size)
{
    if (!varname || !vartypename)
    {
        ScriptAssert_(this, 0, "<internal>", -1,
                      "Error - AddDynamicVariable with no var name/type\n");
        return (false);
    }

    uint32 varhash = Hash(varname);
    eVarType vartype = GetRegisteredType(vartypename, (int32)strlen(vartypename));
    return (AddDynamicVariable(objectid, varhash, vartype, array_size));
}

// ====================================================================================================================
// SetMemberVar():  Given an object id, and the name of a member, set the value.
// ====================================================================================================================
bool8 CScriptContext::SetMemberVar(uint32 objectid, const char* varname, void* value)
{
    if (!varname || !value)
    {
        ScriptAssert_(this, 0, "<internal>", -1,
                      "Error - invalid call to SetMemberVar\n");
        return (false);
    }

    CObjectEntry* oe = GetObjectDictionary()->FindItem(objectid);
    if (!oe)
    {
        ScriptAssert_(this, 0, "<internal>", -1,
                      "Error - Unable to find object: %d\n", objectid);
        return (false);
    }

    uint32 varhash = Hash(varname);
    return oe->SetMemberVar(varhash, value);
}

// ====================================================================================================================
// PrintObject():  Debug method to go through the hierarchy of namespaces, and dump the members/values.
// ====================================================================================================================
void CScriptContext::PrintObject(CObjectEntry* oe, int32 indent)
{
    if (!oe)
        return;

    // -- find the actual class
    CNamespace* classns = oe->GetNamespace();
    while (classns && !classns->IsRegisteredClass())
        classns = classns->GetNext();
    if (!classns)
    {
        ScriptAssert_(this, 0, "<internal>", -1,
                      "Error - Registered object with no class: [%d] %s\n",
                      oe->GetID(), oe->GetName());
        return;
    }

    // -- print the indent
    const char* indentbuf = "    ";
    for (int32 i = 0; i < indent; ++i)
        TinPrint(this, indentbuf);

    // -- print the object id and name
    TinPrint(this, "[%d] %s:", oe->GetID(), oe->GetName());
    bool8 first = true;
    CNamespace* ns = oe->GetNamespace();
    while (ns)
    {
        // -- if this is registered class, highlight it
        if (ns->IsRegisteredClass())
        {
            TinPrint(this, "%s[%s]", !first ? "-->" : " ", UnHash(ns->GetHash()));
        }
        else
        {
            TinPrint(this, "%s%s", !first ? "-->" : " ", UnHash(ns->GetHash()));
        }
        first = false;
        ns = ns->GetNext();
    }

    TinPrint(this, "\n");
}

// ====================================================================================================================
// ListObjects():  Debug method to list all registered objects.
// ====================================================================================================================
void CScriptContext::ListObjects()
{
    TinPrint(this, "\n");
    CObjectEntry* oe = GetObjectDictionary()->First();
    while (oe)
    {
        // -- if the object has a parent group, don't bother printing it - it'll have already
        // -- been printed by its parent group
        if (oe->GetObjectGroup() == NULL)
            PrintObject(oe);

        // -- if the object itself is a group, list it's children
        if (HasMethod(oe->GetID(), "ListObjects"))
        {
            int32 dummy = 0;
            ObjExecF(oe->GetID(), dummy, "ListObjects(1);");
        }

        // -- next object
        oe = GetObjectDictionary()->Next();
    }
}

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CNamespace::CNamespace(CScriptContext* script_context, const char* _name, uint32 _typeID,
                       CreateInstance _createinstance, DestroyInstance _destroyinstance)
{
    mContextOwner = script_context;
    // -- ensure the name lives in the string table
    mName = _name && _name[0] ? script_context->GetStringTable()->AddString(_name)
                             : script_context->GetStringTable()->AddString(kGlobalNamespace);
    mHash = Hash(mName);
    mTypeID = _typeID;
    mNext = NULL;
    mCreateFuncptr = _createinstance;
    mDestroyFuncptr = _destroyinstance;
    mMemberTable = TinAlloc(ALLOC_VarTable, tVarTable, kLocalVarTableSize);
    mMethodTable = TinAlloc(ALLOC_FuncTable, tFuncTable, kLocalFuncTableSize);
}

// ====================================================================================================================
// Destructor
// ====================================================================================================================
CNamespace::~CNamespace()
{
    mMemberTable->DestroyAll();
    TinFree(mMemberTable);
    mMethodTable->DestroyAll();
    TinFree(mMethodTable);
}

// ====================================================================================================================
// GetVarEntry():  Find a variable registered in a given namespace
// ====================================================================================================================
CVariableEntry* CNamespace::GetVarEntry(uint32 varhash)
{
    CNamespace* curnamespace = this;
    while (curnamespace)
    {
        CVariableEntry* ve = GetVarTable()->FindItem(varhash);
        if (ve)
            return ve;
        else
            curnamespace = curnamespace->GetNext();
    }

    // -- not found
    return (NULL);
}

};

// ====================================================================================================================
// EOF
// ====================================================================================================================
