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
// TinParse.h : Parsing methods for TinScript
// ------------------------------------------------------------------------------------------------

#ifndef __TINPARSE_H
#define __TINPARSE_H

#include "string.h"

#include "TinTypes.h"
#include "TinScript.h"

namespace TinScript {

// ------------------------------------------------------------------------------------------------
// forward declarations

class CCompileTreeNode;
class CFunctionEntry;
class CFunctionContext;
class CCodeBlock;
struct tExprParenDepthStack;
class CVariableEntry;
class CFunctionEntry;

typedef CHashTable<CVariableEntry> tVarTable;
typedef CHashTable<CFunctionEntry> tFuncTable;

//eOpCode GetBinOpInstructionType(eBinaryOpType binoptype);
//int GetBinOpPrecedence(eBinaryOpType binoptype);

// ------------------------------------------------------------------------------------------------
// -- tuple defining the token types

#define TokenTypeTuple \
	TokenTypeEntry(NULL)			\
	TokenTypeEntry(STRING)			\
	TokenTypeEntry(BINOP)			\
	TokenTypeEntry(ASSOP)			\
	TokenTypeEntry(UNARY)			\
	TokenTypeEntry(IDENTIFIER)		\
	TokenTypeEntry(KEYWORD)			\
	TokenTypeEntry(REGTYPE)			\
	TokenTypeEntry(EXPECTED)		\
	TokenTypeEntry(FLOAT)			\
	TokenTypeEntry(INTEGER)			\
	TokenTypeEntry(BOOL)			\
	TokenTypeEntry(NAMESPACE)		\
	TokenTypeEntry(PAREN_OPEN)		\
	TokenTypeEntry(PAREN_CLOSE)		\
	TokenTypeEntry(COMMA)			\
	TokenTypeEntry(SEMICOLON)		\
	TokenTypeEntry(PERIOD)			\
	TokenTypeEntry(BRACE_OPEN)		\
	TokenTypeEntry(BRACE_CLOSE)		\
	TokenTypeEntry(SQUARE_OPEN)		\
	TokenTypeEntry(SQUARE_CLOSE)	\
	TokenTypeEntry(EOF)				\
	TokenTypeEntry(ERROR)			\

enum eTokenType {
	#define TokenTypeEntry(a) TOKEN_##a,
	TokenTypeTuple
	#undef TokenTypeEntry
};

// ------------------------------------------------------------------------------------------------
// -- tuple defining the binary operators

// -- note:  the two-char tokens must be listed before the single-chars
// -- in addition, the name of the entry (first column) must be the same
// -- as the OP_name of the operation
// -- precedence - the higher the number, the higher up the tree
// -- which means it will be evaluated later, having a lower actual precedence.
// -- http://msdn.microsoft.com/en-us/library/126fe14k(v=VS.71).aspx
#define BinaryOperatorTuple \
	BinaryOperatorEntry(NULL,					"NULL",     0)			\
	BinaryOperatorEntry(BooleanAnd, 		    "&&",       90)			\
	BinaryOperatorEntry(BooleanOr, 		        "||",       90)			\
	BinaryOperatorEntry(CompareEqual,		    "==",       50)			\
	BinaryOperatorEntry(CompareNotEqual,	    "!=",       50)			\
	BinaryOperatorEntry(CompareLessEqual,		"<=",       40)			\
	BinaryOperatorEntry(CompareGreaterEqual,	">=",       40)			\
	BinaryOperatorEntry(BitLeftShift,			"<<",       30)			\
	BinaryOperatorEntry(BitRightShift,			">>",       30)			\
	BinaryOperatorEntry(CompareLess,			"<",        40)			\
	BinaryOperatorEntry(CompareGreater,			">",        40)			\
	BinaryOperatorEntry(Add,					"+",        20)			\
	BinaryOperatorEntry(Sub,					"-",        20)			\
	BinaryOperatorEntry(Mult,					"*",        10)			\
	BinaryOperatorEntry(Div,					"/",        10)			\
	BinaryOperatorEntry(Mod,					"%",        10)			\
	BinaryOperatorEntry(BitAnd, 		        "&",        60)			\
	BinaryOperatorEntry(BitXor,		            "^",        70)			\
	BinaryOperatorEntry(BitOr, 		            "|",        80)			\

enum eBinaryOpType {
	#define BinaryOperatorEntry(a, b, c) BINOP_##a,
	BinaryOperatorTuple
	#undef BinaryOperatorEntry
	BINOP_COUNT
};

#define AssignOperatorTuple \
	AssignOperatorEntry(NULL,					"NULL")			\
	AssignOperatorEntry(AssignAdd,				"+=")			\
	AssignOperatorEntry(AssignSub,				"-=")			\
	AssignOperatorEntry(AssignMult,				"*=")			\
	AssignOperatorEntry(AssignDiv,				"/=")			\
	AssignOperatorEntry(AssignMod,				"%=")			\
	AssignOperatorEntry(AssignLeftShift,    	"<<=")			\
	AssignOperatorEntry(AssignRightShift,    	">>=")			\
	AssignOperatorEntry(AssignBitAnd,    	    "&=")			\
	AssignOperatorEntry(AssignBitOr,    	    "|=")			\
	AssignOperatorEntry(AssignBitXor,    	    "^=")			\
	AssignOperatorEntry(Assign,					"=")			\

enum eAssignOpType {
	#define AssignOperatorEntry(a, b) ASSOP_##a,
	AssignOperatorTuple
	#undef AssignOperatorEntry
	ASSOP_COUNT
};

#define UnaryOperatorTuple \
    UnaryOperatorEntry(NULL,                    "NULL")        \
    UnaryOperatorEntry(UnaryPreInc,             "++")          \
    UnaryOperatorEntry(UnaryPreDec,             "--")          \
    UnaryOperatorEntry(UnaryBitInvert,          "~")           \
    UnaryOperatorEntry(UnaryNot,                "!")           \
    UnaryOperatorEntry(UnaryNeg,                "-")           \
    UnaryOperatorEntry(UnaryPos,                "+")           \

enum eUnaryOpType {
	#define UnaryOperatorEntry(a, b) UNARY_##a,
	UnaryOperatorTuple
	#undef UnaryOperatorEntry
	UNARY_COUNT
};

// ------------------------------------------------------------------------------------------------
// -- tuple defining the reserved keywords

#define ReservedKeywordTuple \
	ReservedKeywordEntry(NULL)		\
	ReservedKeywordEntry(if)		\
	ReservedKeywordEntry(else)		\
	ReservedKeywordEntry(while)		\
	ReservedKeywordEntry(break)		\
	ReservedKeywordEntry(continue)	\
	ReservedKeywordEntry(for)		\
	ReservedKeywordEntry(return)	\
	ReservedKeywordEntry(schedule)	\
	ReservedKeywordEntry(create)   	\
	ReservedKeywordEntry(destroy) 	\
	ReservedKeywordEntry(self)   	\

enum eReservedKeyword {
	#define ReservedKeywordEntry(a) KEYWORD_##a,
	ReservedKeywordTuple
	#undef ReservedKeywordEntry

	KEYWORD_COUNT
};

// ------------------------------------------------------------------------------------------------
// struct for reading tokens
struct tReadToken
{
	tReadToken(const tReadToken& _in) {
		inbufptr = _in.inbufptr;
		tokenptr = _in.tokenptr;
		length = _in.length;
        type = _in.type;
		linenumber = _in.linenumber;
	}

	tReadToken(const char* _inbufptr, int _linenumber) {
		inbufptr = _inbufptr;
		tokenptr = NULL;
		length = 0;
		type = TOKEN_NULL;
		linenumber = _linenumber;
	}

	const char* inbufptr;
	const char* tokenptr;
	int32 length;
	eTokenType type;
	int32 linenumber;

	private:
		tReadToken() { }
};

// ------------------------------------------------------------------------------------------------
const char* TokenPrint(tReadToken& token);
const char* SkipWhiteSpace(const char* inbuf, int& linenumber);
bool IsIdentifierChar(const char c, bool allownumerics);

bool GetToken(tReadToken& token, bool expectunaryop = false);
const char* GetToken(const char*& inbuf, int& length, eTokenType& type,	const char* expectedtoken,
                     int& linenumber, bool expectunaryop);

const char* ReadFileAllocBuf(const char* filename);

// ------------------------------------------------------------------------------------------------
CCompileTreeNode*& AppendToRoot(CCompileTreeNode& root);
bool ParseStatementBlock(CCodeBlock* codeblock, CCompileTreeNode*& root, tReadToken& filebuf,
                         bool requiresbraceclose);
CCodeBlock* ParseFile(CScriptContext* script_context, const char* filename);
CCodeBlock* ParseText(CScriptContext* script_context, const char* filename, const char* filebuf);

bool SaveBinary(CCodeBlock* codeblock, const char* binfilename);
CCodeBlock* LoadBinary(CScriptContext* script_context, const char* binfilename);

bool TryParseVarDeclaration(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link);
bool TryParseStatement(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link);
bool TryParseExpression(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link);
bool TryParseIfStatement(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link);
bool TryParseWhileLoop(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link);
bool TryParseForLoop(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link);
bool TryParseFuncDefinition(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link);
bool TryParseFuncCall(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link,
                      bool ismethod);
bool TryParseArrayHash(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link);
bool TryParseSchedule(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link);
bool TryParseCreateObject(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link);
bool TryParseDestroyObject(CCodeBlock* codeblock, tReadToken& filebuf, CCompileTreeNode*& link);

// ------------------------------------------------------------------------------------------------
CVariableEntry* AddVariable(CScriptContext* script_context, tVarTable* curglobalvartable,
                            CFunctionEntry* curfuncdefinition, const char* varname,
                            unsigned int varhash, eVarType vartype);
CVariableEntry* GetVariable(CScriptContext* script_context, tVarTable* globalVarTable,
                            unsigned int nshash, unsigned int funchash,
                            unsigned int varhash, unsigned int arrayvarhash);

CFunctionEntry* FuncDeclaration(CScriptContext* script_context, unsigned int namespacehash,
                                const char* funcname, unsigned int funchash, EFunctionType type);
CFunctionEntry* FuncDeclaration(CScriptContext* script_context, CNamespace* nsentry,
                                const char* funcname, unsigned int funchash, EFunctionType type);

tExprParenDepthStack* GetGlobalParenStack();

// ------------------------------------------------------------------------------------------------
// debugging methods
const char* GetAssOperatorString(eAssignOpType assop);

bool DumpFile(const char* filename);

void DumpTree(const char* root, int indent);
void DestroyTree(CCompileTreeNode* root);

int CalcVarTableSize(tVarTable* vartable);
void DumpVarTable(CObjectEntry* oe);
void DumpVarTable(CScriptContext* script_context, CObjectEntry* oe, const tVarTable* vartable);
void DumpFuncTable(CObjectEntry* oe);
void DumpFuncTable(CScriptContext* script_context, const tFuncTable* functable);

// ------------------------------------------------------------------------------------------------

}  // TinScript

#endif // __TINPARSE_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
