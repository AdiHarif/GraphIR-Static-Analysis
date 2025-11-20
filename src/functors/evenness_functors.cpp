#include <exception>
#include <cassert>

#include <souffle/SouffleInterface.h>

using souffle::RamDomain;
using souffle::SymbolTable;
using souffle::RecordTable;

using std::string;
using namespace std;

extern "C" {
    RamDomain evennessTypeLub(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type1, RamDomain type2);
    RamDomain evennessTypeGlb(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type1, RamDomain type2);
    RamDomain binaryOpOnEvennessTypes(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type1, RamDomain type2, RamDomain op);
    RamDomain unaryOpOnEvennessType(SymbolTable* st, RecordTable* recordTable, RamDomain type, RamDomain op);
}

enum evennessType {
    Even = 0,
    EvennesAny,
    EvennesBottom,
    Odd,
};

const RamDomain nil = 0;

const size_t evennessMaxArity = 0;

RamDomain evennessTypeLub(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type1, RamDomain type2)
{
    if (type1 == type2) {
        return type1;
    }

    if (type1 == EvennesBottom) {
        return type2;
    }
    if (type2 == EvennesBottom) {
        return type1;
    }
    if (type1 == EvennesAny) {
        return type1;
    }
    if (type2 == EvennesAny) {
        return type2;
    }

    return EvennesAny;
}

RamDomain evennessTypeGlb(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type1, RamDomain type2)
{
    if (type1 == type2) {
        return type1;
    }

    if (type1 == EvennesBottom) {
        return type1;
    }
    if (type2 == EvennesBottom) {
        return type2;
    }
    if (type1 == EvennesAny) {
        return type2;
    }
    if (type2 == EvennesAny) {
        return type1;
    }

    return EvennesBottom;
}

RamDomain binaryOpOnEvennessTypes(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type1, RamDomain type2, RamDomain op)
{
    string opStr = symbolTable->decode(op);

    if(opStr == "*")
    {
        if(type1 == Even || type2 == Even) {
            return Even;
        }
        if(type1 == EvennesAny || type2 == EvennesAny) {
            return EvennesAny;
        }
        if(type1 == EvennesBottom || type2 == EvennesBottom) {
            return EvennesBottom;
        }
        return Odd;
    }
    if(opStr == "+" || opStr == "-")
    {
        if((type1 == Even && type2 == Even) || (type1 == Odd && type2 == Odd)) {
            return Even;
        }
        if(type1 == EvennesAny || type2 == EvennesAny) {
            return EvennesAny;
        }
        if(type1 == EvennesBottom || type2 == EvennesBottom) {
            return EvennesBottom;
        }
        return Odd;
    }

    return EvennesAny;
}

RamDomain unaryOpOnEvennessType(SymbolTable* st, RecordTable* recordTable, RamDomain type, RamDomain op)
{
    string opStr = st->decode(op);

    if(opStr == "++" || opStr == "--")
    {
        return type == Even ? Odd : Even;
    }
    if(opStr == "~")
    {
        return EvennesAny;
    }
    return type;
}