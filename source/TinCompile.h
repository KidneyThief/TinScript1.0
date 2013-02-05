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

#ifndef __TINCOMPILE_H
#define __TINCOMPILE_H

#include "assert.h"

#include "TinTypes.h"
#include "TinParse.h"
#include "TinRegistration.h"
#include "TinExecute.h"
#include "TinScript.h"

namespace TinScript {

// ------------------------------------------------------------------------------------------------
// forward declarations
class CVariableEntry;
class CFunctionContext;
class CFunctionEntry;
class CExecStack;
class CFunctionCallStack;

// ------------------------------------------------------------------------------------------------
// compile tree node types
#define CompileNodeTypesTuple \
	CompileNodeTypeEntry(NOP)					\
	CompileNodeTypeEntry(Value)					\
	CompileNodeTypeEntry(Self)       			\
	CompileNodeTypeEntry(ObjMember) 			\
	CompileNodeTypeEntry(Assignment)			\
	CompileNodeTypeEntry(BinaryOp)				\
	CompileNodeTypeEntry(UnaryOp)				\
	CompileNodeTypeEntry(IfStmt)				\
	CompileNodeTypeEntry(CondBranch)			\
	CompileNodeTypeEntry(WhileLoop)				\
	CompileNodeTypeEntry(ForLoop)				\
	CompileNodeTypeEntry(FuncDecl)				\
	CompileNodeTypeEntry(FuncCall)				\
	CompileNodeTypeEntry(FuncReturn)			\
	CompileNodeTypeEntry(ObjMethod) 			\
	CompileNodeTypeEntry(ArrayHash) 			\
	CompileNodeTypeEntry(ArrayVarDecl)			\
	CompileNodeTypeEntry(SelfVarDecl)			\
	CompileNodeTypeEntry(Schedule)			    \
	CompileNodeTypeEntry(CreateObject)  	    \
	CompileNodeTypeEntry(DestroyObject)  	    \

// enum
enum ECompileNodeType {
	#define CompileNodeTypeEntry(a) e##a,
	CompileNodeTypesTuple
	#undef CompileNodeTypeEntry

	eNodeTypeCount
};

const char* GetNodeTypeString(ECompileNodeType nodetype);

// ------------------------------------------------------------------------------------------------
// operation types

#define OperationTuple \
	OperationEntry(NULL)				\
	OperationEntry(NOP)					\
	OperationEntry(VarDecl)				\
	OperationEntry(ParamDecl)			\
	OperationEntry(Assign)				\
	OperationEntry(PushParam)			\
	OperationEntry(Push)				\
	OperationEntry(PushLocalVar)		\
	OperationEntry(PushLocalValue)		\
	OperationEntry(PushGlobalVar)		\
	OperationEntry(PushGlobalValue)		\
	OperationEntry(PushArrayVar)     	\
	OperationEntry(PushArrayValue)     	\
	OperationEntry(PushMember)			\
	OperationEntry(PushMemberVal)		\
	OperationEntry(PushSelf)    		\
	OperationEntry(Pop)					\
	OperationEntry(Add)					\
	OperationEntry(Sub)					\
	OperationEntry(Mult)				\
	OperationEntry(Div)					\
	OperationEntry(Mod)					\
	OperationEntry(AssignAdd)			\
	OperationEntry(AssignSub)			\
	OperationEntry(AssignMult)			\
	OperationEntry(AssignDiv)			\
	OperationEntry(AssignMod)			\
	OperationEntry(AssignLeftShift)		\
	OperationEntry(AssignRightShift)	\
	OperationEntry(AssignBitAnd)        \
	OperationEntry(AssignBitOr)         \
	OperationEntry(AssignBitXor)        \
	OperationEntry(BooleanAnd)      	\
	OperationEntry(BooleanOr)         	\
	OperationEntry(CompareEqual)		\
	OperationEntry(CompareNotEqual)		\
	OperationEntry(CompareLess)			\
	OperationEntry(CompareLessEqual)	\
	OperationEntry(CompareGreater)		\
	OperationEntry(CompareGreaterEqual)	\
	OperationEntry(BitLeftShift)        \
	OperationEntry(BitRightShift)       \
	OperationEntry(BitAnd)	            \
	OperationEntry(BitOr)	            \
	OperationEntry(BitXor)	            \
	OperationEntry(UnaryPreInc)	        \
	OperationEntry(UnaryPreDec)	        \
	OperationEntry(UnaryBitInvert)	    \
	OperationEntry(UnaryNot)	        \
	OperationEntry(UnaryNeg)	        \
	OperationEntry(UnaryPos)	        \
	OperationEntry(Branch)				\
	OperationEntry(BranchTrue)			\
	OperationEntry(BranchFalse)			\
	OperationEntry(FuncDecl)    		\
	OperationEntry(FuncDeclEnd)    		\
	OperationEntry(FuncCallArgs)		\
	OperationEntry(FuncCall)			\
	OperationEntry(FuncReturn)			\
	OperationEntry(MethodCallArgs)		\
	OperationEntry(NSMethodCallArgs)    \
	OperationEntry(ArrayHash)			\
	OperationEntry(ArrayVarDecl)		\
	OperationEntry(SelfVarDecl)		    \
	OperationEntry(ScheduleBegin)       \
	OperationEntry(ScheduleParam)       \
	OperationEntry(ScheduleEnd)         \
	OperationEntry(CreateObject)		\
	OperationEntry(DestroyObject)		\
	OperationEntry(EOF)					\

enum eOpCode {
	#define OperationEntry(a) OP_##a,
	OperationTuple
	#undef OperationEntry
	OP_COUNT
};

const char* GetOperationString(eOpCode op);

// ------------------------------------------------------------------------------------------------
class CCompileTreeNode
{
	public:
		CCompileTreeNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link,
                         ECompileNodeType nodetype, int _linenumber);
		virtual ~CCompileTreeNode();

		CCompileTreeNode* next;
		CCompileTreeNode* leftchild;
		CCompileTreeNode* rightchild;

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;
		virtual void Dump(char*& output, int& length) const;

		ECompileNodeType GetType() const { return type; }
        CCodeBlock* GetCodeBlock() const { return codeblock; }
        int GetLineNumber() const { return linenumber; }

		static CCompileTreeNode* CreateTreeRoot(CCodeBlock* codeblock);

	protected:
		ECompileNodeType type;
        CCodeBlock* codeblock;
        int linenumber;

	protected:
		CCompileTreeNode(CCodeBlock* _codeblock = NULL) {
            type = eNOP;
            codeblock = _codeblock;
            linenumber = -1;
        }
};

class CValueNode : public CCompileTreeNode
{
	public:
		CValueNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber, const char* _value,
                   int _valuelength, bool _isvar, eVarType _valtype);
		CValueNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber, int _paramindex,
                   eVarType _valtype);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;
		virtual void Dump(char*& output, int& length) const;

	protected:
		bool isvariable;
        bool isparam;
        int paramindex;
		char value[kMaxTokenLength];
        eVarType valtype;

	protected:
		CValueNode() { }
};

class CBinaryOpNode : public CCompileTreeNode
{
	public:
		CBinaryOpNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                      eBinaryOpType _binaryoptype, bool _isassignop, eVarType _resulttype);

		CBinaryOpNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                      eAssignOpType _assoptype, bool _isassignop, eVarType _resulttype);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;
		virtual void Dump(char*& output, int& length)const;

        eOpCode GetOpCode() const { return binaryopcode; }
        int GetBinaryOpPrecedence() const { return binaryopprecedence; }

	protected:
        eOpCode binaryopcode;
        int binaryopprecedence;
		eVarType binopresult;
		bool isassignop;

	protected:
		CBinaryOpNode() { }
};

class CUnaryOpNode : public CCompileTreeNode
{
	public:
		CUnaryOpNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                     eUnaryOpType _unaryoptype);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CUnaryOpNode() { }
        eOpCode unaryopcode;
};

class CSelfNode : public CCompileTreeNode
{
	public:
		CSelfNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CSelfNode() { }
};

class CObjMemberNode : public CCompileTreeNode
{
	public:
		CObjMemberNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                       const char* _membername, int _memberlength);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;
		virtual void Dump(char*& output, int& length) const;

	protected:
		char membername[kMaxTokenLength];

	protected:
		CObjMemberNode() { }
};

class CIfStatementNode : public CCompileTreeNode
{
	public:
		CIfStatementNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CIfStatementNode() { }
};

class CCondBranchNode : public CCompileTreeNode
{
	public:
		CCondBranchNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CCondBranchNode() { }
};

class CWhileLoopNode : public CCompileTreeNode
{
	public:
		CWhileLoopNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CWhileLoopNode() { }
};

class CParenOpenNode : public CCompileTreeNode
{
	public:
		CParenOpenNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CParenOpenNode() { }
};

class CFuncDeclNode : public CCompileTreeNode
{
	public:
		CFuncDeclNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                      const char* _funcname, int _length, const char* _funcns, int _funcnslength);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;
		virtual void Dump(char*& output, int& length)const;

	protected:
		CFuncDeclNode() { }
        char funcname[kMaxNameLength];
        char funcnamespace[kMaxNameLength];
        CFunctionEntry* functionentry;
};

class CFuncCallNode : public CCompileTreeNode
{
	public:
		CFuncCallNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                      const char* _funcname, int _length, const char* _nsname, int _nslength,
                      bool _ismethod);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;
		virtual void Dump(char*& output, int& length)const;

	protected:
		char funcname[kMaxNameLength];
		char nsname[kMaxNameLength];
        bool ismethod;

	protected:
		CFuncCallNode() { }
};

class CFuncReturnNode : public CCompileTreeNode
{
	public:
		CFuncReturnNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CFuncReturnNode() { }
        CFunctionEntry* functionentry;
};

class CObjMethodNode : public CCompileTreeNode
{
	public:
		CObjMethodNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                       const char* _methodname, int _methodlength);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;
		virtual void Dump(char*& output, int& length) const;

	protected:
		char methodname[kMaxTokenLength];

	protected:
		CObjMethodNode() { }
};

class CArrayHashNode : public CCompileTreeNode
{
	public:
		CArrayHashNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CArrayHashNode() { }
};

class CArrayVarDeclNode : public CCompileTreeNode
{
	public:
		CArrayVarDeclNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                          eVarType _type);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CArrayVarDeclNode() { }
        eVarType type;
};

class CSelfVarDeclNode : public CCompileTreeNode
{
	public:
		CSelfVarDeclNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                         const char* _varname, int _varnamelength, eVarType _type);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CSelfVarDeclNode() { }
        eVarType type;
        char varname[kMaxNameLength];
};

class CScheduleNode : public CCompileTreeNode
{
	public:
		CScheduleNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                      const char* _funcname, int _funclength, int _delaytime);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;
		virtual void Dump(char*& output, int& length)const;

	protected:
		char funcname[kMaxNameLength];
        int delaytime;

	protected:
		CScheduleNode() { }
};

class CSchedParamNode : public CCompileTreeNode
{
	public:
		CSchedParamNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                        int _paramindex);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

    protected :
        int paramindex;

	protected:
		CSchedParamNode() { }
};

class CCreateObjectNode : public CCompileTreeNode
{
	public:
		CCreateObjectNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                          const char* _classname, unsigned int _classlength, const char* _objname,
                          unsigned int _objlength);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CCreateObjectNode() { }
		char classname[kMaxTokenLength];
		char objectname[kMaxTokenLength];
};

class CDestroyObjectNode : public CCompileTreeNode
{
	public:
		CDestroyObjectNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber);

		virtual int Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const;

	protected:
		CDestroyObjectNode() { }
};

// ------------------------------------------------------------------------------------------------
class CCodeBlock {
	public:

		CCodeBlock(const char* _filename = NULL);
		virtual ~CCodeBlock();

        void AllocateInstructionBlock(int _size, int _linecount) {
            assert(instrblock == NULL);
			instrblock = NULL;
			instrcount = _size;
			if (_size > 0)
				instrblock = new unsigned int[_size];
            if(_linecount > 0)
                linenumbers = new unsigned int[_linecount];
        }

        const char* GetFileName() const {
            return filename;
        }

        void AddLineNumber(int linenumber, unsigned int* instrptr) {
            if(linenumbers) {
                unsigned int offset = CalcOffset(instrptr);
                linenumbers[linenumberindex++] = (offset << 16) + (linenumber & (0xffff));
            }
            else
                ++linenumbercount;
        }

		const unsigned int GetInstructionCount() const {
			return instrcount;
		}
		const unsigned int* GetInstructionPtr() const {
			return instrblock;
		}
		unsigned int* GetInstructionPtr() {
			return instrblock;
		}

		const unsigned int GetLineNumberCount() const {
			return linenumbercount;
		}
		const unsigned int* GetLineNumberPtr() const {
			return linenumbers;
		}
		unsigned int* GetLineNumberPtr() {
			return linenumbers;
		}

        unsigned int CalcLineNumber(const unsigned int* instrptr) const {
            if(!instrptr || linenumbercount == 0)
                return 0;

            // get the offset
            unsigned int curoffset = CalcOffset(instrptr);
            unsigned int lineindex = 0;
            do {
                unsigned int offset = linenumbers[lineindex] >> 16;
                unsigned int line = linenumbers[lineindex] & 0xffff;
                if(curoffset < offset && line != 0xffff)
                    return (line - 1); // text editors count from 1
                ++lineindex;
            } while (lineindex < linenumbercount);

            return 0;
        }

        unsigned int CalcOffset(const unsigned int* instrptr) const {
            return kBytesToWordCount((unsigned int)instrptr - (unsigned int)instrblock);
        }

        int CalcInstrCount(const CCompileTreeNode& root);
        bool CompileTree(const CCompileTreeNode& root);
        bool Execute(unsigned int offset, CExecStack& execstack,
                     CFunctionCallStack& funccallstack);

        void AddFunction(CFunctionEntry* _func) {
            assert(_func);
            if(!functionlist->FindItem(_func->GetHash())) {
                functionlist->AddItem(*_func, _func->GetHash());
            }
        }

        void RemoveFunction(CFunctionEntry* _func) {
            assert(_func);
            functionlist->RemoveItem(_func->GetHash());
        }

        int IsInUse() {
            return !functionlist->IsEmpty();
        }

        CFunctionCallStack* smFuncDefinitionStack;
        tVarTable* smCurrentGlobalVarTable;

        static void Initialize() {
            gCodeBlockList = new CHashTable<CCodeBlock>(kGlobalFuncTableSize);
        }

        static void Shutdown() {
            DestroyUnusedCodeBlocks();
            assert(gCodeBlockList->IsEmpty());
            delete gCodeBlockList;
        }

        static void DestroyCodeBlock(CCodeBlock* codeblock) {
            if(!codeblock)
                return;
            if(codeblock->IsInUse()) {
                ScriptAssert_(0, "<internal>", -1,
                              "Error - Attempting to destroy active codeblock: %s\n", codeblock->GetFileName());
                return;
            }
            gCodeBlockList->RemoveItem(codeblock, codeblock->filenamehash);
            delete codeblock;
        }

        static void DestroyUnusedCodeBlocks() {
            for(unsigned int i = 0; i < gCodeBlockList->Size(); ++i) {
                CCodeBlock* codeblock = gCodeBlockList->FindItemByBucket(i);
                while(codeblock) {
                    CCodeBlock* nextcodeblock = gCodeBlockList->GetNextItemInBucket(i);
                    if(!codeblock->IsInUse()) {
                        gCodeBlockList->RemoveItem(codeblock, codeblock->filenamehash);
                        delete codeblock;
                        codeblock = gCodeBlockList->FindItemByBucket(i);
                    }
                    else {
                        codeblock = nextcodeblock;
                    }
                }
            }
        }

	private:
        char filename[kMaxNameLength];
        unsigned int filenamehash;
		unsigned int* instrblock;
		unsigned int instrcount;

        // -- keep track of the linenumber offsets
        unsigned int linenumberindex;
        unsigned int linenumbercount;
        unsigned int* linenumbers;

        // -- need to keep a list of all functions that are tied to this codeblock
        tFuncTable* functionlist;

        // -- need to keep a list of all the current codeblocks, hashed by filename
        static CHashTable<CCodeBlock>* gCodeBlockList;
};

// ------------------------------------------------------------------------------------------------
// debugging support
void SetDebugCodeBlock(bool torf);
bool GetDebugCodeBlock();

}  // TinScript

#endif // __TINCOMPILE_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
