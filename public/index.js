// elements for output
const outEls = {
  code: document.getElementById("code"),
  tokens: document.getElementById("tokens"),
  ast: document.getElementById("ast"),
  execute: document.getElementById("execute")
}

// elements for input
const exprEl = document.getElementById("expr")
const submitEl = document.getElementById("submit")

// output state (tokens, ast, execute, or code)
let outState = "code"

// wasm module
Module = {
  print: (text) => {
    if (text === "CODE") outState = "code"
    else if (text == "TOKENS") outState = "tokens"
    else if (text == "AST") outState = "ast"
    else if (text == "EXECUTE") outState = "execute"

    outEls[outState].innerHTML += text + "</br>"
  },

  onRuntimeInitialized: () => {

    // wrap C method
    let runCode = Module.cwrap("run", null, ["string", "number"])

    // on click, run
    submitEl.addEventListener("click", (e) => {
      outState = "code"
      Object.values(outEls).forEach((item) => item.innerHTML = "")
      runCode(exprEl.value, exprEl.value.length)
    })
  }
}