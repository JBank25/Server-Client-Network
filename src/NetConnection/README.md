When a `Connection` is instantiated we have to specify a few things in the constructory

	1. `owner parent` - Connections are "owned" by either a client or a
server. This field specifies whether or not a client or server owns the
connection. The enum class for this type is defined within the connection
class.
	1. `asio::io_context &asioContext`- A reference to the io_context we
are using. This is owned either by the server or the client. 
	1. `asio::ip::tcp::socket socket`- Connections have a socket it is unique and owned by them
	1. `tsqueue<owned_message><T>> &qIn` - Connections must have a ptr to
the message queue they feed from whether they are the client or the server,
pass this in by reference
