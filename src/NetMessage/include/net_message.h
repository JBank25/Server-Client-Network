#pragma once
#include "net_common.h"

namespace olc
{
    namespace net
    {
        /**
         * Header of messages. Contains a field which is the id of the message
         * and another holding the size in bytes of the message. This header will
         * be a fixed size.  
         */
        template <typename T>
        struct message_header
        {
            // Use templates to customize the message across users.
            // Will use an enum class for this ultimately. Strongly typed 
            // to help find bugs
            T id{};
            uint32_t sizeBytes = 0;
        };

        template <typename T>
        struct message
        {
            message_header<T> header{};
            std::vector<uint8_t> body;

            // Returns size of entire message packet in bytes
            size_t sizeBytes() const
            {
                return sizeof(message_header<T>) + body.size();
            }

            // Override for std::cout compatibility - produces friendly description of message
            friend std::ostream &operator<<(std::ostream &os, const message<T> &msg)
            {
                os << "ID:" << int(msg.header.id) << " Size:" << msg.header.sizeBytes;
                return os;
            }

            // Convenience Operator overloads - These allow us to add and remove stuff from
            // the body vector as if it were a stack, so First in, Last Out. These are a
            // template in itself, because we dont know what data type the user is pushing or
            // popping, so lets allow them all. NOTE: It assumes the data type is fundamentally
            // Plain Old Data (POD). TLDR: Serialise & Deserialise into/from a vector

            // Pushes any POD-like data into the message buffer
            template <typename DataType>
            friend message<T> &operator<<(message<T> &msg, const DataType &data)
            {
                // Check that the type of the data being pushed is trivially copyable
                static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

                // Cache current size of vector, as this will be the point we insert the data
                size_t i = msg.body.size();

                // Resize the vector by the size of the data being pushed
                msg.body.resize(msg.body.size() + sizeof(DataType));

                // Physically copy the data into the newly allocated vector space
                std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

                // Recalculate the message size
                msg.header.sizeBytes = msg.sizeBytes();

                // Return the target message so it can be "chained"
                return msg;
            }

            // Pulls any POD-like data form the message buffer
            template <typename DataType>
            friend message<T> &operator>>(message<T> &msg, DataType &data)
            {
                // Check that the type of the data being pushed is trivially copyable
                static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");

                // Cache the location towards the end of the vector where the pulled data starts
                size_t i = msg.body.size() - sizeof(DataType);

                // Physically copy the data from the vector into the user variable
                std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

                // Shrink the vector to remove read bytes, and reset end position
                msg.body.resize(i);

                // Recalculate the message size
                msg.header.sizeBytes = msg.sizeBytes();

                // Return the target message so it can be "chained"
                return msg;
            }
        };

        template <typename T>
        class connection;

        template <typename T>
        struct owned_message
        {
            std::shared_ptr<connection <T>> remote = nullptr;
            message<T> msg;

            friend std::ostream& operator <<(std::ostream& os, const owned_message<T>& msg)
            {
                os << msg.msg;
                return os;
            }

        };
    }
}