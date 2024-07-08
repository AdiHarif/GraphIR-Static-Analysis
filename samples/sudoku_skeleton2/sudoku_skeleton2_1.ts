
function partial_verify(board, x, y) {
	let base_x = Math.floor(x / 3) * 3;
	let base_y = Math.floor(y / 3) * 3;
	// for (let i = 0; i < 9; i++) {
	let i = 0;
	while (i < 9) {
		if (i != y && board[x][i] == board[x][y]) {
			return false;
		}
		if (i != x && board[i][y] == board[x][y]) {
			return false;
		}
		let pos_x = base_x + Math.floor(i / 3);
		let pos_y = base_y + (i % 3);
		if ((pos_x != x || pos_y != y) && board[pos_x][pos_y] == board[x][y]) {
			return false;
		}
		i = i + 1;
	}
	return true;
}

function solve(board, x, y) {
	let z = x * 9 + y + 1;
	if (z == 82) {
		return true;
	}
	if (board[x][y] != 0) {
		return solve(board, Math.floor(z / 9), z % 9);
	}
	// for (let i = 1; i <= 9; i++) {
	let i = 1;
	while (i <= 9) {
		board[x][y] = i;
		if (partial_verify(board, x, y)) {
			if (solve(board, Math.floor(z / 9), z % 9)) {
				return true;
			}
		}
		i = i + 1;
	}
	board[x][y] = 0;
	return false;
}

function _main() {
	let board = [
		[0, 6, 0, 0, 0, 0, 9, 0, 2],
		[0, 0, 0, 9, 2, 0, 0, 0, 0],
		[0, 8, 0, 0, 4, 0, 0, 1, 0],
		[0, 0, 0, 1, 0, 0, 0, 0, 3],
		[0, 0, 0, 0, 0, 0, 2, 9, 1],
		[0, 0, 0, 0, 0, 0, 4, 6, 0],
		[2, 0, 4, 3, 0, 8, 0, 0, 0],
		[3, 0, 0, 0, 0, 4, 0, 0, 0],
		[1, 0, 0, 0, 0, 0, 0, 0, 5]
	]
	solve(board, 0, 0);

	const solution = [
		[4, 6, 3, 8, 5, 1, 9, 7, 2],
		[7, 1, 5, 9, 2, 3, 6, 8, 4],
		[9, 8, 2, 7, 4, 3, 5, 1, 6],
		[5, 4, 9, 1, 7, 2, 6, 8, 3],
		[6, 3, 7, 4, 8, 5, 2, 9, 1],
		[8, 2, 1, 6, 3, 9, 4, 5, 7],
		[2, 5, 4, 3, 1, 8, 7, 6, 9],
		[3, 7, 6, 5, 9, 4, 1, 2, 8],
		[1, 9, 8, 2, 6, 7, 3, 4, 5]
	]
	// for (let i = 0; i < 9; i++) {
		// 	for (let j = 0; j < 9; j++) {
	let i = 0;
	while (i < 9) {
		let j = 0;
		while (j < 9) {
			if (board[i][j] != solution[i][j]) {
				console.log('Invalid solution');
				return;
			}
			j = j + 1;
		}
		i = i + 1;
	}
}

_main();
