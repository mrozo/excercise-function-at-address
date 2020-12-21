# addr2func

Search map file for a function name specified by address.

## Usage

```
add2func --address=<address> --map=<map file>
add2func --help
Arguments:
--address=<address> - address of to be searched for
--map=<map file> - path to the map file
--help - display this help
```

## Build

Use `make all` command to build the application in ./build directory.

Other build targets:
 1. `make debug` - create executable in ./debug/ with debug symbols and debug macros enabled
 2. `make gdb` - debug application in gdb 
 3. `make clean`
