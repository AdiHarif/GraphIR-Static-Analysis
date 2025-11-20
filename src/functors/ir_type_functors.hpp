#include <exception>
#include <cassert>

#include <souffle/SouffleInterface.h>

using souffle::RamDomain;
using souffle::SymbolTable;
using souffle::RecordTable;

using std::string;

typedef RamDomain AdtTag;
typedef RamDomain AdtId;
typedef RamDomain AdtListId;
typedef RamDomain AdtSetId;

extern "C" {
    RamDomain irTypeLub(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2);
    RamDomain irTypeGlb(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2);
    RamDomain irTypeToString(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type);
    RamDomain getElementType(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type);
    RamDomain getFunctionRetType(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type);
    RamDomain getFunctionParamType(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type, RamDomain index);
    RamDomain functionRetTemplate(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type);
    RamDomain functionParamTemplate(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type, RamDomain index);
}

enum irType {
    Any = 0,
    Array,
    Boolean,
    Bottom,
    Function,
    Integer,
    Null,
    Number,
    Object,
    String,
    Tuple,
    Undefined,
    Union,
    UnsignedInteger,
    UserDefined,
    Void
};

const size_t maxArity = 2;
