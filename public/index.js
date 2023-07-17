// elements for output
const outEls = {
  code: document.getElementById("code"),
  tokens: document.getElementById("tokens"),
  ast: document.getElementById("ast"),
  execute: document.getElementById("execute")
}

// elements for selecting viewing mode
const selectionEls = {
  code: document.getElementById("codeSelect"),
  tokens: document.getElementById("tokensSelect"),
  ast: document.getElementById("astSelect"),
  execute: document.getElementById("executeSelect")
}

// elements for input
const exprEl = document.getElementById("expr")
const submitEl = document.getElementById("submit")

// output state (tokens, ast, execute, or code)
let outState = "code"

// updates ui
function updateView(viewState) {
  // reset buttons and view panes
  Object.keys(selectionEls).forEach((key) => {
    selectionEls[key].classList.remove("selected")
    outEls[key].classList.remove("show")
  })
  
  // add selected class
  selectionEls[viewState].classList.add("selected")
  outEls[viewState].classList.add("show")
}

updateView("execute")

// add event listenrs
Object.keys(selectionEls).forEach((key) => {
  selectionEls[key].addEventListener("click", (e) => {
    updateView(key)
  })
})

// wasm module
Module = {
  print: (text) => {
    if (text === "CODE") outState = "code"
    else if (text == "TOKENS") outState = "tokens"
    else if (text == "AST") outState = "ast"
    else if (text == "EXECUTE") outState = "execute"
    else outEls[outState].innerHTML += text + "</br>"
  },

  onRuntimeInitialized: () => {

    // wrap C method
    let runCode = Module.cwrap("run", null, ["string", "number"])

    // submit method
    function submitMethod(e) {
      outState = "code"
      Object.values(outEls).forEach((item) => item.innerHTML = "")
      runCode(exprEl.value, exprEl.value.length)
    }
    // on click or enter, run
    submitEl.addEventListener("click", submitMethod)
    exprEl.addEventListener("keydown", (e) => e.key === "Enter" ? submitMethod() : null)
  }
}