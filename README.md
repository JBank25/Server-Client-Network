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

Messages do have a notion of tagging. In some instances servers may want to respond to a specific client. To facilitate this we use the connection a client/server has to identify the message

### Header
An enum class will be used for the header. Strong typing means we can identify a bug from invalid headers.

Using template, `T id{}` will allow users to customize the message and still allow the framework to behave in a defined way. 

### Body
Will use a `std::vector<uint8_t>` for the body so we are always working with bytes.

## Architecture

## Server
Servers will communicate with clients via a connection. A server can have many
connections and can communicate with one at a time or all at once.

The server itself will only have one queue for processing messages. It will be
single threaded and should process the messages as they come in. 

Servers differentiate themselves from clients in that they can have many
connections.

Servers must know where the message they receive came from in the event that it
needs to respond back to that specific client. They connection is what
identifies specific client, connections ARE the interface in which the server
uses to communicate to clients.

The server has two roles
    1. Waiting for and handling connections/disconnections
    2. Handl application logic (Server lends itself to user subclasses)

The ASIO context associated with the server will need to wait for connections.
This will be the work we "prime" ASIO with. Once the connection is made, a
socket will be created, we then create a connection object to handle the socket.
We then provide the server the opportunity to reject the connection. If we
choose to connect then we will place the connection object into a container of
connections, if we choose not to then we will simply go back to waiting for
connections. 
    1. Accept Connection
    2. Create socket
    3. Create connection object from socket
    4. Choose what to do with the connection (Accept? Reject?)
    5. If accepted, put the connection in our pool of connections
    6. Wait for new connections

Connection objects will be `shared_ptr`. We can rely on these to keep things tidy.

Once a connection is established, the connection will assign some work to itself
in reading the header. If the header has contents then a read body task will
fire. Once the read body task has expired we have read an entire message and we
can add it to the queue. The connection task will go back to waiting for a
header.

At a high level the server will:
    1. Be issued a task by the asio context to sit and wait for connections - `WaitForClientConnection()`
    2. When something does try to connect we create a new connection object `std::shared_ptr<connection<T>>` for it
    3. We give the server a chance to decide what it wants to do with this object (accept, rejectm custom logic for this, etc)
    4. Add the connection to the container of connections
    5. Allocate the connection an ID.
    6. Reprime the context with more work to do (`WaitForClientConnection()` regardless of if the connection is accepted or rejected)

## Client
Clients have a queue associated with them which receives messages. The client will do it work and have the queue populated asyhcronously by the connection the client has. 

The connection the client has will have a queue within it as well. This queue will need to send the data it has to the server so it can be processed.

`Client`                    `Connection`
    `Queue`     <-----x         `ASIO`
    `Messages`  x----->         `Queue`

The connection the client has which is receieving messages will ultimately be connected to a different connection which is connected to the server. The asio framework is the glue between the connection

`Client` <----> `Connection` <----> `Connection` <----> `Server`
                             `ASIO`

It should be noted that clients can only have a single connection whereas servers can have many.

Client must provide a physical queue to store messages. This is the same queue
that will be used by the connection.  `tsqueue<owned_message<T>> m_qMessagesIn`
See `tsqueue<owned_message<T>> &m_qMessagesIn` in Connection



## Queue
Clients, servers, and connections all utilize the same `Queue` class. 
The queue must be thread safe. At any given time a queue could be accessed by the client or the server.

The server has many different connections, again necessitating it be thread-safe.

The implementation is essentially just putting a scoped lock around the `std::deque<T>` type.


![alt text](image.png)


## Connection
The connection object is used by both clients and servers. Connections are how
we send messages and are responsible for all the asio stuff

Each connection has a socket `asio::ip::tcp::socket m_socket`. Recall that ASIO
can not run properly and sockets can not function without an io context
`asio::io_context &m_asioContext`, this is a reference owned by either the
client interface or server object 

Connections only contain queues of messages going out. As a result we will need
a threadsafe queue of messages within our connection 
`tsqueue<message<T>> m_qMessagesOut`

Messages coming in to the server or the client are stored in a queue owned by
the server or the client. The connection needs to know where that queue is. As a
result we will create a reference to a threadsafe queue of owned_messages,
`tsqueue<owned_message<T>> &m_qMessagesIn`

A connection is "owned" by either a client or a server and on creation we will
mark the object as such
```
    enum class owner
    {
        server,
        client
    };
```

`ReadHeader()` will utilize `asio::async_read()` to read from a particular socket into a buffer capable of reading a buffer in of size `message_header<T>`.

## ASIO
The ASIO context must always have work or it will shutdown.

In the case of the server, we want 