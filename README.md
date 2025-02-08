![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/JBank25/Server-Client-Network/ci-main.yml?branch=main&label=build)


If you are doing this for the first time you'll need the Conan dependencies for both Release and Debug build
```
    $ task conan-install
    $ task conan-init-profile
    $ task conan-prep-builds
```

Build the project using:
```
    $ task build
```

Common tasks are managed using [Taskfiles](https://taskfile.dev/)
```
    $ task list
```

## Project Structure

### Modules
Modules are laid out within the `src/` directory
```
    $ tree -L1
    .
    ├── CMakeLists.txt
    ├── NetClient
    ├── NetCommon
    ├── NetConnection
    ├── NetMessage
    ├── NetServer
    └── main.cpp
```
Each of these modules will, generally speaking, have a `include/` and a `test/` directory associated with them as well as a `CMakeLists.txt` and `Module.cmake` to manage the build.
```
    .
    ├── CMakeLists.txt
    ├── Module.cmake
    ├── README.md
    ├── include
    └── test
```
### include/
Contains the header files for the module. 

### test/
Contains unit tests using [GoogleTest](https://github.com/google/googletest) for all the headers within the `include/` directory

### CMake
`CMakeLists.txt` should generally be the same across all modules. Module specific variable names are pulled out in `Module.cmake`


## Messages
Messages in this framework are comprised of two fields:
    1. Header - consists of a few fields. Will be a fixed size
        a. ID - identifier for what the message is 
        b. Size - size of the entire message including the header in bytes
    2. Body - payload of the message. Could be 0 or more bytes.

### Header
An enum class will be used for the header. Strong typing means we can identify a bug from invalid headers.

Using template, `T id{}` will allow users to customize the message and still allow the framework to behave in a defined way. 

### Body
Will use a `std::vector<uint8_t>` for the body so we are always working with bytes.