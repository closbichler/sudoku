/*----------------------
 | Suoku WASM glue code
 |
 | Author: Clemens Losbichler
 | Dependencies: sudoku.wasm
 ----------------------*/

var wasmInstance, game = {
  board: null
}

function readCString(ptr) {
  const buffer = wasmInstance.exports.memory.buffer
  const bytes = new Uint8Array(buffer)

  let end = ptr
  while (bytes[end] !== 0) end++

  const slice = bytes.subarray(ptr, end)
  return new TextDecoder("utf-8").decode(slice)
}

function sudokuPtrToBoard(sudokuPtr) {
  let buffer = wasmInstance.exports.memory.buffer
  let board = {
    ptr: sudokuPtr,
    n: new Int32Array(buffer, sudokuPtr, 1)[0],
    b: new Int32Array(buffer, sudokuPtr + 4, 1)[0],
    field: []
  }

  let fieldPtr = new Uint32Array(buffer, sudokuPtr + 8, 1)[0]
  for (let i = 0; i < board.n; i++) {
    let rowPtr = new Uint32Array(buffer, fieldPtr + i * 4, 1)[0]
    let row = new Uint8Array(buffer, rowPtr, board.n)
    board.field.push(row);
  }

  return board
}

function printBoard(board) {
  let str = "";
  for (let i = 0; i < board.n; i++) {
    for (let j = 0; j < board.n; j++) {
      str += board.field[i][j] + " ";
    }
    str += "\n";
  }
  console.log(str);
}

function createSudokuHtml(board) {
  let container = document.getElementById("sudoku-container");
  container.innerHTML = "";
  for (let i = 0; i < board.n; i++) {
    let row = document.createElement("div");
    row.className = "sudoku-row";
    for (let j = 0; j < board.n; j++) {
      let cell = document.createElement("input");
      cell.type = "text";
      cell.className = "sudoku-cell";
      if (i % board.b == 0) cell.className += " sudoku-bt";
      if (j % board.b == 0) cell.className += " sudoku-bl";
      if (i == board.n - 1) cell.className += " sudoku-bb";
      if (j == board.n - 1) cell.className += " sudoku-br";
      cell.oninput = (e) => {
            cell.value = e.data;
            game.board.field[i][j] = parseInt(cell.value) || 0;
      }
      // TODO
      cell.onkeydown = (e) => {}
      cell.onkeyup = (e) => {}
      if (i == 0 && j == 0) cell.setAttribute("autofocus", true);
      row.appendChild(cell);
    }
    container.appendChild(row);
  }
}

function updateSudokuHtml(board) {
  let container = document.getElementById("sudoku-container");
  for (let i = 0; i < board.n; i++) {
    let row = container.children[i];
    for (let j = 0; j < board.n; j++) {
      let cell = row.children[j];
      cell.value = board.field[i][j] === 0 ? "" : board.field[i][j];
    }
  }
}

function rand(max) {
  return parseInt(Math.floor(Math.random() * max) + 1);
}

async function init() {
  const wasmModule = await WebAssembly.instantiateStreaming(fetch("./sudoku.wasm"), {
    env: { rand: rand },
  })
  wasmInstance = wasmModule.instance

  game.board = sudokuPtrToBoard(wasmInstance.exports.export_sudoku_example_easy(0));
  
  document.getElementById("solve-button").addEventListener("click", (e) => {
    let result = wasmInstance.exports.export_solve_sudoku(game.board.ptr)
    console.log(result)
    game.board = sudokuPtrToBoard(game.board.ptr);
    updateSudokuHtml(game.board);
  })

  document.getElementById("generate-button").addEventListener("click", (e) => {
    let size = game.board.n;
    let blockSize = game.board.b;
    let newBoardPtr = wasmInstance.exports.export_generate_sudoku(size, blockSize, blockSize * blockSize + size);

    // switch (type) {
    //   case "easy":               newBoardPtr = wasmInstance.exports.export_sudoku_example_easy(0); break;
    //   case "medium":             newBoardPtr = wasmInstance.exports.export_sudoku_example_medium(0); break;
    //   case "hard":               newBoardPtr = wasmInstance.exports.export_sudoku_example_hard(0); break;
    //   case "very-hard":          newBoardPtr = wasmInstance.exports.export_sudoku_example_very_hard(0); break;
    //   case "multiple-solutions": newBoardPtr = wasmInstance.exports.export_sudoku_example_multiple_solutions(0); break;
    //   default: console.log("Invalid type selected"); break;
    // }

    game.board = sudokuPtrToBoard(newBoardPtr);
    updateSudokuHtml(game.board);
  })

  document.getElementById("count-button").addEventListener("click", (e) => {
    let count = wasmInstance.exports.export_count_solutions(game.board.ptr)
    let text = count > 100000 ? "Number of solutions: > 100.000" : "Number of solutions: " + count;

    document.getElementById("solution-count").textContent = text;
  })

  document.getElementById("clear-button").addEventListener("click", (e) => {
    let size = game.board.n;
    let blockSize = game.board.b;
    game.board = sudokuPtrToBoard(wasmInstance.exports.export_sudoku_create_empty(size, blockSize));
    updateSudokuHtml(game.board);
  })

  document.getElementById("blank-board-button").addEventListener("click", (e) => {
    let size = document.getElementById("size-input").value;
    let blockSize = document.getElementById("blocksize-input").value;
    game.board = sudokuPtrToBoard(wasmInstance.exports.export_sudoku_create_empty(size, blockSize));
    createSudokuHtml(game.board);
    updateSudokuHtml(game.board);
  })

  createSudokuHtml(game.board);
  updateSudokuHtml(game.board);
}
init();
