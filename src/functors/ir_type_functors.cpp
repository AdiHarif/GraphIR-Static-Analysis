
#include <exception>

#include <souffle/SouffleInterface.h>

using namespace souffle;

extern "C" {

// enum irType {
//     Bottom = 0,
//     Undefined,
//     Number,
//     Boolean,
//     String,
//     Symbol,
//     Union,
//     Array,
//     Any
// };

enum irType {
    Any = 0,
    Array,
    Boolean,
    Bottom,
    Number,
    String,
    Symbol,
    Undefined,
    Union
};

const RamDomain nil = 0;

const size_t maxArity = 2;

int type_compare(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2);

RamDomain set_union(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId1, RamDomain setId2) {
    if (setId1 == nil) {
        return setId2;
    }
    if (setId2 == nil) {
        return setId1;
    }

    const RamDomain* set1 = recordTable->unpack(setId1, maxArity);
    const RamDomain* set2 = recordTable->unpack(setId2, maxArity);

    int cmp = type_compare(symbolTable, recordTable, set1[0], set2[0]);
    if (cmp == 0) {
        RamDomain newSet[2] = {set1[0], set_union(symbolTable, recordTable, set1[1], set2[1])};
        return recordTable->pack(newSet, 2);
    }

    if (cmp < 0) {
        RamDomain newSet[2] = {set1[0], set_union(symbolTable, recordTable, set1[1], setId2)};
        return recordTable->pack(newSet, 2);
    }

    if (cmp > 0) {
        RamDomain newSet[2] = {set2[0], set_union(symbolTable, recordTable, setId1, set2[1])};
        return recordTable->pack(newSet, 2);
    }
}

RamDomain set_intersect(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId1, RamDomain setId2) {
    if (setId1 == nil || setId2 == nil) {
        return nil;
    }

    const RamDomain* set1 = recordTable->unpack(setId1, 2);
    const RamDomain* set2 = recordTable->unpack(setId2, 2);

    int cmp = type_compare(symbolTable, recordTable, set1[0], set2[0]);
    if (cmp == 0) {
        RamDomain newSet[2] = {set1[0], set_intersect(symbolTable, recordTable, set1[1], set2[1])};
        return recordTable->pack(newSet, 2);
    }

    if (cmp < 0) {
        return set_intersect(symbolTable, recordTable, set1[1], setId2);
    }

    if (cmp > 0) {
        return set_intersect(symbolTable, recordTable, setId1, set2[1]);
    }
}

RamDomain set_create(SymbolTable* symbolTable, RecordTable* recordTable, const RamDomain type1, const RamDomain type2) {
    int cmp = type_compare(symbolTable, recordTable, type1, type2);
    if (cmp == 0) {
        return type1;
    }
    if (cmp > 0) {
        return set_create(symbolTable, recordTable, type2, type1);
    }

    RamDomain tail[2] = {type2, nil};
    RamDomain tlRecord = recordTable->pack(tail, 2);
    RamDomain newSet[2] = {type1, tlRecord};
    return recordTable->pack(newSet, 2);
}

int set_compare(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2) {
    if (arg1 == arg2) {
        return 0;
    }
    if (arg1 == nil) {
        return -1;
    }
    if (arg2 == nil) {
        return 1;
    }

    const RamDomain* type1 = recordTable->unpack(arg1, maxArity);
    const RamDomain* type2 = recordTable->unpack(arg2, maxArity);

    if (type1[0] != type2[0]) {
        return type1[0] - type2[0];
    }

    else {
        return set_compare(symbolTable, recordTable, type1[1], type2[1]);
    }
}

int type_compare(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2) {
    if (arg1 == arg2) {
        return 0;
    }

    const RamDomain* type1 = recordTable->unpack(arg1, maxArity);
    const RamDomain* type2 = recordTable->unpack(arg2, maxArity);

    if (type1[0] != type2[0]) {
        return type1[0] - type2[0];
    }

    switch (type1[0]) {
        case Bottom:
        case Undefined:
        case Number:
        case Boolean:
        case String:
        case Symbol:
        case Any:
            return 0;
        case Array:
            return type_compare(symbolTable, recordTable, type1[1], type2[1]);
        case Union:
            return set_compare(symbolTable, recordTable, type1[1], type2[1]);
        default:
            throw std::runtime_error("Unexpected irType constructor");
    }
}

RamDomain set_contains(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId, RamDomain value);
RamDomain set_insert(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId, RamDomain value);

RamDomain irTypeLub(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type1, RamDomain type2) {
    if (type1 == type2) {
        return type1;
    }

    const RamDomain* t1 = recordTable->unpack(type1, maxArity);
    const RamDomain* t2 = recordTable->unpack(type2, maxArity);

    if (t1[0] == Bottom) {
        return type2;
    }
    if (t2[0] == Bottom) {
        return type1;
    }
    if (t1[0] == Any) {
        return type1;
    }
    if (t2[0] == Any) {
        return type2;
    }

    if (t1[0] == Union && t2[0] == Union) {
        RamDomain newType[2] = {Union, set_union(symbolTable, recordTable, t1[1], t2[1])};
        return recordTable->pack(newType, 2);
    }
    if (t1[0] == Union) {
        if (set_contains(symbolTable, recordTable, t1[1], type2)) {
            return type1;
        }
        RamDomain newType[2] = {Union, set_insert(symbolTable, recordTable, t1[1], type2)};
        return recordTable->pack(newType, 2);
    }
    if (t2[0] == Union) {
        if (set_contains(symbolTable, recordTable, t2[1], type1)) {
            return type2;
        }
        RamDomain newType[2] = {Union, set_insert(symbolTable, recordTable, t2[1], type1)};
        return recordTable->pack(newType, 2);
    }
    if (type_compare(symbolTable, recordTable, type1, type2) == 0) {
        return type1;
    }
    RamDomain newType[2] = {Union, set_create(symbolTable, recordTable, type1, type2)};
    return recordTable->pack(newType, 2);
}


RamDomain irTypeGlb(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2) {

    const RamDomain* type1 = recordTable->unpack(arg1, maxArity);
    const RamDomain* type2 = recordTable->unpack(arg2, maxArity);

    if (type1[0] == Bottom || type2[0] == Bottom) {
        return arg1;
    }
    if (type1[0] == Any) {
        return arg2;
    }
    if (type2[0] == Any) {
        return arg1;
    }

    RamDomain newType[2];

    if (type1[0] == Union && type2[0] == Union) {
        newType[0] = Union;
        newType[1] = set_intersect(symbolTable, recordTable, type1[1], type2[1]);
        return recordTable->pack(newType, 2);
    }

    if (type1[0] == Union) {
        if (set_contains(symbolTable, recordTable, type1[1], type2[1])) {
            return arg2;
        }
        else {
            newType[0] = Bottom;
            return recordTable->pack(newType, 1);
        }
    }

    if (type2[0] == Union) {
        if (set_contains(symbolTable, recordTable, type2[1], type1[1])) {
            return arg1;
        }
        else {
            newType[0] = Bottom;
            return recordTable->pack(newType, 1);
        }
    }

    if (type_compare(symbolTable, recordTable, arg1, arg2) == 0) {
        return arg1;
    }

    newType[0] = Bottom;
    return recordTable->pack(newType, 1);
}

}