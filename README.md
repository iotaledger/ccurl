# ccurl
C port of the Curl library

## Usage

* To Compile

    CMake is a build dependency. If you are testing, you must also
    install CUnit (BCUnit).

`mkdir build && cd build && cmake .. && cmake --build . &&  cd ..`

### Note for Windows

You may build using mingw or cygwin, but if you build with Visual C++, it is
highly recommended that you also install the 
[LLVM compiler](http://releases.llvm.org/download.html). Instead of the `cmake ..`
statement above, I would recommend the following: `cmake -A x64 -T LLVM-vs2014 ..`
