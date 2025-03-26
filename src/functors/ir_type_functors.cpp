
#include <exception>
#include <cassert>

#include <souffle/SouffleInterface.h>

using souffle::RamDomain;
using souffle::SymbolTable;
using souffle::RecordTable;

using std::string;

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
    Function,
    Integer,
    Null,
    Number,
    Object,
    String,
    Tuple,
    Undefined,
    Union,
    UserDefined,
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
    const RamDomain* sig1;
    const RamDomain* sig2;

    switch (type1[0]) {
        case Bottom:
        case Undefined:
        case Null:
        case Number:
        case Boolean:
        case String:
        case Void:
        case Any:
            return 0;
        case Tuple:
            return type_list_compare(symbolTable, recordTable, type1[1], type2[1]);
        case Array:
        case Object:
            return type_compare(symbolTable, recordTable, type1[1], type2[1]);
        case Union:
            return type_list_compare(symbolTable, recordTable, type1[1], type2[1]);
        case UserDefined:
            return strcmp(symbolTable->decode(type1[1]).c_str(), symbolTable->decode(type2[1]).c_str());
        case Function:
            sig1 = recordTable->unpack(type1[1], 2);
            sig2 = recordTable->unpack(type2[1], 2);
            cmp = type_compare(symbolTable, recordTable, sig1[0], sig2[0]);
            if (cmp == 0) {
                cmp = type_list_compare(symbolTable, recordTable, sig1[1], sig2[1]);
            }
            return cmp;
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

bool irTypeLatticeLte(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2) {
    if (arg1 == arg2) {
        return true;
    }

    const RamDomain* type1 = recordTable->unpack(arg1, maxArity);
    const RamDomain* type2 = recordTable->unpack(arg2, maxArity);

    if (type1[0] == Bottom) {
        return true;
    }
    if (type2[0] == Bottom) {
        return false;
    }
    if (type2[0] == Any) {
        return true;
    }
    if (type1[0] == Any) {
        return false;
    }

    if (type1[0] == Array && type2[0] == Array || type1[0] == Object && type2[0] == Object) {
        return irTypeLatticeLte(symbolTable, recordTable, type1[1], type2[1]);
    }

    if (type1[0] == Union && type2[0] == Union) {
        const RamDomain* set = recordTable->unpack(type1[1], maxArity);

        while (set) {
            RamDomain hd = set[0];
            RamDomain tl = set[1];
            if (!irTypeLatticeLte(symbolTable, recordTable, hd, arg2)) {
                return false;
            }
            set = recordTable->unpack(tl, maxArity);
        }
        return true;
    }

    if (type2[0] == Union) {
        const RamDomain* set = recordTable->unpack(type2[1], maxArity);

        while (set) {
            RamDomain hd = set[0];
            RamDomain tl = set[1];
            if (irTypeLatticeLte(symbolTable, recordTable, arg1, hd)) {
                return true;
            }
            set = recordTable->unpack(tl, maxArity);
        }
        return false;
    }

    if (type1[0] == Tuple && type2[0] == Tuple) {
        const RamDomain* set1 = recordTable->unpack(type1[1], maxArity);
        const RamDomain* set2 = recordTable->unpack(type1[1], maxArity);
        while (true) {
            if (!set1 && !set2) {
                return true;
            }
            if (!set1 || !set2) {
                return false;
            }

            RamDomain hd1 = set1[0];
            RamDomain tl1 = set1[1];
            RamDomain hd2 = set2[0];
            RamDomain tl2 = set2[1];

            if (!irTypeLatticeLte(symbolTable, recordTable, hd1, hd2)) {
                return false;
            }
            set1 = recordTable->unpack(tl1, maxArity);
            set2 = recordTable->unpack(tl1, maxArity);
        }
    }

    if (type1[0] == Function && type2[0] == Function) {
        const RamDomain* sig1 = recordTable->unpack(type1[1], 2);
        const RamDomain* sig2 = recordTable->unpack(type2[1], 2);
        if (irTypeLatticeLte(symbolTable, recordTable, sig1[0], sig2[0])) {
            return false;
        }

        const RamDomain* set1 = recordTable->unpack(sig1[1], maxArity);
        const RamDomain* set2 = recordTable->unpack(sig2[1], maxArity);

        while (true) {
            if (!set1 && !set2) {
                return true;
            }
            if (!set1 || !set2) {
                return false;
            }

            RamDomain hd1 = set1[0];
            RamDomain tl1 = set1[1];
            RamDomain hd2 = set2[0];
            RamDomain tl2 = set2[1];

            if (!irTypeLatticeLte(symbolTable, recordTable, hd1, hd2)) {
                return false;
            }
            set1 = recordTable->unpack(tl1, maxArity);
            set2 = recordTable->unpack(tl1, maxArity);
        }
    }

    if (type1[0] == UserDefined && type2[0] == UserDefined) {
        return symbolTable->decode(type1[1]) == symbolTable->decode(type2[1]);
    }

    if (type1[0] == Integer && type2[0] == Number) {
        return true;
    }

    return type1[0] == type2[0];
}

RamDomain type_set_remove(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId, RamDomain value);

RamDomain type_set_insert(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId, RamDomain value) {
    if (setId == 0) { // set is nil
        RamDomain newSet[2] = {value, 0};
        return recordTable->pack(newSet, 2);
    }
    const RamDomain* set = recordTable->unpack(setId, 2);
    while (set) {
        RamDomain hd = set[0];
        RamDomain tl = set[1];
        if (irTypeLatticeLte(symbolTable, recordTable, hd, value) || irTypeLatticeLte(symbolTable, recordTable, value, hd)) {
            RamDomain newValue = irTypeLub(symbolTable, recordTable, hd, value);
            RamDomain tmpSet = type_set_remove(symbolTable, recordTable, setId, hd);
            return type_set_insert(symbolTable, recordTable, tmpSet, newValue);
        }

        set = recordTable->unpack(tl, 2);
    }

    set = recordTable->unpack(setId, 2);
    RamDomain hd = set[0];
    RamDomain tl = set[1];
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

RamDomain type_set_remove(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain setId, RamDomain value) {
    if (setId == 0) {
        return 0;
    }
    const RamDomain* set = recordTable->unpack(setId, 2);
    RamDomain hd = set[0];
    RamDomain tl = set[1];

    if (type_compare(symbolTable, recordTable, hd, value) == 0) {
        return tl;
    }
    if (type_compare(symbolTable, recordTable, hd, value) > 0) {
        return setId;
    }
    RamDomain newTl = type_set_remove(symbolTable, recordTable, tl, value);
    RamDomain newSet[2] = {hd, newTl};
    return recordTable->pack(newSet, 2);
}

RamDomain irTypeListLub(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type1, RamDomain type2);

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

    if (t1[0] == Object && t2[0] == Object) {
        RamDomain newType[2] = {Object, irTypeLub(symbolTable, recordTable, t1[1], t2[1])};
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

    if (t1[0] == Function && t2[0] == Function) {
        const RamDomain* sig1 = recordTable->unpack(t1[1], 2);
        const RamDomain* sig2 = recordTable->unpack(t2[1], 2);
        RamDomain newSig[2] = {
            irTypeLub(symbolTable, recordTable, sig1[0], sig2[0]),
            irTypeListLub(symbolTable, recordTable, sig1[1], sig2[1])
        };
        RamDomain newType[2] = {Function, recordTable->pack(newSig, 2)};
        return recordTable->pack(newType, 2);
    }

    if (type_compare(symbolTable, recordTable, type1, type2) == 0) {
        return type1;
    }
    RamDomain newType[2] = {Union, type_set_create(symbolTable, recordTable, type1, type2)};
    return recordTable->pack(newType, 2);
}

RamDomain irTypeListLub(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain arg1, RamDomain arg2) {
    if (arg1 == nil) {
        return arg2;
    }
    if (arg2 == nil) {
        return arg1;
    }

    const RamDomain* list1 = recordTable->unpack(arg1, maxArity);
    const RamDomain* list2 = recordTable->unpack(arg2, maxArity);

    RamDomain newType[2];

    newType[0] = irTypeLub(symbolTable, recordTable, list1[0], list2[0]);
    newType[1] = irTypeListLub(symbolTable, recordTable, list1[1], list2[1]);

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

    if (type1[0] == Object && type2[0] == Object) {
        RamDomain newType[2] = {Object, irTypeGlb(symbolTable, recordTable, type1[1], type2[1])};
        return recordTable->pack(newType, 2);
    }

    if (type1[0] == Union && type2[0] == Union) {
        RamDomain glb = recordTable->pack({Bottom, nil});
        const RamDomain* set = recordTable->unpack(type1[1], 2);
        while (set) {
            const RamDomain hd = set[0];
            const RamDomain tl = set[1];

            RamDomain tmp = irTypeGlb(symbolTable, recordTable, hd, arg2);
            glb = irTypeLub(symbolTable, recordTable, tmp, glb);

            set = recordTable->unpack(tl, 2);
        }
        assert(irTypeLatticeLte(symbolTable, recordTable, glb, arg1));
        assert(irTypeLatticeLte(symbolTable, recordTable, glb, arg2));
        return glb;
    }

    if (type1[0] == Union) {
        if (type_set_contains(symbolTable, recordTable, type1[1], arg2)) {
            return arg2;
        }
        else {
            RamDomain glb = recordTable->pack({Bottom, nil});
            const RamDomain* set = recordTable->unpack(type1[1], 2);
            while (set) {
                const RamDomain hd = set[0];
                const RamDomain tl = set[1];

                RamDomain tmp = irTypeGlb(symbolTable, recordTable, hd, arg2);
                glb = irTypeLub(symbolTable, recordTable, tmp, glb);

                set = recordTable->unpack(tl, 2);
            }
            assert(irTypeLatticeLte(symbolTable, recordTable, glb, arg1));
            assert(irTypeLatticeLte(symbolTable, recordTable, glb, arg2));
            return glb;
        }
    }

    if (type2[0] == Union) {
        return irTypeGlb(symbolTable, recordTable, arg2, arg1);
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
        result += "," + joinTypeList(symbolTable, recordTable, type[1]);
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
        "Function",
        "Integer64",
        "Null",
        "Float64",
        "Object",
        "StaticString",
        "Tuple",
        "Undefined",
        "Union",
        "UserDefined",
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

    if (t[0] == Array || t[0] == Object) {
        const string base = typeNames[t[0]];
        const RamDomain elementString = irTypeToString(symbolTable, recordTable, t[1]);
        return symbolTable->encode(base + "<" + symbolTable->decode(elementString) + ">");
    }

    if (t[0] == UserDefined) {
        return symbolTable->encode("UserDefined<" + symbolTable->decode(t[1]) + ">");
    }
    if (t[0] == Function) {
        const RamDomain* sig = recordTable->unpack(t[1], 2);
        const RamDomain retString = irTypeToString(symbolTable, recordTable, sig[0]);
        const RamDomain argString = symbolTable->encode(joinTypeList(symbolTable, recordTable, sig[1]));
        return symbolTable->encode("[" + symbolTable->decode(argString) + "]" + "->" + symbolTable->decode(retString));
    }

    return symbolTable->encode(typeNames[t[0]]);
}

RamDomain getElementType(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type) {
    const RamDomain* t = recordTable->unpack(type, maxArity);
    RamDomain ret[2] = {Bottom, nil};

    switch (t[0]) {
        case Any:
            ret[0] = Any;
            break;
        case Boolean:
        case Bottom:
        case Function:
        case Integer:
        case Null:
        case Number:
        case String:
        case Tuple:
        case Undefined:
        case UserDefined:
        case Void:
            ret[0] = Bottom;
            break;
        case Object:
        case Array:
            return t[1];
        case Union: {
                const RamDomain* list = recordTable->unpack(t[1], maxArity);
                RamDomain lub = getElementType(symbolTable, recordTable, list[0]);
                while (list[1] != nil) {
                    list = recordTable->unpack(list[1], maxArity);
                    RamDomain element = getElementType(symbolTable, recordTable, list[0]);
                    lub = irTypeLub(symbolTable, recordTable, lub, element);
                }
                return lub;
            }
        default:
            throw std::runtime_error("Unexpected irType constructor");
    }
    return recordTable->pack(ret, 2);
}

RamDomain getFunctionRetType(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type) {
    const RamDomain* t = recordTable->unpack(type, maxArity);
    if (t[0] == Function) {
        const RamDomain* sig = recordTable->unpack(t[1], 2);
        return sig[0];
    }
    const RamDomain bottom[2] = {Bottom, nil};
    return recordTable->pack(bottom, 2);
}

RamDomain getFunctionParamType(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type, RamDomain index) {
    const RamDomain* t = recordTable->unpack(type, maxArity);
    if (t[0] != Function) {
        const RamDomain bottom[2] = {Bottom, nil};
        return recordTable->pack(bottom, 2);
    }
    const RamDomain* sig = recordTable->unpack(t[1], 2);
    if (sig[1] == nil) {
        const RamDomain bottom[2] = {Bottom, nil};
        return recordTable->pack(bottom, 2);
    }

    const RamDomain* args = recordTable->unpack(sig[1], maxArity);
    while (index--) {
        if (args[1] == nil) {
            const RamDomain bottom[2] = {Bottom, nil};
            return recordTable->pack(bottom, 2);
        }
        args = recordTable->unpack(args[1], maxArity);
    }
    return args[0];
}

RamDomain functionRetTemplate(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type) {
    const RamDomain sig[2] = { type, nil };
    const RamDomain f[2] = { Function, recordTable->pack(sig, 2) };
    return recordTable->pack(f, 2);
}

RamDomain functionParamTemplate(SymbolTable* symbolTable, RecordTable* recordTable, RamDomain type, RamDomain index) {
    const RamDomain bottom[2] = {Bottom, nil};
    RamDomain params[2] = { type, nil };
    while (index--) {
        params[1] = recordTable->pack(params, 2);
        params[0] = recordTable->pack(bottom, 2);
    }

    const RamDomain sig[2] = { recordTable->pack(bottom, 2), recordTable->pack(params, 2) };

    const RamDomain f[2] = { Function, recordTable->pack(sig, 2) };
    return recordTable->pack(f, 2);
}
