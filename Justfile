test *args: build
    just run {{args}}

run *args:
    -./build/hydro {{args}}

build:
    cmake -S . -B build/ > /dev/null
    cmake --build build > /dev/null
