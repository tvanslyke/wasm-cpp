# wasm-cpp
[![Language](https://img.shields.io/badge/language-C++17-blue.svg)](https://isocpp.org/) ![Language2](https://img.shields.io/badge/language-C11-blue.svg) ![Language3](https://img.shields.io/badge/language-CPython3.6-blue.svg) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

wasm-cpp is an **in-progress** WebAssembly runtime environment written in C++17 and C11 with a Python3.6 frontend.

This project is currently unfinished and untested.

## Existing Features
Supports standard WebAssembly features like static linking of wasm modules as well as:
* Full stack traces upon trapping.  

## Planned Features
The following features will likely be added in the future, once the author has time to implement them.
* A CPython module which allows dynamic debugging via a Python shell or script.
* A debug build which carries type information for all values in the wasm runtime.
* An optimized instruction dispatch loop that uses GCC's computed goto (labels as values) feature.
* A built-in "env" module, similar to what is provided by [binaryen](https://github.com/WebAssembly/binaryen)

## Dependencies
The only dependencies that wasm-cpp has are the languages it is written in (and their associated standard libraries).  That is, wasm-cpp only depends on:
* A C++17 compiler and standard library implementation.
* A C11 compiler and standard library implementation.
* CPython3.6 and its standard library.

## Installation
Only an incomplete, non-portable CMakeLists.txt is provided at this time.  As the project nears completion, a proper installation method and guide will be provided.

## Usage
Coming soon.

## Contributing and Bug Reports
Contributions and bug reports are welcome.  Please submit all patches and bug reports via Github's pull request and issues features, respectively.

## License
Thie project is licensed under the MIT License.  See [MIT-LICENSE](./MIT-LICENSE) for details.
