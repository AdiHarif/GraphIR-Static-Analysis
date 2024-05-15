
function arrays(x: number) {
    const a = [1, 2, 3];
    const b = [4, 5, 6];
    const c = [7, 8, 9];
    let d;
    if (x > 0) {
        d = a;
    }
    else {
        d = b;
    }
    d.push(10);
    return a;
}

arrays(1);
