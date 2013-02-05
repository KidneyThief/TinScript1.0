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
// TinCompile.cpp
// ------------------------------------------------------------------------------------------------

// -- lib includes
#include "stdafx.h"
#include "assert.h"
#include "string.h"
#include "stdio.h"

#include "TinScript.h"
#include "TinParse.h"
#include "TinCompile.h"
#include "TinExecute.h"
#include "TinNamespace.h"

// -- name of the global namespace
static const char* kGlobalNamespace = "_global";

// enable this for debug output while the byte code is generated
bool gDebugCodeBlock = false;

namespace TinScript {

CHashTable<CCodeBlock>* CCodeBlock::gCodeBlockList;

// ------------------------------------------------------------------------------------------------
// CCompileTree implementation
const char* gCompileNodeTypes[eNodeTypeCount] = {
	#define CompileNodeTypeEntry(a) #a,
	CompileNodeTypesTuple
	#undef CompileNodeTypeEntry
};

const char* GetNodeTypeString(ECompileNodeType nodetype) {
	return gCompileNodeTypes[nodetype];
}

const char* gOperationName[OP_COUNT] = {
	#define OperationEntry(a) #a,
	OperationTuple
	#undef OperationEntry
};

const char* GetOperationString(eOpCode op) {
	return gOperationName[op];
}

eOpCode gBinInstructionType[] = {
	#define BinaryOperatorEntry(a, b, c) OP_##a,
	BinaryOperatorTuple
	#undef BinaryOperatorEntry
};

int gBinOpPrecedence[] = {
	#define BinaryOperatorEntry(a, b, c) c,
	BinaryOperatorTuple
	#undef BinaryOperatorEntry
};

eOpCode GetBinOpInstructionType(eBinaryOpType binoptype) {
    return gBinInstructionType[binoptype];
}

int GetBinOpPrecedence(eBinaryOpType binoptype) {
    return gBinOpPrecedence[binoptype];
}

static eOpCode gAssInstructionType[] = {
	#define AssignOperatorEntry(a, b) OP_##a,
	AssignOperatorTuple
	#undef AssignOperatorEntry
};

eOpCode GetAssOpInstructionType(eAssignOpType assoptype) {
    return gAssInstructionType[assoptype];
}

static eOpCode gUnaryInstructionType[] = {
	#define UnaryOperatorEntry(a, b) OP_##a,
	UnaryOperatorTuple
	#undef UnaryOperatorEntry
};

eOpCode GetUnaryOpInstructionType(eUnaryOpType unarytype) {
    return gUnaryInstructionType[unarytype];
}

#define DebugByteCodeTuple \
	DebugByteCodeEntry(NULL) \
	DebugByteCodeEntry(instr) \
	DebugByteCodeEntry(vartype) \
	DebugByteCodeEntry(var) \
	DebugByteCodeEntry(value) \
	DebugByteCodeEntry(func) \
	DebugByteCodeEntry(hash) \
	DebugByteCodeEntry(nshash) \
	DebugByteCodeEntry(self) \

enum eDebugByteType {
	#define DebugByteCodeEntry(a) DBG_##a,
	DebugByteCodeTuple
	#undef DebugByteCodeEntry
};

static const char* gDebugByteTypeName[] = {
	#define DebugByteCodeEntry(a) #a,
	DebugByteCodeTuple
	#undef DebugByteCodeEntry
};

// ------------------------------------------------------------------------------------------------
int PushInstructionRaw(bool countonly, unsigned int*& instrptr, void* content, int wordcount,
					eDebugByteType debugtype, const char* debugmsg = NULL) {

	if(!countonly) {
		memcpy(instrptr, content, wordcount * 4);
		instrptr += wordcount;
	}

#if DEBUG_CODEBLOCK
    if(gDebugCodeBlock && !countonly) {
	    for(int i = 0; i < wordcount; ++i) {
		    if (i == 0) {
			    const char* debugtypeinfo = NULL;
			    switch(debugtype) {
				    case DBG_instr:
					    debugtypeinfo = GetOperationString((eOpCode)(*(unsigned int*)content));
					    break;
				    case DBG_vartype:
					    debugtypeinfo = GetRegisteredTypeName((eVarType)(*(unsigned int*)content));
					    break;
                    case DBG_var:
                    case DBG_func:
                        debugtypeinfo = UnHash(*(unsigned int*)content);
                        break;
				    default:
					    debugtypeinfo = "";
					    break;
			    }
			    printf("0x%08x\t\t:\t// [%s: %s]: %s\n", ((unsigned int*)content)[i],
													     gDebugByteTypeName[debugtype],
													     debugtypeinfo,
													     debugmsg ? debugmsg : "");
		    }
		    else
			    printf("0x%x\n", ((unsigned int*)content)[i]);
	    }
    }
#endif
	return wordcount;
}

int PushInstruction(bool countonly, unsigned int*& instrptr, unsigned int content,
					eDebugByteType debugtype, const char* debugmsg = NULL) {
	return PushInstructionRaw(countonly, instrptr, (void*)&content, 1, debugtype, debugmsg);
}

void DebugEvaluateNode(const CCompileTreeNode& node, bool countonly, unsigned int* instrptr) {
#if DEBUG_CODEBLOCK
    if(gDebugCodeBlock && !countonly)
	    printf("\n--- Eval: %s\n", GetNodeTypeString(node.GetType()));

    // if we're debugging, add 
    if(node.GetCodeBlock())
        node.GetCodeBlock()->AddLineNumber(node.GetLineNumber(), instrptr);
#endif
}

void DebugEvaluateBinOpNode(const CBinaryOpNode& binopnode, bool countonly) {
#if DEBUG_CODEBLOCK
    if(gDebugCodeBlock && !countonly) {
	    printf("\n--- Eval: %s [%s]\n", GetNodeTypeString(binopnode.GetType()),
                                        GetOperationString(binopnode.GetOpCode()));
    }
#endif
}

// ------------------------------------------------------------------------------------------------
int CompileVarTable(tVarTable* vartable, unsigned int*& instrptr, bool countonly) {
    int size = 0;
	if(vartable) {
		// -- create instructions to declare each variable
		for(unsigned int i = 0; i < vartable->Size(); ++i) {
			CVariableEntry* ve = vartable->FindItemByBucket(i);
			while (ve) {
                size += PushInstruction(countonly, instrptr, OP_VarDecl, DBG_instr);
                size += PushInstruction(countonly, instrptr, ve->GetHash(), DBG_var);
                size += PushInstruction(countonly, instrptr, ve->GetType(), DBG_vartype);

				ve = vartable->GetNextItemInBucket(i);
			}
		}
	}
    return size;
}

// ------------------------------------------------------------------------------------------------
int CompileFunctionContext(CFunctionContext* funccontext, unsigned int*& instrptr,
                           bool countonly) {
    int size = 0;
    assert(funccontext);

    // -- push the parameters
    int paramcount = funccontext->GetParameterCount();
    for(int i = 0; i < paramcount; ++i) {
        CVariableEntry* ve = funccontext->GetParameter(i);
        assert(ve);
        size += PushInstruction(countonly, instrptr, OP_ParamDecl, DBG_instr);
        size += PushInstruction(countonly, instrptr, ve->GetHash(), DBG_var);
        size += PushInstruction(countonly, instrptr, ve->GetType(), DBG_vartype);
    }

    // -- now declare the rest of the local vars
    tVarTable* vartable = funccontext->GetLocalVarTable();
    assert(vartable);
	if(vartable) {
		for(unsigned int i = 0; i < vartable->Size(); ++i) {
			CVariableEntry* ve = vartable->FindItemByBucket(i);
			while (ve) {
                if(! funccontext->IsParameter(ve)) {
                    size += PushInstruction(countonly, instrptr, OP_VarDecl, DBG_instr);
                    size += PushInstruction(countonly, instrptr, ve->GetHash(), DBG_var);
                    size += PushInstruction(countonly, instrptr, ve->GetType(), DBG_vartype);
                }
				ve = vartable->GetNextItemInBucket(i);
			}
		}
	}

    // -- initialize the stack var offsets
    if(!countonly)
        funccontext->InitStackVarOffsets();

    return size;
}

// ------------------------------------------------------------------------------------------------
CCompileTreeNode* CCompileTreeNode::CreateTreeRoot(CCodeBlock* codeblock)
{
	CCompileTreeNode* root = new CCompileTreeNode(codeblock);
	root->next = NULL;
	root->leftchild = NULL;
	root->rightchild = NULL;

	return root;
}

CCompileTreeNode::CCompileTreeNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link,
                                   ECompileNodeType nodetype, int _linenumber) {
	type = nodetype;
	next = NULL;
	leftchild = NULL;
	rightchild = NULL;

	// -- hook up the node to the tree
	_link = this;

    // -- store the current code block we're compiline
    codeblock = _codeblock;
    linenumber = _linenumber;
}

CCompileTreeNode::~CCompileTreeNode() {
	assert(leftchild == NULL && rightchild == NULL);
}

int CCompileTreeNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	// -- NOP nodes have no children, but loop through and evaluate the chain of siblings
	const CCompileTreeNode* rootptr = next;
	while (rootptr) {
		size += rootptr->Eval(instrptr, TYPE_void, countonly);

        // -- we're done if the rootptr is a NOP, as it would have already evaluated
        // -- the rest of the linked list
        if(rootptr->GetType() == eNOP)
            break;

		rootptr = rootptr->next;
	}

	return size;
}

void CCompileTreeNode::Dump(char*& output, int& length) const
{
	sprintf_s(output, length, "type: %s", gCompileNodeTypes[type]);
	int debuglength = strlen(output);
	output += debuglength;
	length -= debuglength;
}

// ------------------------------------------------------------------------------------------------
CValueNode::CValueNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                       const char* _value, int _valuelength, bool _isvar,
                       eVarType _valtype) :
                       CCompileTreeNode(_codeblock, _link, eValue, _linenumber) {
	SafeStrcpy(value, _value, _valuelength + 1);
	isvariable = _isvar;
    isparam = false;
    valtype = _valtype;
}

CValueNode::CValueNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                       int _paramindex, eVarType _valtype) :
                       CCompileTreeNode(_codeblock, _link, eValue, _linenumber) {
    value[0] = '\0';
	isvariable = false;
    isparam = true;
    paramindex = _paramindex;
    valtype = _valtype;
}

int CValueNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {
	
	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	// if the value is being used, push it on the stack
	if(pushresult > TYPE_void) {
        if(isparam) {
			size += PushInstruction(countonly, instrptr, OP_PushParam, DBG_instr);
			size += PushInstruction(countonly, instrptr, paramindex, DBG_hash);
        }
		else if(isvariable) {
            int stacktopdummy = 0;
            CObjectEntry* dummy = NULL;
            CFunctionEntry* curfunction = codeblock->smFuncDefinitionStack->GetTop(dummy, stacktopdummy);
			// -- ensure we can find the variable
			unsigned int varhash = Hash(value);
            unsigned int funchash = curfunction ? curfunction->GetHash() : 0;
            unsigned int nshash = curfunction ? curfunction->GetNamespaceHash() : 0;
            CVariableEntry* var = GetVariable(codeblock->smCurrentGlobalVarTable, nshash, funchash,
                                              varhash, 0);
			if(!var) {
                ScriptAssert_(0, codeblock->GetFileName(), linenumber,
                              "Error - undefined variable: %s\n", value);
				return 0;
			}
			eVarType vartype = var->GetType();

            // -- if this variable is a hash table, but we're supposed to push a non-hashtable
            // -- result, then we'd better have a hash value to dereference the table
            if(vartype == TYPE_hashtable && pushresult != TYPE_hashtable) {
                if(!rightchild) {
                    ScriptAssert_(0, codeblock->GetFileName(), linenumber,
                                  "Error - hashtable variable %s missing a rightchild\n",
                                  UnHash(var->GetHash()));
				    return 0;
                }

                // -- the right child will be an ArrayHashNode (tree), to resolve to a hash value
                // -- used to index into the hashtable
                size += rightchild->Eval(instrptr, TYPE_int, countonly);
            }

			// -- if we're supposed to be pushing a var (e.g. for an assign...)
            // -- or a hashtable, to declare an array entry...
			if(pushresult == TYPE__var || pushresult == TYPE_hashtable) {
                // -- if we want the hash table entry, or the variable to assign
                if(vartype == TYPE_hashtable && pushresult != TYPE_hashtable)
    				size += PushInstruction(countonly, instrptr, OP_PushArrayVar, DBG_instr);
                // -- else we want the hashtable variable
                else {
                    // -- if this isn't a func var, make sure we push ns 0 for a global variable
                    if(var->GetFunctionEntry() == NULL) {
				        size += PushInstruction(countonly, instrptr, OP_PushGlobalVar, DBG_instr);
				        size += PushInstruction(countonly, instrptr, 0, DBG_hash);
				        size += PushInstruction(countonly, instrptr, funchash, DBG_func);
        				size += PushInstruction(countonly, instrptr, var->GetHash(), DBG_var);
                    }
                    else {
				        size += PushInstruction(countonly, instrptr, OP_PushLocalVar, DBG_instr);
				        size += PushInstruction(countonly, instrptr, var->GetType(), DBG_vartype);

                        // -- for local vars, it's the offset on the stack we need to push
                        int stackoffset = var->GetStackOffset();
                        if(!countonly && stackoffset < 0) {
                            ScriptAssert_(0, codeblock->GetFileName(), linenumber,
                                          "Error - invalid stack offset for local var: %s\n",
                                          UnHash(var->GetHash()));
                            return 0;
                        }
        				size += PushInstruction(countonly, instrptr, stackoffset, DBG_var);
                    }
                }
			}

			// -- otherwise we push the hash, but the instruction is to get the value
			else {
                if(vartype == TYPE_hashtable)
    				size += PushInstruction(countonly, instrptr, OP_PushArrayValue, DBG_instr);
                else {
                    // -- if this isn't a func var, make sure we push ns 0 for a global variable
                    if(var->GetFunctionEntry() == NULL) {
					    size += PushInstruction(countonly, instrptr, OP_PushGlobalValue, DBG_instr);
    			        size += PushInstruction(countonly, instrptr, 0, DBG_hash);
				        size += PushInstruction(countonly, instrptr, funchash, DBG_func);
				        size += PushInstruction(countonly, instrptr, var->GetHash(), DBG_var);
                    }
                    else {
					    size += PushInstruction(countonly, instrptr, OP_PushLocalValue, DBG_instr);
    			        size += PushInstruction(countonly, instrptr, var->GetType(), DBG_vartype);

                        // -- for local vars, it's the offset on the stack we need to push
                        int stackoffset = var->GetStackOffset();
                        if(!countonly && stackoffset < 0) {
                            ScriptAssert_(0, codeblock->GetFileName(), linenumber,
                                          "Error - invalid stack offset for local var: %s\n",
                                          UnHash(var->GetHash()));
                            return 0;
                        }
				        size += PushInstruction(countonly, instrptr, stackoffset, DBG_var);
                    }
                }
			}
		}

		// -- else we're pushing an actual value
		else {
			size += PushInstruction(countonly, instrptr, OP_Push, DBG_instr);

			// -- the next instruction is the type to be pushed
            eVarType pushtype = (pushresult == TYPE__resolve) ? valtype : pushresult;
			size += PushInstruction(countonly, instrptr, pushtype, DBG_vartype);

			// convert the value string to the appropriate type
			// increment the instrptr by the number of 4-byte instructions
			char valuebuf[kMaxTokenLength];
			if(gRegisteredStringToType[pushtype]((void*)valuebuf, (char*)value)) {
				int resultsize = kBytesToWordCount(gRegisteredTypeSize[pushtype]);
    		    size += PushInstructionRaw(countonly, instrptr, (void*)valuebuf, resultsize,
										   DBG_value);
    		}
			else {
				printf("Error - unable to convert value %s to type %s\n",
										value, gRegisteredTypeNames[pushtype]);
			}
		}

		// -- return the size
		return size;
	}

	return size;
}

void CValueNode::Dump(char*& output, int& length) const
{
    if(isparam)
	    sprintf_s(output, length, "type: %s, param: %d", gCompileNodeTypes[type], paramindex);
    else if(isvariable)
	    sprintf_s(output, length, "type: %s, var: %s", gCompileNodeTypes[type], value);
    else
	    sprintf_s(output, length, "type: %s, %s", gCompileNodeTypes[type], value);
	int debuglength = strlen(output);
	output += debuglength;
	length -= debuglength;
}

// ------------------------------------------------------------------------------------------------
CSelfNode::CSelfNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber) :
                               CCompileTreeNode(_codeblock, _link, eSelf, _linenumber) {
}

int CSelfNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {
	
	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	// -- if the value is being used, push it on the stack
	if(pushresult > TYPE_void) {
	    size += PushInstruction(countonly, instrptr, OP_PushSelf, DBG_var);
	}

	return size;
}

// ------------------------------------------------------------------------------------------------
CObjMemberNode::CObjMemberNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                               const char* _membername, int _memberlength) :
                               CCompileTreeNode(_codeblock, _link, eObjMember, _linenumber) {
	SafeStrcpy(membername, _membername, _memberlength + 1);
}

int CObjMemberNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {
	
	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	// -- ensure we have a left child
	if(!leftchild) {
		printf("Error - CObjMemberNode with no left child\n");
		return 0;
	}

  	// -- evaluate the left child, pushing the a result of TYPE_object
	size += leftchild->Eval(instrptr, TYPE_object, countonly);

	// -- if the value is being used, push it on the stack
	if(pushresult > TYPE_void) {

        // -- get the hash of the member
		unsigned int memberhash = Hash(membername);

		// -- if we're supposed to be pushing a var (for an assign...), we actually push
        // -- a member (still a variable, but the lookup is different)
		if(pushresult == TYPE__var) {
			size += PushInstruction(countonly, instrptr, OP_PushMember, DBG_instr);
			size += PushInstruction(countonly, instrptr, memberhash, DBG_var);
		}

		// -- otherwise we push the hash, but the instruction is to get the value
		else {
			size += PushInstruction(countonly, instrptr, OP_PushMemberVal, DBG_instr);
			size += PushInstruction(countonly, instrptr, memberhash, DBG_var);
		}
	}

    // -- otherwise, we're referencing a member without actually doing anything - pop the stack
    else {
        size += PushInstruction(countonly, instrptr, OP_Pop, DBG_instr);
    }

	return size;
}

void CObjMemberNode::Dump(char*& output, int& length) const
{
	sprintf_s(output, length, "type: %s, %s", gCompileNodeTypes[type], membername);
	int debuglength = strlen(output);
	output += debuglength;
	length -= debuglength;
}

// ------------------------------------------------------------------------------------------------
CBinaryOpNode::CBinaryOpNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                             eBinaryOpType _binaryoptype, bool _isassignop, eVarType _resulttype) :
                             CCompileTreeNode(_codeblock, _link, eBinaryOp, _linenumber) {

	binaryopcode = GetBinOpInstructionType(_binaryoptype);
    binaryopprecedence = GetBinOpPrecedence(_binaryoptype);
	binopresult = _resulttype;
	isassignop = _isassignop;
}

CBinaryOpNode::CBinaryOpNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                             eAssignOpType _assoptype, bool _isassignop, eVarType _resulttype) :
                             CCompileTreeNode(_codeblock, _link, eBinaryOp, _linenumber) {

	binaryopcode = GetAssOpInstructionType(_assoptype);
    binaryopprecedence = 0;
	binopresult = _resulttype;
	isassignop = _isassignop;
}

// ------------------------------------------------------------------------------------------------
int CBinaryOpNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateBinOpNode(*this, countonly);
	int size = 0;

	// -- ensure we have a left child
	if(!leftchild) {
		printf("Error - CBinaryOpNode with no left child\n");
		return 0;
	}

	// -- ensure we have a left child
	if(!rightchild) {
		printf("Error - CBinaryOpNode with no right child\n");
		return 0;
	}

	// -- note:  if the binopresult is TYPE_NULL, simply inherit the result from the parent node
	eVarType childresulttype = binopresult != TYPE_NULL ? binopresult : pushresult;

	// -- evaluate the left child, pushing the result of the type required
	// -- except in the case of an assignment operator - the left child is the variable
	size += leftchild->Eval(instrptr, isassignop ? TYPE__var : childresulttype, countonly);

	// -- evaluate the right child, pushing the result
	size += rightchild->Eval(instrptr, childresulttype, countonly);

	// -- push the specific operation to be performed
	size += PushInstruction(countonly, instrptr, binaryopcode, DBG_instr);

	return size;
}

void CBinaryOpNode::Dump(char*& output, int& length) const
{
	sprintf_s(output, length, "type: %s, op: %s", gCompileNodeTypes[type],
              gOperationName[binaryopcode]);
	int debuglength = strlen(output);
	output += debuglength;
	length -= debuglength;
}
// ------------------------------------------------------------------------------------------------
CUnaryOpNode::CUnaryOpNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                           eUnaryOpType _unaryoptype) :
                           CCompileTreeNode(_codeblock, _link, eUnaryOp, _linenumber) {
	unaryopcode = GetUnaryOpInstructionType(_unaryoptype);
}

int CUnaryOpNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	// -- ensure we have a left child
	if(!leftchild) {
		printf("Error - CUnaryOpNode with no left child\n");
		return 0;
	}

    // -- (some) unary operators only operate on specific types
    // $$$TZA We need a way to apply unary operators to user-defined registered types
    // -- e.g.  do we bother evaluating  ++C3Vector?
    eVarType resulttype = pushresult;
    switch(unaryopcode) {
        case OP_UnaryPreInc:
        case OP_UnaryPreDec:
        case OP_UnaryNeg:
        {
            if(pushresult != TYPE_int && pushresult != TYPE_float)
                resulttype = TYPE_float;
            break;
        }

        case OP_UnaryBitInvert:
            resulttype = TYPE_int;
            break;

        case OP_UnaryNot:
            resulttype = TYPE_bool;
            break;

        default:
            break;
    }

	// -- evaluate the left child, pushing the result of the type required
	// -- except in the case of an assignment operator - the left child is the variable
	size += leftchild->Eval(instrptr, resulttype, countonly);

	// -- push the specific operation to be performed
	size += PushInstruction(countonly, instrptr, unaryopcode, DBG_instr);

	return size;
}

// ------------------------------------------------------------------------------------------------
CIfStatementNode::CIfStatementNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link,
                                   int _linenumber) : CCompileTreeNode(_codeblock, _link, eIfStmt,
                                                                       _linenumber) {
}

int CIfStatementNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	// -- ensure we have a left child
	if(!leftchild) {
		printf("Error - CIfStatementNode with no left child\n");
		return 0;
	}

	// -- ensure we have a right child
	if(!rightchild) {
		printf("Error - CIfStatementNode with no right child\n");
		return 0;
	}

	// -- evaluate the left child, which is the condition
	size += leftchild->Eval(instrptr, TYPE_bool, countonly);

	// -- evaluate the right child, which is the branch node
	size += rightchild->Eval(instrptr, TYPE_void, countonly);

	return size;
}

// ------------------------------------------------------------------------------------------------
CCondBranchNode::CCondBranchNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link,
                                 int _linenumber) : CCompileTreeNode(_codeblock, _link,
                                                                     eCondBranch, _linenumber) {
}

int CCondBranchNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	// -- left child is if the stacktop contains a 'true' value
	size += PushInstruction(countonly, instrptr, OP_BranchFalse, DBG_instr);
	// -- cache the current intrptr, because we'll need to how far to
	// -- jump, after we've evaluated the left child
	// -- push a placeholder in the meantime
	unsigned int* branchwordcount = instrptr;
	unsigned int empty = 0;
	size += PushInstructionRaw(countonly, instrptr, (void*)&empty, 1, DBG_NULL,
							   "placeholder for branch");

	// -- if we have a left child, this is the 'true' tree
	if(leftchild) {
		int cursize = size;
		size += leftchild->Eval(instrptr, TYPE_void, countonly);

		// -- the size of the leftchild is how many instructions to jump, should the
		// -- branch condition be false - but add two, since the end of the 'true'
		// -- tree will have to jump the 'false' section
        if(!countonly) {
		    int jumpcount = size - cursize;
		    if (rightchild)
			    jumpcount += 2;
		    *branchwordcount = jumpcount;
        }
	}

	// -- the right tree is the 'false' tree
	if(rightchild) {
		// -- start with adding a branch at the end of the 'true' section
		size += PushInstruction(countonly, instrptr, OP_Branch, DBG_instr);
		branchwordcount = instrptr;
		size += PushInstructionRaw(countonly, instrptr, (void*)&empty, 1, DBG_NULL,
								   "placeholder for branch");

		// now evaluate the right child, tracking it's size
		int cursize = size;
		size += rightchild->Eval(instrptr, TYPE_void, countonly);

		// fill in the jumpcount
        if(!countonly) {
		    int jumpcount = size - cursize;
		    *branchwordcount = jumpcount;
        }
	}

	return size;
}

// ------------------------------------------------------------------------------------------------
CWhileLoopNode::CWhileLoopNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber) :
                               CCompileTreeNode(_codeblock, _link, eWhileLoop, _linenumber) {
}

int CWhileLoopNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	// -- ensure we have a left child
	if(!leftchild) {
		printf("Error - CWhileLoopNode with no left child\n");
		return 0;
	}

	// -- ensure we have a left child
	if(!rightchild) {
		printf("Error - CWhileLoopNode with no right child\n");
		return 0;
	}

	// the instruction at the start of the leftchild is where we begin each loop
	// -- evaluate the left child, which is the condition
	size += leftchild->Eval(instrptr, TYPE_bool, countonly);

	// -- add a BranchFalse here, to skip the body of the while loop
	size += PushInstruction(countonly, instrptr, OP_BranchFalse, DBG_instr);
	unsigned int* branchwordcount = instrptr;
	unsigned int empty = 0;
	size += PushInstructionRaw(countonly, instrptr, (void*)&empty, 1, DBG_NULL,
							   "placeholder for branch");
	int cursize = size;
	// -- evaluate the right child, which is the body of the while loop
	size += rightchild->Eval(instrptr, TYPE_void, countonly);

	// -- after the body of the while loop has been executed, we want to jump back
	// -- to the top and evaluate the condition again
	int jumpcount = -(size + 2);  // note + 2 is to account for the actual jump itself
	size += PushInstruction(countonly, instrptr, OP_Branch, DBG_instr);
	size += PushInstruction(countonly, instrptr, (unsigned int)jumpcount, DBG_NULL);


	// fill in the top jumpcount, which is to skip the while loop body if the condition is false
    if(!countonly) {
	    jumpcount = size - cursize;
	    *branchwordcount = jumpcount;
    }

	return size;
}

// ------------------------------------------------------------------------------------------------
CParenOpenNode::CParenOpenNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber) :
                               CCompileTreeNode(_codeblock, _link, eWhileLoop, _linenumber) {
}

int CParenOpenNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	return size;
}

// ------------------------------------------------------------------------------------------------
CFuncDeclNode::CFuncDeclNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                             const char* _funcname, int _length, const char* _funcns,
                             int _funcnslength) :
                             CCompileTreeNode(_codeblock, _link, eFuncDecl, _linenumber) {
    SafeStrcpy(funcname, _funcname, _length + 1);
    SafeStrcpy(funcnamespace, _funcns, _funcnslength + 1);

    int stacktopdummy = 0;
    CObjectEntry* dummy = NULL;
    functionentry = codeblock->smFuncDefinitionStack->GetTop(dummy, stacktopdummy);
}

int CFuncDeclNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

    // -- get the function entry
	unsigned int funchash = Hash(funcname);

    // -- if we're using a namespace, find the function entry from there
    unsigned int funcnshash = funcnamespace[0] != '\0' ? Hash(funcnamespace) : 0;
    tFuncTable* functable = NULL;
    if(funcnshash != 0) {
        CNamespace* nsentry = CNamespace::FindOrCreateNamespace(funcnamespace, true);
        if(!nsentry) {
            ScriptAssert_(0, codeblock->GetFileName(), linenumber,
                          "Error - Failed to find/create Namespace: %s\n", funcnamespace);
            return 0;
        }
        functable = nsentry->GetFuncTable();
    }
    else {
	    functable = GetGlobalNamespace()->GetFuncTable();
    }

	CFunctionEntry* fe = functable->FindItem(funchash);
	if(!fe) {
		ScriptAssert_(0, codeblock->GetFileName(), linenumber,  "Error - undefined function: %s\n",
                     funcname);
		return 0;
	}

    // -- set the current function definition
    codeblock->smFuncDefinitionStack->Push(fe, NULL, 0);

	EFunctionType functype = fe->GetType();
    eVarType returntype = fe->GetReturnType();

    // -- recreate the function entry - first the instruction 
    size += PushInstruction(countonly, instrptr, OP_FuncDecl, DBG_instr);

    // -- function hash
    size += PushInstruction(countonly, instrptr, fe->GetHash(), DBG_func);

    // -- push the function namespace hash
    size += PushInstruction(countonly, instrptr, funcnshash, DBG_hash);

    // -- push the function offset placeholder
	unsigned int* funcoffset = instrptr;
	unsigned int empty = 0;
	size += PushInstructionRaw(countonly, instrptr, (void*)&empty, 1, DBG_NULL,
							   "placeholder for func offset");

    // -- function context - parameters + local vartable
    size += CompileFunctionContext(fe->GetContext(), instrptr, countonly);

    // -- need to complete the function declaration
    size += PushInstruction(countonly, instrptr, OP_FuncDeclEnd, DBG_instr);

    // -- we want to skip over the entire body, as it's not for immediate execution
    size += PushInstruction(countonly, instrptr, OP_Branch, DBG_instr);
	unsigned int* branchwordcount = instrptr;
	size += PushInstructionRaw(countonly, instrptr, (void*)&empty, 1, DBG_NULL,
							   "placeholder for branch");
    int cursize = size;

    // -- we're now at the start of the function body
    if(!countonly) {
        // -- fill in the missing offset
        unsigned int offset = codeblock->CalcOffset(instrptr);
        fe->SetCodeBlockOffset(codeblock, offset);
        *funcoffset = offset;
    }

    // -- before the function body, we need to dump out the dictionary of local vars
    size += CompileVarTable(fe->GetLocalVarTable(), instrptr, countonly);

    // -- compile the function body
    assert(leftchild != NULL);
    size += leftchild->Eval(instrptr, returntype, countonly);

    // -- fill in the jumpcount
    if(!countonly) {
        int jumpcount = size - cursize;
	    *branchwordcount = jumpcount;
    }

    // -- clear the current function definition
    CObjectEntry* dummy = NULL;
    codeblock->smFuncDefinitionStack->Pop(dummy);

	return size;
}

void CFuncDeclNode::Dump(char*& output, int& length) const
{
	sprintf_s(output, length, "type: %s, funcname: %s", gCompileNodeTypes[type], funcname);
	int debuglength = strlen(output);
	output += debuglength;
	length -= debuglength;
}

// ------------------------------------------------------------------------------------------------
CFuncCallNode::CFuncCallNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                             const char* _funcname, int _length, const char* _nsname,
                             int _nslength, bool _ismethod) :
                             CCompileTreeNode(_codeblock, _link, eFuncCall, _linenumber) {
    SafeStrcpy(funcname, _funcname, _length + 1);
    SafeStrcpy(nsname, _nsname, _nslength + 1);
    ismethod = _ismethod;
}

int CFuncCallNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

    // -- if this function is *not* a method call, we need to push a 0 objectID
    /*
    if(!ismethod) {
        unsigned int empty = 0;
        size += PushInstruction(countonly, instrptr, OP_Push, DBG_instr);
		size += PushInstructionRaw(countonly, instrptr, (void*)&empty, 1, DBG_NULL,
								   "func call with no object");
    }
    */

    // -- get the function/method hash
	unsigned int funchash = Hash(funcname);
	unsigned int nshash = Hash(nsname);

    // -- first we push the function to the call stack
    // -- for methods, we want to find the method searching from the top of the objects hierarchy
    if(ismethod) {
        size += PushInstruction(countonly, instrptr, OP_MethodCallArgs, DBG_instr);
        size += PushInstruction(countonly, instrptr, 0, DBG_nshash);
    }
    else {
        // -- if this isn't a method, but we specified a namespace, then it's a
        // -- method from a specific namespace in an object's hierarchy.
        // -- PushSelf, since this will have been called via NS::Func() instead of obj.Func();
        if(nshash != 0) {
            size += PushInstruction(countonly, instrptr, OP_PushSelf, DBG_self);
            size += PushInstruction(countonly, instrptr, OP_MethodCallArgs, DBG_instr);
            size += PushInstruction(countonly, instrptr, nshash, DBG_nshash);
        }
        else {
            size += PushInstruction(countonly, instrptr, OP_FuncCallArgs, DBG_instr);
            size += PushInstruction(countonly, instrptr, nshash, DBG_nshash);
        }
    }
    size += PushInstruction(countonly, instrptr, funchash, DBG_func);

    // -- then evaluate all the argument assignments
    size += leftchild->Eval(instrptr, TYPE_void, countonly);

    // -- then call the function
    size += PushInstruction(countonly, instrptr, OP_FuncCall, DBG_instr);

    // -- if we're not looking for a return value
    if(pushresult <= TYPE_void) {
        // -- all functions will return a value - by default, a "" for void functions
        size += PushInstruction(countonly, instrptr, OP_Pop, DBG_instr);
    }

	return size;
}

void CFuncCallNode::Dump(char*& output, int& length) const
{
	sprintf_s(output, length, "type: %s, funcname: %s", gCompileNodeTypes[type], funcname);
	int debuglength = strlen(output);
	output += debuglength;
	length -= debuglength;
}

// ------------------------------------------------------------------------------------------------
CFuncReturnNode::CFuncReturnNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link,
                                 int _linenumber) : CCompileTreeNode(_codeblock, _link,
                                                                     eFuncReturn, _linenumber) {

    int stacktopdummy = 0;
    CObjectEntry* dummy = NULL;
    functionentry = _codeblock->smFuncDefinitionStack->GetTop(dummy, stacktopdummy);
}

int CFuncReturnNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

    // -- get the context, which will contain the return parameter (type)..
    assert(functionentry != NULL);
    CFunctionContext* context = functionentry->GetContext();
    assert(context != NULL && context->GetParameterCount() > 0);
    CVariableEntry* returntype = context->GetParameter(0);

    // -- all functions are required to return a value, to keep the virtual machine consistent
    if(!leftchild) {
        ScriptAssert_(leftchild != NULL, codeblock->GetFileName(), linenumber,
                        "Error - CFuncReturnNode::Eval() - invalid return from function %s()\n",
                        functionentry->GetName());
        return 0;
    }

    if(returntype->GetType() <= TYPE_void)
        size += leftchild->Eval(instrptr, TYPE_int, countonly);
    else
        size += leftchild->Eval(instrptr, returntype->GetType(), countonly);

    // -- finally, issue the function return instruction
	size += PushInstruction(countonly, instrptr, OP_FuncReturn, DBG_instr);

	return size;
}

// ------------------------------------------------------------------------------------------------
CObjMethodNode::CObjMethodNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                               const char* _methodname, int _methodlength) :
                               CCompileTreeNode(_codeblock, _link, eObjMethod, _linenumber) {
	SafeStrcpy(methodname, _methodname, _methodlength + 1);
}

int CObjMethodNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	// -- ensure we have a left child
	if(!leftchild) {
		printf("Error - CObjMemberNode with no left child\n");
		return 0;
	}

  	// -- evaluate the left child, pushing the a result of TYPE_object
	size += leftchild->Eval(instrptr, TYPE_object, countonly);

    // -- evaluate the right child, which contains the function call node
    size += rightchild->Eval(instrptr, pushresult, countonly);

	return size;
}

void CObjMethodNode::Dump(char*& output, int& length) const
{
	sprintf_s(output, length, "type: %s, %s", gCompileNodeTypes[type], methodname);
	int debuglength = strlen(output);
	output += debuglength;
	length -= debuglength;
}

// ------------------------------------------------------------------------------------------------
CArrayHashNode::CArrayHashNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link,
                                 int _linenumber) : CCompileTreeNode(_codeblock, _link,
                                                                     eArrayHash, _linenumber) {
}

int CArrayHashNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

    if(!leftchild) {
        ScriptAssert_(leftchild != NULL, codeblock->GetFileName(), linenumber,
                        "Error - CArrayHashNode::Eval() - missing leftchild\n");
        return 0;
    }

    if(!rightchild) {
        ScriptAssert_(rightchild != NULL, codeblock->GetFileName(), linenumber,
                        "Error - CArrayHashNode::Eval() - missing rightchild\n");
        return 0;
    }

   	// -- evaluate the left child, which pushes the "current hash", TYPE_int
	size += leftchild->Eval(instrptr, TYPE_int, countonly);

   	// -- evaluate the right child, which pushes the next string to be hashed and appended
	size += rightchild->Eval(instrptr, TYPE_string, countonly);

    // -- push an OP_ArrayHash, pops the top two stack items, the first is a "hash in progress",
    // -- and the second is a string to continue to add to the hash value
    // -- pushes the int hash result back onto the stack
	size += PushInstruction(countonly, instrptr, OP_ArrayHash, DBG_instr);

	return size;
}

// ------------------------------------------------------------------------------------------------
CArrayVarDeclNode::CArrayVarDeclNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link,
                                     int _linenumber, eVarType _type) :
                                     CCompileTreeNode(_codeblock, _link,
                                                      eArrayVarDecl, _linenumber) {
    type = _type;
}

int CArrayVarDeclNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

    if(!leftchild) {
        ScriptAssert_(leftchild != NULL, codeblock->GetFileName(), linenumber,
                        "Error - CArrayHashNode::Eval() - missing leftchild\n");
        return 0;
    }

    if(!rightchild) {
        ScriptAssert_(rightchild != NULL, codeblock->GetFileName(), linenumber,
                        "Error - CArrayHashNode::Eval() - missing rightchild\n");
        return 0;
    }

   	// -- left child will have pushed the hashtable variable
	size += leftchild->Eval(instrptr, TYPE_hashtable, countonly);

   	// -- right child will contain the hash value for the entry we're declaring
	size += rightchild->Eval(instrptr, TYPE_int, countonly);

	size += PushInstruction(countonly, instrptr, OP_ArrayVarDecl, DBG_instr);
	size += PushInstruction(countonly, instrptr, type, DBG_vartype);

	return size;
}

// ------------------------------------------------------------------------------------------------
CSelfVarDeclNode::CSelfVarDeclNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link,
                                   int _linenumber, const char* _varname, int _varnamelength,
                                   eVarType _type) : CCompileTreeNode(_codeblock, _link,
                                                                      eSelfVarDecl, _linenumber) {
	SafeStrcpy(varname, _varname, _varnamelength + 1);
    type = _type;
}

int CSelfVarDeclNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	unsigned int varhash = Hash(varname);
	size += PushInstruction(countonly, instrptr, OP_SelfVarDecl, DBG_instr);
	size += PushInstruction(countonly, instrptr, varhash, DBG_var);
	size += PushInstruction(countonly, instrptr, type, DBG_vartype);

	return size;
}

// ------------------------------------------------------------------------------------------------
CScheduleNode::CScheduleNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                             const char* _funcname, int _funclength, int _delaytime) :
                             CCompileTreeNode(_codeblock, _link, eFuncCall, _linenumber) {
	SafeStrcpy(funcname, _funcname, _funclength + 1);
    delaytime = _delaytime;
};

int CScheduleNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

    // -- get the function hash
	unsigned int funchash = Hash(funcname);

	// -- ensure we have a left child
	if(!leftchild) {
		printf("Error - CScheduleNode with no left child\n");
		return 0;
	}

	// -- ensure we have a right child
	if(!rightchild) {
		printf("Error - CScheduleNode with no right child\n");
		return 0;
	}

    // -- evaluate the left child, to get the object ID
    size += leftchild->Eval(instrptr, TYPE_object, countonly);

    // -- push the instruction to begin the schedule call
    size += PushInstruction(countonly, instrptr, OP_ScheduleBegin, DBG_instr);

    // -- push the hash of the function we're scheduling
    size += PushInstruction(countonly, instrptr, funchash, DBG_hash);

    // -- push the delaytime
    size += PushInstruction(countonly, instrptr, delaytime, DBG_value);

    // -- evaluate the right child, tree of all parameters for the scheduled function call
    size += rightchild->Eval(instrptr, TYPE_void, countonly);

    // -- finalize the schedule call, which will push the schedule ID on the stack
    size += PushInstruction(countonly, instrptr, OP_ScheduleEnd, DBG_instr);

    // -- if we're not looking for a return value (e.g. not assigning this schedule call)
    if(pushresult <= TYPE_void) {
        // -- all functions will return a value - by default, a "" for void functions
        size += PushInstruction(countonly, instrptr, OP_Pop, DBG_instr);
    }

	return size;
}

void CScheduleNode::Dump(char*& output, int& length) const
{
	sprintf_s(output, length, "type: %s, funcname: %s", gCompileNodeTypes[type], funcname);
	int debuglength = strlen(output);
	output += debuglength;
	length -= debuglength;
}

CSchedParamNode::CSchedParamNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link, int _linenumber,
                                 int _paramindex) :
                                 CCompileTreeNode(_codeblock, _link, eFuncCall, _linenumber) {
    paramindex = _paramindex;
}

int CSchedParamNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

	// -- ensure we have a left child
	if(!leftchild) {
		printf("Error - CScheduleNode with no left child\n");
		return 0;
	}

    // -- evaluate the left child, resolving to the value of the parameter
    size += leftchild->Eval(instrptr, TYPE__resolve, countonly);

    // -- push the instruction to assign the parameter
    size += PushInstruction(countonly, instrptr, OP_ScheduleParam, DBG_instr);

    // -- push the index of the param to assign
    size += PushInstruction(countonly, instrptr, paramindex, DBG_value);

    return size;
};

// ------------------------------------------------------------------------------------------------
CCreateObjectNode::CCreateObjectNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link,
                                     int _linenumber, const char* _classname,
                                     unsigned int _classlength, const char* _objname,
                                     unsigned int _objlength) :
                                     CCompileTreeNode(_codeblock, _link, eCreateObject,
                                                      _linenumber) {
	SafeStrcpy(classname, _classname, _classlength + 1);
	SafeStrcpy(objectname, _objname, _objlength + 1);
}

int CCreateObjectNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

    // -- create the object by classname, objectname
	unsigned int classhash = Hash(classname);
    unsigned int objecthash = Hash(objectname);
	size += PushInstruction(countonly, instrptr, OP_CreateObject, DBG_instr);
	size += PushInstruction(countonly, instrptr, classhash, DBG_hash);
	size += PushInstruction(countonly, instrptr, objecthash, DBG_hash);

    // -- if we're not looking to assign the new object ID to anything, pop the stack
    if(pushresult <= TYPE_void) {
        size += PushInstruction(countonly, instrptr, OP_Pop, DBG_instr);
    }

	return size;
}

// ------------------------------------------------------------------------------------------------
CDestroyObjectNode::CDestroyObjectNode(CCodeBlock* _codeblock, CCompileTreeNode*& _link,
                                       int _linenumber) :
                                       CCompileTreeNode(_codeblock, _link, eDestroyObject,
                                                        _linenumber) {
}

int CDestroyObjectNode::Eval(unsigned int*& instrptr, eVarType pushresult, bool countonly) const {

	DebugEvaluateNode(*this, countonly, instrptr);
	int size = 0;

  	// -- evaluate the left child, pushing the a result of TYPE_object
	size += leftchild->Eval(instrptr, TYPE_object, countonly);

    // -- create the object by classname, objectname
	size += PushInstruction(countonly, instrptr, OP_DestroyObject, DBG_instr);

	return size;
}

// ------------------------------------------------------------------------------------------------
// CodeBlock implementation
CCodeBlock::CCodeBlock(const char* _filename) {
    instrblock = NULL;
    instrcount = 0;

    smFuncDefinitionStack = new CFunctionCallStack(32);
    smCurrentGlobalVarTable = new tVarTable(kLocalVarTableSize);
    functionlist = new tFuncTable(kLocalFuncTableSize);

    // -- add to the resident list of codeblocks, if a name was given
    filename[0] = '\0';
    filenamehash = 0;
    if(_filename && _filename[0]) {
        SafeStrcpy(filename, _filename, kMaxNameLength);
        filenamehash = Hash(filename);
        gCodeBlockList->AddItem(*this, filenamehash);
    }

    // -- keep track of the linenumber offsets
    linenumberindex = 0;
    linenumbercount = 0;
    linenumbers = NULL;
}

CCodeBlock::~CCodeBlock() {
	if(instrblock)
		delete [] instrblock;
    smCurrentGlobalVarTable->DestroyAll();
    delete smCurrentGlobalVarTable;
    functionlist->DestroyAll();
    delete functionlist;

    if(linenumbers)
        delete [] linenumbers;
}

int CCodeBlock::CalcInstrCount(const CCompileTreeNode& root) {

	// -- the root is always a NOP, which will loop through and eval its siblings
	unsigned int* instrptr = NULL;
    int instrcount = 0;

    // -- add the size needed to store this block's global variables
    instrcount += CompileVarTable(smCurrentGlobalVarTable, instrptr, true);

    // -- run through the tree, calculating the size needed to contain the compiled code
	instrcount += root.Eval(instrptr, TYPE_void, true);

    // -- add one to account for the OP_EOF added to the end of every code block
    ++instrcount;

    return instrcount;
}

// ------------------------------------------------------------------------------------------------
bool CCodeBlock::CompileTree(const CCompileTreeNode& root) {

	// -- the root is always a NOP, which will loop through and eval its siblings
	unsigned int* instrptr = instrblock;

    // -- write out the instructions to populate the global variables needed
    CompileVarTable(smCurrentGlobalVarTable, instrptr, false);

    // -- compile the tree
	root.Eval(instrptr, TYPE_void, false);

	// -- push the specific operation to be performed
	PushInstruction(false, instrptr, OP_EOF, DBG_instr);

    unsigned int verifysize = (unsigned int)instrptr - (unsigned int)(instrblock);
	assert(instrcount == verifysize >> 2);

	return true;
}

// ------------------------------------------------------------------------------------------------
// -- debugging suppport

void SetDebugCodeBlock(bool torf) {
    gDebugCodeBlock = torf;
}

bool GetDebugCodeBlock() {
    return gDebugCodeBlock;
}

REGISTER_FUNCTION_P1(SetDebugCodeBlock, SetDebugCodeBlock, void, bool);

} // TinScript

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------