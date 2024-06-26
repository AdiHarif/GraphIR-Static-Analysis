
#include <souffle/SouffleInterface.h>

using namespace souffle;
using std::string;

extern "C" {

RamDomain list_append(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain listId, RamDomain value) {
    if (listId == 0) { // list is nil
        RamDomain newList[2] = {value, 0};
        return recordTable->pack(newList, 2);
    }
    const RamDomain* list = recordTable->unpack(listId, 2);
    const RamDomain hd = list[0];
    const RamDomain tl = list[1];
    const RamDomain newList[2] = {hd, list_append(symbolTable, recordTable, tl, value)};
    return recordTable->pack(newList, 2);
}

RamDomain list_contains(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain listId, RamDomain value) {
    if (listId == 0) { // list is nil
        return 0;
    }
    const RamDomain* list = recordTable->unpack(listId, 2);
    const RamDomain hd = list[0];
    const RamDomain tl = list[1];
    if (hd == value) {
        return 1;
    }
    return list_contains(symbolTable, recordTable, tl, value);
}

RamDomain list_join_by_string(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain listId, RamDomain separator) {
    if (listId == 0) { // list1 is nil
        return symbolTable->encode("");
    }
    const RamDomain* list = recordTable->unpack(listId, 2);
    RamDomain hd = list[0];
    RamDomain tl = list[1];
    string out = symbolTable->decode(hd);
    while (tl != 0) {
        list = recordTable->unpack(tl, 2);
        hd = list[0];
        tl = list[1];
        out += symbolTable->decode(separator);
        out += symbolTable->decode(hd);
    }
    return symbolTable->encode(out);
}

}