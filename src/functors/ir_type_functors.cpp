
#include <exception>

#include <souffle/SouffleInterface.h>

using souffle::RamDomain;
using souffle::SymbolTable;
using souffle::RecordTable;

using std::string;

extern "C" {
    RamDomain irTypeLub(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2);
    RamDomain irTypeGlb(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2);
    RamDomain irTypeToString(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type);
}

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
    // Function,
    Integer,
    Number,
    String,
    Symbol,
    Tuple,
    Undefined,
    Union,
    Void
};

const RamDomain nil = 0;

const size_t maxArity = 2;

int type_compare(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2);
int tag_compare(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2);

RamDomain type_set_union(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId1, RamDomain setId2) {
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
        RamDomain newSet[2] = {set1[0], type_set_union(symbolTable, recordTable, set1[1], set2[1])};
        return recordTable->pack(newSet, 2);
    }

    if (tag_compare(symbolTable, recordTable, set1[0], set2[0]) == 0) {
        RamDomain newSet[2] = {
            irTypeLub(symbolTable, recordTable, set1[0], set2[0]),
            type_set_union(symbolTable, recordTable, set1[1], set2[1])
        };
        return recordTable->pack(newSet, 2);
    }

    if (cmp < 0) {
        RamDomain newSet[2] = {set1[0], type_set_union(symbolTable, recordTable, set1[1], setId2)};
        return recordTable->pack(newSet, 2);
    }

    if (cmp > 0) {
        RamDomain newSet[2] = {set2[0], type_set_union(symbolTable, recordTable, setId1, set2[1])};
        return recordTable->pack(newSet, 2);
    }
}

RamDomain type_set_intersect(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId1, RamDomain setId2) {
    if (setId1 == nil || setId2 == nil) {
        return nil;
    }

    const RamDomain* set1 = recordTable->unpack(setId1, 2);
    const RamDomain* set2 = recordTable->unpack(setId2, 2);

    int cmp = type_compare(symbolTable, recordTable, set1[0], set2[0]);
    if (cmp == 0) {
        RamDomain newSet[2] = {set1[0], type_set_intersect(symbolTable, recordTable, set1[1], set2[1])};
        return recordTable->pack(newSet, 2);
    }

    if (cmp < 0) {
        return type_set_intersect(symbolTable, recordTable, set1[1], setId2);
    }

    if (cmp > 0) {
        return type_set_intersect(symbolTable, recordTable, setId1, set2[1]);
    }
}

RamDomain type_set_create(SymbolTable* symbolTable, RecordTable* recordTable, const RamDomain type1, const RamDomain type2) {
    int cmp = type_compare(symbolTable, recordTable, type1, type2);
    if (cmp == 0) {
        return type1;
    }
    if (cmp > 0) {
        return type_set_create(symbolTable, recordTable, type2, type1);
    }

    RamDomain tail[2] = {type2, nil};
    RamDomain tlRecord = recordTable->pack(tail, 2);
    RamDomain newSet[2] = {type1, tlRecord};
    return recordTable->pack(newSet, 2);
}

int type_list_compare(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2) {
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
        return type_list_compare(symbolTable, recordTable, type1[1], type2[1]);
    }
}

int tag_compare(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2) {
    if (arg1 == arg2) {
        return 0;
    }

    const RamDomain* type1 = recordTable->unpack(arg1, maxArity);
    const RamDomain* type2 = recordTable->unpack(arg2, maxArity);

    return type1[0] != type2[0];
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

    int cmp;

    switch (type1[0]) {
        case Bottom:
        case Undefined:
        case Number:
        case Boolean:
        case String:
        case Symbol:
        case Void:
        case Any:
            return 0;
        case Tuple:
            return type_list_compare(symbolTable, recordTable, type1[1], type2[1]);
        case Array:
            return type_compare(symbolTable, recordTable, type1[1], type2[1]);
        case Union:
            return type_list_compare(symbolTable, recordTable, type1[1], type2[1]);
        // case Function:
        //     cmp = type_list_compare(symbolTable, recordTable, type1[1], type2[1]);
        //     if (cmp == 0) {
        //         cmp = type_compare(symbolTable, recordTable, type1[2], type2[2]);
        //     }
        //     return cmp;
        default:
            throw std::runtime_error("Unexpected irType constructor");
    }
}

RamDomain type_set_contains(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId, RamDomain value) {
        if (setId == 0) { // set is nil
        return 0;
    }
    const RamDomain* set = recordTable->unpack(setId, 2);
    const RamDomain hd = set[0];
    const RamDomain tl = set[1];
    if (type_compare(symbolTable, recordTable, hd, value) == 0) {
        return 1;
    }
    if (type_compare(symbolTable, recordTable, hd, value) > 0) {
        return 0;
    }
    return type_set_contains(symbolTable, recordTable, tl, value);
}

RamDomain type_set_insert(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId, RamDomain value) {
    if (setId == 0) { // set is nil
        RamDomain newSet[2] = {value, 0};
        return recordTable->pack(newSet, 2);
    }
    const RamDomain* set = recordTable->unpack(setId, 2);
    const RamDomain hd = set[0];
    const RamDomain tl = set[1];
    if (type_compare(symbolTable, recordTable, hd, value) == 0) {
        return setId;
    }
    else if (tag_compare(symbolTable, recordTable, hd, value) == 0) {
        RamDomain newSet[2] = {
            irTypeLub(symbolTable, recordTable, hd, value),
            tl
        };
        return recordTable->pack(newSet, 2);
    }
    else if (type_compare(symbolTable, recordTable, hd, value) < 0) {
        RamDomain newSet[2] = {hd, type_set_insert(symbolTable, recordTable, tl, value)};
        return recordTable->pack(newSet, 2);
    } else {
        RamDomain newSet[2] = {value, setId};
        return recordTable->pack(newSet, 2);
    }
}

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

    if (t1[0] == Array && t2[0] == Array) {
        RamDomain newType[2] = {Array, irTypeLub(symbolTable, recordTable, t1[1], t2[1])};
        return recordTable->pack(newType, 2);
    }

    if (t1[0] == Union && t2[0] == Union) {
        RamDomain newType[2] = {Union, type_set_union(symbolTable, recordTable, t1[1], t2[1])};
        return recordTable->pack(newType, 2);
    }
    if (t1[0] == Union) {
        if (type_set_contains(symbolTable, recordTable, t1[1], type2)) {
            return type1;
        }
        RamDomain newType[2] = {Union, type_set_insert(symbolTable, recordTable, t1[1], type2)};
        return recordTable->pack(newType, 2);
    }
    if (t2[0] == Union) {
        if (type_set_contains(symbolTable, recordTable, t2[1], type1)) {
            return type2;
        }
        RamDomain newType[2] = {Union, type_set_insert(symbolTable, recordTable, t2[1], type1)};
        return recordTable->pack(newType, 2);
    }

    if (t1[0] == Integer && t2[0] == Number) {
        return type2;
    }
    if (t1[0] == Number && t2[0] == Integer) {
        return type1;
    }

    if (type_compare(symbolTable, recordTable, type1, type2) == 0) {
        return type1;
    }
    RamDomain newType[2] = {Union, type_set_create(symbolTable, recordTable, type1, type2)};
    return recordTable->pack(newType, 2);
}

RamDomain irTypeGlb(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2) {

    const RamDomain* type1 = recordTable->unpack(arg1, maxArity);
    const RamDomain* type2 = recordTable->unpack(arg2, maxArity);

    if (type1[0] == Bottom) {
        return arg1;
    }
    if (type2[0] == Bottom) {
        return arg2;
    }

    if (type1[0] == Any) {
        return arg2;
    }
    if (type2[0] == Any) {
        return arg1;
    }

    RamDomain newType[2];

    if (type1[0] == Array && type2[0] == Array) {
        RamDomain newType[2] = {Array, irTypeGlb(symbolTable, recordTable, type1[1], type2[1])};
        return recordTable->pack(newType, 2);
    }

    if (type1[0] == Union && type2[0] == Union) {
        newType[0] = Union;
        newType[1] = type_set_intersect(symbolTable, recordTable, type1[1], type2[1]);
        return recordTable->pack(newType, 2);
    }

    if (type1[0] == Union) {
        if (type_set_contains(symbolTable, recordTable, type1[1], arg2)) {
            return arg2;
        }
        else {
            newType[0] = Bottom;
            return recordTable->pack(newType, 2);
        }
    }

    if (type2[0] == Union) {
        if (type_set_contains(symbolTable, recordTable, type2[1], arg1)) {
            return arg1;
        }
        else {
            newType[0] = Bottom;
            return recordTable->pack(newType, 2);
        }
    }

    if (type1[0] == Integer && type2[0] == Number) {
        return arg1;
    }
    if (type1[0] == Number && type2[0] == Integer) {
        return arg2;
    }

    if (type_compare(symbolTable, recordTable, arg1, arg2) == 0) {
        return arg1;
    }

    newType[0] = Bottom;
    return recordTable->pack(newType, 2);
}

RamDomain irTypeToString(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type);

string joinTypeList(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain list, string sep=",") {
    if (list == nil) {
        return "";
    }

    const RamDomain* type = recordTable->unpack(list, maxArity);
    string result = symbolTable->decode(irTypeToString(symbolTable, recordTable, type[0]));
    if (type[1] != nil) {
        result += ", " + joinTypeList(symbolTable, recordTable, type[1]);
    }
    return result;
}

RamDomain irTypeToString(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type) {
    const RamDomain* t = recordTable->unpack(type, maxArity);
    static const string typeNames[] = {
        "Any",
        "DynamicArray",
        "Integer1",
        "Bottom",
        // "Function",
        "Float64",
        "StaticString",
        "Symbol",
        "Tuple",
        "Undefined",
        "Union",
        "Void"
    };

    if (t[0] == Union || t[0] == Tuple) {
        RamDomain tail = t[1];
        string result = t[0] == Union ? "Union<" : "[";
        while (tail != nil) {
            const RamDomain* type = recordTable->unpack(tail, maxArity);
            const RamDomain typeName = irTypeToString(symbolTable, recordTable, type[0]);
            result += symbolTable->decode(typeName);
            tail = type[1];
            if (tail != nil) {
                result += ",";
            }
        }
        result += t[0] == Union ? ">" : "]";
        return symbolTable->encode(result);
    }

    if (t[0] == Array) {
        const RamDomain elementString = irTypeToString(symbolTable, recordTable, t[1]);
        return symbolTable->encode("DynamicArray<" + symbolTable->decode(elementString) + ">");
    }

    // if (t[0] == Function) {
    //     const RamDomain argString = symbolTable->encode(joinTypeList(symbolTable, recordTable, t[1]));
    //     const RamDomain retString = irTypeToString(symbolTable, recordTable, t[2]);
    //     return symbolTable->encode("[" + symbolTable->decode(argString) + "]" + " -> " + symbolTable->decode(retString));
    // }

    return symbolTable->encode(typeNames[t[0]]);
}
