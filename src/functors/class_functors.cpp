
#include <string>
#include <iostream>

#include <exception>

#include <souffle/SouffleInterface.h>

using souffle::RamDomain;
using souffle::SymbolTable;
using souffle::RecordTable;

using std::string;


const RamDomain nil = 0;

extern "C" {

RamDomain getClassName(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain fullName) {
    const string s = symbolTable->decode(fullName);
    size_t pos = s.find("::");
    if (pos == string::npos) {
        return nil;
    }
    return symbolTable->encode(s.substr(0, pos));
}

RamDomain getMethodName(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain fullName) {
    const string s = symbolTable->decode(fullName);
    size_t pos = s.find("::");
    if (pos == string::npos) {
        return fullName;
    }
    return symbolTable->encode(s.substr(pos + 2));
}

}
