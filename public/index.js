// elements
outEl = document.getElementById("out")
exprEl = document.getElementById("expr")
submitEl = document.getElementById("submit")

// wasm module
Module = {
  print: (text) => {outEl.innerHTML += text + "</br>"},

  onRuntimeInitialized: () => {

    // wrap C method
    let runCode = Module.cwrap("run", null, ["string", "number"])

    // on click, run
    submitEl.addEventListener("click", (e) => {
      outEl.innerHTML = ""
      runCode(exprEl.value, exprEl.value.length)
    })
    
  }
}