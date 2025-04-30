# Simple Compiler

A C++ compiler for a simplified programming language that supports lexical and syntax analysis.

## Features

- Lexical analysis (scanning)
- Syntax analysis (parsing)
- Support for basic language constructs:
  - Variable declarations
  - Function declarations
  - Control flow statements (if, while, for)
  - Expressions and operators
  - Comments (single-line and multi-line)
  - File inclusion

## Building

### Prerequisites

- CMake 3.10 or higher
- C++17 compatible compiler
- Make or Ninja build system

### Build Steps

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Configure the project:
```bash
cmake ..
```

3. Build the project:
```bash
cmake --build .
```

## Usage

The compiler can be run with the following command:

```bash
./simple_compiler <input_file> [options]
```

### Options

- `--help`: Show help message
- `--lex-only`: Only perform lexical analysis
- `--parse-only`: Only perform parsing
- `--output <file>`: Specify output file

### Example

```bash
./simple_compiler input.txt --output output.txt
```

## Language Syntax

### Keywords

- `IfTrue`, `Otherwise`: Conditional statements
- `Imw`, `Float`, `String`, `Bool`, `Void`: Data types
- `Return`: Function return
- `While`, `For`: Loops
- `Break`, `Continue`: Loop control

### Operators

- Arithmetic: `+`, `-`, `*`, `/`
- Logical: `&&`, `||`, `~`
- Relational: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Assignment: `=`

### Comments

- Single-line: `//^ comment`
- Multi-line: `//@ comment @/`

### File Inclusion

```c
#include "filename"
```

## Error Handling

The compiler provides detailed error messages including:
- Line and column numbers
- Error type (lexical or syntax)
- Error description

## Project Structure

- `scanner.h/cpp`: Lexical analyzer implementation
- `parser.h/cpp`: Syntax analyzer implementation
- `main.cpp`: Main program entry point
- `CMakeLists.txt`: Build configuration

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details. 