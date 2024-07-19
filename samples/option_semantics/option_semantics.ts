
function foo() {
    const arr = new Array(10); // contains empty
    arr[1] = 0;
    if (arr[0]) {
        console.log("Test failed (1)");
        return;
    }
    if (arr[0] == 0) {
        console.log("Test failed (2)");
        return;
    }
    if (arr[1] != 0) {
        console.log("Test failed (3)");
        return;
    }
    console.log("Test passed");
}

foo();
