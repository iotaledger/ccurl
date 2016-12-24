# ccurl
C port of the Curl library

## Usage

* To Compile

    CMake and OpenCL are build dependencies. If you are testing, you must also
    install CUnit (BCUnit).

`mkdir build && cd build && cmake .. && cd .. && make -C build`

Include the header files found in `src/`.

In order to run automated tests, you must have BCUnit installed.
