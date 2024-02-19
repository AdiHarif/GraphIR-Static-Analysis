
function far(x: number | string) {
    return;
}

function boo(x: any, y: any) {
    if (x == y) {
        far(1);
    }
    else {
        far(2);
    }
}

function zab(x: any) {
    return;
}

boo(1, 2);
boo('a', 3);
zab(1);
zab('a')