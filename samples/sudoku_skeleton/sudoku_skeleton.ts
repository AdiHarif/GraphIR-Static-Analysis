
function partial_verify(board, x, y) {
	return board[x][y] == 0;
}

function solve(board, x, y) {
	if (board[x][y] != 0) {
		return solve(board, 0, 0);
	}
	board[x][y] = 1;
	if (partial_verify(board, x, y)) {
		if (solve(board, 0, 0)) {
			return true;
		}
	}
	board[x][y] = 2;
	return false;
}

function main() {
	let board = new Array(9);
	// for (let i = 0; i < 9; i++) {
	// 	board[i] = new Array(9);
	// }
	let i = 0;
	while (i < 9) {
		board[i] = new Array(9);
		i = i + 1;
	}
	solve(board, 0, 0);
}

main();
