var wasm;

async function init() {
  wasm = await WebAssembly.instantiateStreaming(fetch("./sudoku.wasm"), {
    env: { },
  });
  
  console.log(wasm.instance.exports.get_kek());
  console.log(wasm.instance.exports.get_arr());

  // let sudoku = wasm.instance.exports.sudoku_create_empty(9, 3);
  // wasm.instance.exports.sudoku_example_easy(sudoku);
  // let sudoku_text_easy = wasm.instance.exports.sudoku_print(sudoku);
  // console.log(sudoku_text_easy);
}
init();
