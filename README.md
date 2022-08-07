# pattern

This utility summarizes the pattern in a group of filenames.

It is locale-aware, understanding not just UTF-8, but many other ASCII-compatible encodings, like Big5.
It also understands user-perceived characters, a.k.a. grapheme clusters: a letter with several diacritics counts 
as one character.

## Example

If you have a directory `dir` with the following files:

    main.cpp.106t.stdarg
    main.cpp.127t.sincos
    main.cpp.134t.sink5.7

running `pattern dir` will give you a character-by-character analysis:

                        0 4           a 5 .  
                        2 6       i d c o g  
    m a i n . c p p . 1 3 7 t . s t n k r s 7

All three files start with the characters `main.cpp.1`, but after that, they start to differ: the next character is
one of `0`, `2`, or `3` depending on the filename.

Running `pattern -d . dir` will instead analyze the filenames in "chunks" split around the delimiter `.`:

                 106t   sincos    
                 127t   sink5     
    main . cpp . 134t . stdarg . 7

The first two chunks are the same between all filenames, but the next two are all different.

## Dependencies

- [GNU libunistring](https://www.gnu.org/software/libunistring/)
- a C++ compiler supporting C++11
- a POSIX environment

## How to install

`make && sudo make install`

## Notes

- This is not yet thoroughly tested, and may contain bugs.

- The terminal in which this utility runs should use a font that is not variable width; i.e. the font should be
  monospace, duospace, etc.

- There is still desirable functionality that is missing, like reading from stdin and processing directories 
  recursively.