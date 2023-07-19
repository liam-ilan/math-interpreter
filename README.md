# Just a Math Interpeter
Written in C, compiled to WASM | [https://liam-ilan.github.io/math-interpreter/](https://liam-ilan.github.io/math-interpreter/)

## About
This project is a simple math interpeter. It supports expressions involving additon ("+"), subtraction ("-"), multiplication ("*"), division ("/"), brackets ("(" and ")"), and both floating point and integer arithmetic. The grammar for the inperpreter, in EBNF, is:

```ebnf
unary = ("-", unary) | int | float | ("(", expression, ")");
factor = (factor, ("*" | "/"), unary) | unary;
expression = (expression, ("+" | "-"), factor) | factor;
```

Or graphically, 

![Syntax diagram](public/readme-syntax-diagram.png)

*Generated with Vincent Jacques' DrawGrammar*

The interpeter is implemented in around 600 lines of C, and then compiled to WASM with Emscripten to run on the web. Play around with the interpreter [here](https://liam-ilan.github.io/math-interpreter/).

## Development
All interpreter code can be found in `main.c`. To compile the interpreter with gcc (not emscripten), run
```bash
gcc main.c -o ./main
```

The version of `./main` included in this repo was built for Linux.

When compiled without emscripten, the interpreter reads from a supplied file. Run
```bash
./main file_name.here
```
to run the math expression contained in the given file. `code.txt` contains a sample test expression to get started.

### Compiling for the Web
This project utilzes Emscripten and WASM to compile and run on the web [here](https://liam-ilan.github.io/math-interpreter/).

To compile the interpreter with Emscripten, run,
```
emcc main.c -o public/run.js -s EXPORTED_FUNCTIONS="['_run']" -s EXPORTED_RUNTIME_METHODS="['cwrap']"
```

> Note: To run locally, navigate to the `./public` directory, and run with the local webserver of your choice. The site will not work when loaded through `file://`, as sites loaded this way do not support XHR. Read more at [https://emscripten.org/docs/getting_started/Tutorial.html](https://emscripten.org/docs/getting_started/Tutorial.html).

## Credits
- Build by [Liam Ilan](https://www.liamilan.com/).