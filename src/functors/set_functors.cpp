
#include <souffle/SouffleInterface.h>

using namespace souffle;

extern "C" {

RamDomain set_insert(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId, RamDomain value) {
    if (setId == 0) { // set is nil
        RamDomain newSet[2] = {value, 0};
        return recordTable->pack(newSet, 2);
    }
    const RamDomain* set = recordTable->unpack(setId, 2);
    const RamDomain hd = set[0];
    const RamDomain tl = set[1];
    if (hd == value) {
        return setId;
    }
    else if (hd < value) {
        RamDomain newSet[2] = {hd, set_insert(symbolTable, recordTable, tl, value)};
        return recordTable->pack(newSet, 2);
    } else {
        RamDomain newSet[2] = {value, setId};
        return recordTable->pack(newSet, 2);
    }

}

RamDomain set_contains(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId, RamDomain value) {
    if (setId == 0) { // set is nil
        return 0;
    }
    const RamDomain* set = recordTable->unpack(setId, 2);
    const RamDomain hd = set[0];
    const RamDomain tl = set[1];
    if (hd == value) {
        return 1;
    }
    if (hd > value) {
        return 0;
    }
    return set_contains(symbolTable, recordTable, tl, value);
}

}