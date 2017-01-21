# ccurl
C port of the Curl library

## Usage

Don't forget to grab submodules! use `git submodule update --init --recursive` 
when cloning, and `git submodule update --recursive` when pulling from this 
repository.

### To Compile

    CMake is a build dependency. If you are testing, you must also
    install CUnit (BCUnit).

`mkdir build && cd build && cmake .. && cmake --build . &&  cd ..`

#### Note for Windows

You may build using mingw or cygwin, but if you build with Visual C++, it is
highly recommended that you also install the 
[LLVM compiler](http://releases.llvm.org/download.html). Instead of the `cmake ..`
statement above, I would recommend the following: `cmake -A x64 -T LLVM-vs2014 ..`

### Command-line programs

There are two included command-line programs, ccurl-cli, and ccurl-digest. Usage 
is as follows:

`% ccurl-digest $(ccurl-cli <MinWeightMagnitude> <TransactionTrytes>)`

where <MinWeightMagnitude> is replaced with, for example, 18, and 
<TransactionTrytes> is replaced with your 2673-length transaction.

#### Tricks

If using GPU acceleration, you may tune the loop length with 
`ccurl_pow_set_loop_count(my_loop_count);` with the library, or 
setting an environment variable to, for example, `CCURL_LOOP_COUNT=64`, when 
using ccurl-cli.
