var wasm;

async function init() {
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;

  wasm = await WebAssembly.instantiateStreaming(fetch("./sudoku.wasm"), {
    env: {  },
  });

  let sudoku = wasm.instance.exports.sudoku_create_empty(9, 3);
  wasm.instance.exports.sudoku_example_easy(sudoku);
  let sudoku_text_easy = wasm.instance.exports.sudoku_print(sudoku);
  console.log(sudoku_text_easy);
}
init();
