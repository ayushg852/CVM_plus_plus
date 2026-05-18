# CVM++: A Lightweight Custom Scripting Language

CVM++ is a lightweight, statically-typed scripting language implemented in Modern C++ (C++17). It features a custom Lexer, Recursive Descent Parser, and a stack-based Virtual Machine.

## Features
- **Explicit Data Types**: `integer`, `decimal`, `character`, `string`, `boolean`.
- **Type Inference**: Use `let` for automatic type deduction.
- **Control Flow**: `if/else`, `during` (while), and `for` loops.
- **Functions**: Support for custom functions and recursion.
- **Input/Output**: Built-in `display()` for output and `enter()` for input.
- **Error Handling**: Detailed compiler errors with line/column information.

## Syntax Overview

### Variables
```
integer x = 10;
decimal y = 20.5;
string s = "Hello CVM++";
let autoVar = 5; // inferred as integer
```

### Loops
```
for(integer i from 1 to 5) {
    display(i);
}

integer j = 5;
during(j > 0) {
    display(j);
    j = j - 1;
}
```

### Functions
```
integer factorial(integer n) {
    if (n <= 1) { return 1; }
    return n * factorial(n - 1);
}
display(factorial(5));
```

## Building the Project
The project uses `g++` and a simple PowerShell build script.

1. Ensure `g++` is in your PATH (supports C++17).
2. Run the build script:
   ```powershell
   .\build.ps1
   ```
3. This will generate `cvm.exe`.

## Running Scripts
You can run a `.cvm` file directly:
```powershell
.\cvm.exe hello.cvm
```

Or run the interactive REPL:
```powershell
.\cvm.exe
```

## Debugging
The compiler provides detailed error messages:
- `datatype mismatch at line X`
- `undefined variable 'x' at line X`
- `missing return statement at line X`

## Sample Programs
- `hello.cvm`: Basic input/output.
- `math.cvm`: Arithmetic and logical operations.
- `loops.cvm`: Loop constructs.
- `recursion.cvm`: Recursive factorial implementation.
