
#include <exception>
#include <cassert>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>

#include <souffle/SouffleInterface.h>

using std::string;
using namespace std; 

#include "ir_type_functors.hpp"

extern "C" {
    RamDomain unaryOpOnType(SymbolTable* st, RecordTable* recordTable, RamDomain fullType, RamDomain op);
    RamDomain binaryOpOnTypes(SymbolTable* st, RecordTable* recordTable, RamDomain fullType1, RamDomain fullType2, RamDomain op);
    int stof(const char * str);
}

int stof(const char * str)
{
    return std::stof(str);
}

RamDomain negateType(RamDomain type) {
    if(type == UnsignedInteger)
    {
        return Integer;
    }
    return type;
}

bool isNumerical(RamDomain type) {
        // return type == Undefined || type == UnsignedInteger || type == Integer || type == Number || type == Bottom;
        return type == UnsignedInteger || type == Integer || type == Number || type == Bottom;
}

RamDomain unaryOpOnTypeAux(SymbolTable* st, RecordTable* recordTable, RamDomain fullType, RamDomain op)
{
    string opStr = st->decode(op);
    const RamDomain* t = recordTable->unpack(fullType, maxArity);
    RamDomain type = t[0];
 
    if (!isNumerical(type)) {
        throw std::runtime_error("Error: Unsupported type in unaryOpOnType: " + 
            st->decode(irTypeToString(st, recordTable, type)) + " " + opStr);
    }

    if(type == Bottom) {
        return Number;
    }

    if(opStr == "+" || opStr == "++")
    {
        return type;
    }
    if(opStr == "-")
    {
        return negateType(type);
    }
    if (opStr == "--")
    {
        if (type == Number) {
            return Number;
        }
        return Integer;
    }
    
    if(opStr == "~")
    {
        return Integer;
    }
    
    throw std::runtime_error("Error: Unsupported operator in unaryOpOnType: " + opStr);
}

RamDomain unaryOpOnType(SymbolTable* st, RecordTable* recordTable, RamDomain fullType, RamDomain op)
{
    RamDomain newType[2];
    newType[0] = unaryOpOnTypeAux(st, recordTable, fullType, op);
    return recordTable->pack(newType, maxArity);
}

RamDomain binaryOpOnTypesAux(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain fullType1, RamDomain fullType2, RamDomain op)
{
    const RamDomain* t1 = recordTable->unpack(fullType1, maxArity);
    RamDomain type1 = t1[0];
    const RamDomain* t2 = recordTable->unpack(fullType2, maxArity);
    RamDomain type2 = t2[0];
    string opStr = symbolTable->decode(op);
    
    if (!isNumerical(type1) || !isNumerical(type2)) {
        throw std::runtime_error("Error: Unsupported type in binaryOpOnTypes: " + 
            symbolTable->decode(irTypeToString(symbolTable, recordTable, fullType1)) + " " + opStr + " " 
                                     + symbolTable->decode(irTypeToString(symbolTable, recordTable, fullType2)));
    }

    if(type1 == Bottom || type2 == Bottom) {
        if (opStr == "+") // + is also a string op so one of the types can be string
        {
            return Bottom;
        }
        return Number;
    }

    if(opStr == "-")
    {
        if(type1 == Number || type2 == Number) {
            return Number;
        }
        return Integer;
    }
    
    if(opStr == "+" || opStr == "*")
    {
        if(type1 == UnsignedInteger && type2 == UnsignedInteger) {
            return UnsignedInteger;
        }
        if(type1 == Number || type2 == Number) {
            return Number;
        }
        return Integer;
    }
    
    if(opStr == "%")
    {
        if(type1 == UnsignedInteger && type2 != Number) {
            return UnsignedInteger;
        }
        if(type1 == Number || type2 == Number) {
            return Number;
        }
        return Integer;
    }
    
    if(opStr == "<<")
    {
        return Integer;
    }
    
    if(opStr == ">>")
    {
        if(type1 == UnsignedInteger) 
        {
            return UnsignedInteger;
        }
        return Integer;;
    }

    if(opStr == ">>>")
    {
        if(type1 == UnsignedInteger) 
        {
            return UnsignedInteger;
        }
        return Integer;
    }
    
    if(opStr == "/")
    {
        return Number;
    }

    if(opStr == "**")
    {
        if(type2 == UnsignedInteger) 
        {
            return type1;
        }

        return Number;
    }

    if(opStr == "^")
    {
        if(type1 == UnsignedInteger && type2 == UnsignedInteger) {
            return UnsignedInteger;
        }
        return Integer;
    }
    
    if(opStr == "|")
    {
        if(type1 == UnsignedInteger && type2 == UnsignedInteger) {
            return UnsignedInteger;
        }
        return Integer;
    }
    
    if(opStr == "&")
    {
        if(type1 == UnsignedInteger || type2 == UnsignedInteger) {
            return UnsignedInteger;
        }
        return Integer;
    }
    
 
    throw std::runtime_error("Error: Unsupported operator in binaryOpOnTypes: " +
        symbolTable->decode(irTypeToString(symbolTable, recordTable, type1)) + " " + opStr + " " 
                                 + symbolTable->decode(irTypeToString(symbolTable, recordTable, type2)));
}

RamDomain binaryOpOnTypes(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain fullType1, RamDomain fullType2, RamDomain op)
{
    RamDomain newType[2];
    newType[0] = binaryOpOnTypesAux(symbolTable, recordTable, fullType1, fullType2, op);
    return recordTable->pack(newType, maxArity);
}
