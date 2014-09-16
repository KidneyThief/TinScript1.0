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
// TinObjectGroup.cpp
// ====================================================================================================================

// -- includes
#include "assert.h"

#include "TinScript.h"
#include "TinRegistration.h"
#include "TinObjectGroup.h"

// == namespace TinScript =============================================================================================

namespace TinScript
{

// == class CMasterMembershipList =====================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CMasterMembershipList::CMasterMembershipList(CScriptContext* script_context, int _size)
{
    assert(script_context != NULL && _size > 0);
    mContextOwner = script_context;

    mMasterMembershipList = TinAlloc(ALLOC_ObjectGroup, CHashTable<tMembershipList>, _size);
}

// ====================================================================================================================
// Destructor
// ====================================================================================================================
CMasterMembershipList::~CMasterMembershipList()
{
    if (mMasterMembershipList)
    {
        mMasterMembershipList->DestroyAll();
        TinFree(mMasterMembershipList);
    }
}

// ====================================================================================================================
// AddMembership():  Notify the master list that an object has been added to an object set.
// ====================================================================================================================
void CMasterMembershipList::AddMembership(CObjectEntry* oe, CObjectSet* group)
{
    // -- sanity check
    if (!oe || !group)
        return;

    uint32 group_id = GetScriptContext()->FindIDByAddress(group);
    uint32 object_id = oe->GetID();

    // $$$TZA ensure we don't add an object to a group that is in its hierarchy of children

    // -- get the member list for the specific object, and add this group to it
    tMembershipList* member_list = mMasterMembershipList->FindItem(object_id);
    if (!member_list)
    {
        member_list = TinAlloc(ALLOC_ObjectGroup, tMembershipList, kObjectGroupTableSize);
        mMasterMembershipList->AddItem(*member_list, object_id);
    }

    // -- ensure we don't add this group twice
    if (!member_list->FindItem(group_id))
        member_list->AddItem(*group, group_id);
}

// ====================================================================================================================
// RemoveMembership():  Notify the master list that an object has been removed from an object set.
// ====================================================================================================================
void CMasterMembershipList::RemoveMembership(CObjectEntry* oe, CObjectSet* group)
{
    // -- sanity check
    if (!oe || !group)
        return;

    uint32 group_id = GetScriptContext()->FindIDByAddress(group);
    uint32 object_id = oe->GetID();

    // -- get the member list for the specific object, and add this group to it
    tMembershipList* member_list = mMasterMembershipList->FindItem(object_id);
    if (!member_list)
    {
        ScriptAssert_(GetScriptContext(), 0, "<internal>", -1,
                      "Error - RemoveMembership() - no membership list for object %d\n", object_id);
        return;
    }

    // -- ensure the object is actually in the group
    if (member_list->FindItem(group_id))
    {
        member_list->RemoveItem(group_id);

        // -- if this is the last group the object is a member of, we can delete the group
        if (member_list->Used() == 0)
        {
            mMasterMembershipList->RemoveItem(object_id);
            TinFree(member_list);
        }
    }
}

// ====================================================================================================================
// OnDelete():  Notify the master list that an object is being deleted - this will remove it from all object sets.
// ====================================================================================================================
void CMasterMembershipList::OnDelete(CObjectEntry* oe)
{
    // -- sanity check
    if (!oe)
        return;

    uint32 objectid = oe->GetID();
    // -- see if this object belongs to any groups
    tMembershipList* member_list = mMasterMembershipList->FindItem(oe->GetID());
    if (member_list)
    {
        int32 cur_count = member_list->Used();
        while (cur_count > 1)
        {
            CObjectSet* group = member_list->First();
            group->RemoveObject(objectid);

            // -- ensure our count actually went down
            int32 new_count = member_list->Used();
            if (new_count != (cur_count - 1))
            {
                //Assert_(false, "Error - CMasterMembershipList::OnDelete() failed to remove object");
                Assert_(false);
            }
            cur_count = new_count;
        }

        // -- delete the last membership entry separately since empty lists
        // -- auto-delete themselves
        CObjectSet* group = member_list->First();
        if (group)
            group->RemoveObject(objectid);
    }
}

// == class CObjectSet ================================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CObjectSet::CObjectSet()
{
    mContextOwner = TinScript::GetContext();
    mObjectList = TinAlloc(ALLOC_ObjectGroup, CHashTable<CObjectEntry>, kObjectGroupTableSize);
}

// ====================================================================================================================
// Destructor
// ====================================================================================================================
CObjectSet::~CObjectSet()
{
    // -- go through the object list, and remove membership of all objects
    CMasterMembershipList* membership_list = GetScriptContext()->GetMasterMembershipList();
    if (mObjectList)
    {
        CObjectEntry* oe = mObjectList->First();
        while (oe)
        {
            membership_list->RemoveMembership(oe, this);
            mObjectList->RemoveItem(oe->GetID());
            oe = mObjectList->First();
        }
        TinFree(mObjectList);
    }
}

// ====================================================================================================================
// Contains():  Returns true if the set contains the object.
// ====================================================================================================================
bool8 CObjectSet::Contains(uint32 objectid)
{
    bool8 result = mObjectList->FindItem(objectid) != NULL;
    return (result);
}

// ====================================================================================================================
// AddObject():  Add an object to this object set.
// ====================================================================================================================
void CObjectSet::AddObject(uint32 objectid)
{
    // -- find the object entry
    CObjectEntry* oe = GetScriptContext()->FindObjectEntry(objectid);
    if (!oe)
    {
        ScriptAssert_(GetScriptContext(), 0, "<internal>", -1,
                      "Error - [%d] CObjectSet::AddObject(): unable to find object %d\n",
                      GetScriptContext()->FindObjectByAddress(this)->GetID(), objectid);
        return;
    }

    if (!mObjectList->FindItem(objectid))
    {
        mObjectList->AddItem(*oe, objectid);
        
        // -- notify the master membership list that an object has been added to a group
        GetScriptContext()->GetMasterMembershipList()->AddMembership(oe, this);

        // -- automatically call "OnAdd" for the group
        if (GetScriptContext()->HasMethod(this, "OnAdd"))
        {
            int32 dummy = 0;
            ObjExecF(this, dummy, "OnAdd(%d);", objectid);
        }
    }
}

// ====================================================================================================================
// RemoveObject():  Remove an object from this object set.
// ====================================================================================================================
void CObjectSet::RemoveObject(uint32 objectid)
{
    // -- find the object entry
    CObjectEntry* oe = GetScriptContext()->FindObjectEntry(objectid);
    if (!oe)
    {
        ScriptAssert_(GetScriptContext(), 0, "<internal>", -1,
                      "Error - [%d] CObjectSet::RemoveObject(): unable to find object %d\n",
                      GetScriptContext()->FindObjectByAddress(this)->GetID(), objectid);
        return;
    }

    if (mObjectList->FindItem(objectid))
    {
        mObjectList->RemoveItem(objectid);

        // -- notify the master membership list that an object has been added to a group
        GetScriptContext()->GetMasterMembershipList()->RemoveMembership(oe, this);

        // -- automatically call "OnAdd" for the group
        if (GetScriptContext()->HasMethod(this, "OnRemove"))
        {
            int32 dummy = 0;
            ObjExecF(this, dummy, "OnRemove(%d);", objectid);
        }
    }
}

// ====================================================================================================================
// ListObjects():  Debug method to dump the contents of this object set to standard out.
// ====================================================================================================================
void CObjectSet::ListObjects(int32 indent)
{
    if (indent == 0)
        TinPrint(GetScriptContext(), "\n");

    CObjectEntry* oe = mObjectList->First();
    while (oe)
    {
        GetScriptContext()->PrintObject(oe, indent);

        // -- if the object is an ObjectSet, list it's objects
        if (GetScriptContext()->HasMethod(oe->GetID(), "ListObjects"))
        {
            int32 dummy = 0;
            ObjExecF(oe->GetID(), dummy, "ListObjects(%d);", indent + 1);
        }

        // -- next object
        oe = mObjectList->Next();
    }
}

// ====================================================================================================================
// RemoveAll():  Remove all objects contained in this object set.
// ====================================================================================================================
void CObjectSet::RemoveAll()
{
    while (mObjectList->Used() > 0)
    {
        CObjectEntry* oe = mObjectList->First();
        RemoveObject(oe->GetID());
    }
}

// ====================================================================================================================
// First():  Returns the first object in this set.  Note:  sets the internal iterator, in anticipation of Next().
// ====================================================================================================================
uint32 CObjectSet::First()
{
    CObjectEntry* oe = mObjectList->First();
    if (oe)
        return (oe->GetID());
    else
        return (0);
}

// ====================================================================================================================
// Next():  Returns the next object added to the set, updating the internal iterator.
// ====================================================================================================================
uint32 CObjectSet::Next()
{
    CObjectEntry* oe = mObjectList->Next();
    if (oe)
        return (oe->GetID());
    else
        return (0);
}

// ====================================================================================================================
// Used():  Returns the number of objects contained in tis object set.
// ====================================================================================================================
int32 CObjectSet::Used()
{
    return mObjectList->Used();
}

// ====================================================================================================================
// GetObjectByIndex():  Returns the nth object added to this object set.  Also sets the internal iterator.
// ====================================================================================================================
uint32 CObjectSet::GetObjectByIndex(int32 index)
{
    // -- sanity check
    if (index < 0 || index >= Used())
        return 0;

    // $$$TZA Really crappy implementation - need to convert to CTable<>?
    CObjectEntry* oe = mObjectList->First();
    while (--index >= 0)
        oe = mObjectList->Next();

    return (oe->GetID());
}

// == class CObjectGroup ==============================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CObjectGroup::CObjectGroup() : CObjectSet()
{
}

// ====================================================================================================================
// Destructor
// ====================================================================================================================
CObjectGroup::~CObjectGroup()
{
    // -- object groups actually delete their children
    if (mObjectList)
    {
        CObjectEntry* oe = mObjectList->First();
        while (oe)
        {
            GetScriptContext()->DestroyObject(oe->GetID());
            oe = mObjectList->First();
        }
    }
}

// ====================================================================================================================
// AddObject():  Adds an object to this object group - automatically removes it from it's previous object group.
// ====================================================================================================================
void CObjectGroup::AddObject(uint32 objectid)
{
    // -- find the object entry
    CObjectEntry* oe = GetScriptContext()->FindObjectEntry(objectid);
    if (!oe)
    {
        ScriptAssert_(GetScriptContext(), 0, "<internal>", -1,
                      "Error - [%d] CObjectGroup::AddObject(): unable to find object %d\n",
                      GetScriptContext()->FindObjectByAddress(this)->GetID(), objectid);
        return;
    }

    // -- if we have a current owner, different this group, remove us from it
    CObjectGroup* current_owner = oe->GetObjectGroup();
    if (current_owner == this)
        return;

    if (current_owner)
        current_owner->RemoveObject(objectid);

    // -- add the object to this group
    CObjectSet::AddObject(objectid);
    oe->SetObjectGroup(this);
}

// ====================================================================================================================
// RemoveObject():  Remove an object from this object group.
// ====================================================================================================================
void CObjectGroup::RemoveObject(uint32 objectid)
{
    // -- find the object entry
    CObjectEntry* oe = GetScriptContext()->FindObjectEntry(objectid);
    if (!oe)
    {
        ScriptAssert_(GetScriptContext(), 0, "<internal>", -1,
                      "Error - [%d] CObjectGroup::RemoveObject(): unable to find object %d\n",
                      GetScriptContext()->FindObjectByAddress(this)->GetID(), objectid);
        return;
    }

    // -- remove the object
    CObjectSet::RemoveObject(objectid);
    oe->SetObjectGroup(NULL);
}

// == Registration =====================================================================================================

// =====================================================================================================================
// -- CObjectSet member/method registration
IMPLEMENT_SCRIPT_CLASS_BEGIN(CObjectSet, VOID)
IMPLEMENT_SCRIPT_CLASS_END()

REGISTER_METHOD_P1(CObjectSet, Contains, Contains, bool8, uint32);
REGISTER_METHOD_P1(CObjectSet, AddObject, AddObject, void, uint32);
REGISTER_METHOD_P1(CObjectSet, RemoveObject, RemoveObject, void, uint32);
REGISTER_METHOD_P1(CObjectSet, ListObjects, ListObjects, void, int32);
REGISTER_METHOD_P0(CObjectSet, RemoveAll, RemoveAll, void);

REGISTER_METHOD_P0(CObjectSet, First, First, uint32);
REGISTER_METHOD_P0(CObjectSet, Next, Next, uint32);
REGISTER_METHOD_P0(CObjectSet, Used, Used, int32);
REGISTER_METHOD_P1(CObjectSet, GetObjectByIndex, GetObjectByIndex, uint32, int32);

// =====================================================================================================================
// -- CObjectGroup member/method registration
IMPLEMENT_SCRIPT_CLASS_BEGIN(CObjectGroup, CObjectSet)
IMPLEMENT_SCRIPT_CLASS_END()

REGISTER_METHOD_P1(CObjectGroup, AddObject, AddObject, void, uint32);
REGISTER_METHOD_P1(CObjectGroup, RemoveObject, RemoveObject, void, uint32);

} // TinScript

// =====================================================================================================================
// EOF
// =====================================================================================================================
