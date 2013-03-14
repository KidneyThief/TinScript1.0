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

#ifndef __TINOBJECTGROUP_H
#define __TINOBJECTGROUP_H

namespace TinScript {

class CObjectSet;

class CMasterMembershipList {
    public:
        typedef CHashTable<CObjectSet> tMembershipList;

        CMasterMembershipList(CScriptContext* script_context = NULL, int32 _size = 0);
        virtual ~CMasterMembershipList();

        CScriptContext* GetScriptContext() {
            return (mContextOwner);
        }

        void AddMembership(CObjectEntry* oe, CObjectSet* group);
        void RemoveMembership(CObjectEntry* oe, CObjectSet* group);

        void OnDelete(CObjectEntry* oe);

    private:
        CScriptContext* mContextOwner;
        CHashTable<tMembershipList>* mMasterMembershipList;
};

// -- a basic storage class for sets of registered objects
class CObjectSet {
    public:
        DECLARE_SCRIPT_CLASS(CObjectSet, VOID);

        CObjectSet(CScriptContext* script_context = NULL, int32 _size = 0);
        virtual ~CObjectSet();

        CScriptContext* GetScriptContext() {
            return (mContextOwner);
        }

        bool8 Contains(uint32 objectid);
        virtual void AddObject(uint32 objectid);
        virtual void RemoveObject(uint32 objectid);
        void ListObjects();

        uint32 First();
        uint32 Next();

        int32 Used();
        uint32 GetObjectByIndex(int32 index);

    protected:
        CScriptContext* mContextOwner;
        CHashTable<CObjectEntry>* mObjectList;
};

// -- a the derived class implies ownership - delete the group, it deletes its children
// -- also, only 
class CObjectGroup : public CObjectSet {
    public:
        DECLARE_SCRIPT_CLASS(CObjectGroup, CObjectSet);

        CObjectGroup(CScriptContext* script_context = NULL, int32 _size = 0);
        virtual ~CObjectGroup();

        virtual void AddObject(uint32 objectid);
        virtual void RemoveObject(uint32 objectid);
};

} // TinScript

#endif

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
