#!/bin/sh
mkdir bins/

xa  -bt0 -Istdlib/ sample_programs/alphabet.s  -o bins/alphabet
xa  -bt0 -Istdlib/ sample_programs/echo.s  -o bins/echo

make
mv x6502 x6502.bc
emcc x6502.bc -o x6502.js --preload-file bins@/
