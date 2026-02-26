var wasm;

async function init() {
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;

  wasm = await WebAssembly.instantiateStreaming(fetch("./sudoku.wasm"), {
    env: {  },
  });
  memoryBuffer = wasm.instance.exports.memory.buffer;

  // document.onmousedown = (e) => {
  //   let x = e.pageX, y = e.pageY;
  //   wasm.instance.exports.on_mouse_down(x, y);
  // };

  // wasm.instance.exports.game_init(canvas.width, canvas.height, window.devicePixelRatio );
  window.requestAnimationFrame(loop);
}
init();
