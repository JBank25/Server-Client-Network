#pragma once
#include <bit>
#include "net_common.h"

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
        uint32_t size = 0;
    };

    /**
     * message will also be a template type, given we have defined the id in 
     * the message_header as a template type
     */
    template <typename T>
    struct message
    {
        message_header<T> header{};
        std::vector<uint8_t> body;

        // Returns size of entire message packet in bytes
        size_t size() const
        {
            return body.size();
        }

        /**
         * @brief Overloads the << operator for outputting message objects to an ostream.
         * 
         * This function allows message objects to be easily printed or written to output streams.
         * It outputs the message's ID and size in bytes.
         * 
         * @tparam T The type parameter of the message template.
         * @param os The output stream to write to.
         * @param msg The message object to be output.
         * @return std::ostream& A reference to the output stream after writing the message data.
         */
        friend std::ostream &operator<<(std::ostream &os, const message<T> &msg)
        {
            os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
            return os;
        }

        // Convenience Operator overloads - These allow us to add and remove stuff from
        // the body vector as if it were a stack, so First in, Last Out. These are a
        // template in itself, because we dont know what data type the user is pushing or
        // popping, so lets allow them all. NOTE: It assumes the data type is fundamentally
        // Plain Old Data (POD). TLDR: Serialise & Deserialise into/from a vector

        /**
         * @brief Overloads the << operator to append data to a message object.
         * 
         * This function allows for easy addition of data to a message's body. It performs
         * the following operations:
         * 1. Checks if the data type is trivially copyable.
         * 2. Resizes the message body to accommodate the new data.
         * 3. Copies the data into the message body.
         * 4. Updates the message size.
         * 
         * @tparam T The type parameter of the message template.
         * @tparam DataType The type of data being appended to the message.
         * @param msg The message object to append data to.
         * @param data The data to be appended to the message.
         * @return message<T>& A reference to the modified message object, allowing for chaining.
         * 
         * @throws static_assert If the DataType is not trivially copyable.
         * 
         * @note This function uses memcpy, which assumes that the DataType is trivially copyable.
         *       Use with caution for complex types.
         */
        template<typename DataType>
        friend message<T>& operator << (message<T>& msg, const DataType& data)
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
            msg.header.size = msg.size();

            // Return the target message so it can be "chained"
            return msg;
        }


        /**
         * @brief Overloads the >> operator to extract data from a message object.
         * 
         * This function allows for easy extraction of data from a message's body. It performs
         * the following operations:
         * 1. Checks if the data type is trivially copyable.
         * 2. Copies the data from the end of the message body to the provided variable.
         * 3. Resizes the message body to remove the extracted data.
         * 4. Updates the message size.
         * 
         * @tparam T The type parameter of the message template.
         * @tparam DataType The type of data being extracted from the message.
         * @param msg The message object to extract data from.
         * @param data The variable to store the extracted data.
         * @return message<T>& A reference to the modified message object, allowing for chaining.
         * 
         * @throws static_assert If the DataType is not trivially copyable.
         * 
         * @note This function uses memcpy, which assumes that the DataType is trivially copyable.
         *       Use with caution for complex types.
         * @note This function extracts data from the end of the message body, assuming a LIFO (Last In, First Out) order.
         * @warning Ensure that the message contains enough data of the correct type before extraction.
         */
        // Pulls any POD-like data form the message buffer
        template<typename DataType>
        friend message<T>& operator >> (message<T>& msg, DataType& data)
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
            msg.header.size = msg.size();

            // Return the target message so it can be "chained"
            return msg;
        }	
    };

    template <typename T>
    class connection;

    template <typename T>
    struct owned_message
    {
        // ptr to remote connection to identify the connection
        std::shared_ptr<connection <T>> remote = nullptr;
        message<T> msg;

        friend std::ostream& operator <<(std::ostream& os, const owned_message<T>& msg)
        {
            os << msg.msg;
            return os;
        }

    };
}