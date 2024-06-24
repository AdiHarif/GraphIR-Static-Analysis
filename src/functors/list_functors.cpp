
#include <souffle/SouffleInterface.h>

using namespace souffle;

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

}