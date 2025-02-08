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

Common tasks are managed using [Taskfiles][https://taskfile.dev/]
```
    $ task list
```


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