```
gcc main.c -o ./main && ./main
```
```
emcc main.c -o public/run.js -s EXPORTED_FUNCTIONS="['_run']" -s EXPORTED_RUNTIME_METHODS="['cwrap']"
```