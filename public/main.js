function readCString(instance, ptr) {
  const memory = instance.exports.memory;
  const bytes = new Uint8Array(memory.buffer);

  let end = ptr;
  while (bytes[end] !== 0) end++;

  const slice = bytes.subarray(ptr, end);
  return new TextDecoder("utf-8").decode(slice);
}

async function init() {
  const wasmModule = await WebAssembly.instantiateStreaming(fetch("./sudoku.wasm"), {
    env: { },
  })
  const instance = wasmModule.instance
  const memory = instance.exports.memory

  // examples
  console.log(memory)

  let stringPtr = instance.exports.get_kek()
  console.log(readCString(instance, stringPtr))
 
  let arrPtr = instance.exports.get_arr()
  console.log(new Int32Array(memory.buffer, arrPtr, 3))
}
init();
