#pragma once

#include "net_common.h"

namespace net
{
    /**
     * @brief At any given time, in the client case, the queue can be accessed by 
     * server via ASIO or by the client itself. 
     * In the case of the server it could have many different clients writing to the queue.
     * Hence we have a need for the queue to be thread safe.
     * 
     * @tparam T 
     */
    template<typename T>
    class tsqueue
    {
    public:
        tsqueue() = default;
        tsqueue(const tsqueue<T>&) = delete;
        // call clear function when queue is destroyed
        virtual ~tsqueue() { clear(); }

    public:
        /**
         * @brief Get a reference to the front element of the queue.
         * 
         * This function returns a constant reference to the first element
         * in the queue without removing it. It uses a scoped lock to ensure
         * thread-safe access to the underlying deque.
         * 
         * @return const T& A constant reference to the front element of the queue.
         * 
         * @throws std::out_of_range if the queue is empty (behavior of std::deque::front).
         * 
         * @note This function is thread-safe due to the use of std::scoped_lock.
         * @note The returned reference is valid only as long as the queue is not modified.
         */
        const T& front()
        {
            std::scoped_lock lock(muxQueue);
            return deqQueue.front();
        }

        // Returns and maintains item at back of Queue
        const T& back()
        {
            std::scoped_lock lock(muxQueue);
            return deqQueue.back();
        }

        /**
        * @brief Removes and returns the first element from deqQueue in thread safe way
        * 
        * @tparam T The type of elements stored in the queue
        * @return T The element that was at the front of the queue (by value)
        * @throws std::runtime_error If the queue is empty when called
        * @note Thread-safe: Uses mutex locking to ensure safe concurrent access
        */
        T pop_front()
        {
            // apply scoped lock
            std::scoped_lock lock(muxQueue);
            // get object
            auto t = std::move(deqQueue.front());
            deqQueue.pop_front();
            return t;
        }

        // Removes and returns item from back of Queue
        T pop_back()
        {
            std::scoped_lock lock(muxQueue);
            auto t = std::move(deqQueue.back());
            deqQueue.pop_back();
            return t;
        }

        // Adds an item to back of Queue
        void push_back(const T& item)
        {
            std::scoped_lock lock(muxQueue);
            deqQueue.emplace_back(std::move(item));

            // wake up sleeper in wait() as queue is no longer empty
            std::unique_lock<std::mutex> ul(muxBlocking);
            cvBlocking.notify_one();
        }

        // Adds an item to front of Queue
        void push_front(const T& item)
        {
            std::scoped_lock lock(muxQueue);
            deqQueue.emplace_front(std::move(item));

            // wake up sleeper in wait() as queue is no longer empty
            std::unique_lock<std::mutex> ul(muxBlocking);
            cvBlocking.notify_one();
        }

        // Returns true if Queue has no items
        bool empty()
        {
            std::scoped_lock lock(muxQueue);
            return deqQueue.empty();
        }

        // Returns number of items in Queue
        size_t count()
        {
            std::scoped_lock lock(muxQueue);
            return deqQueue.size();
        }

        // Clears Queue
        void clear()
        {
            std::scoped_lock lock(muxQueue);
            deqQueue.clear();
        }

        /**
         * @brief - blocking function that suspends the calling object until a message
         * is written to the queue.
         * 
         */
        void wait()
        {
            // tight loop checking if queue is empty
            while (empty())
            {
                /**
                 * Sleep until condition variable is told to wake up. Only
                 * push_back() and push_front() can add to the queue. Good places
                 * to tell this thread to wake up
                 */
                std::unique_lock<std::mutex> ul(muxBlocking);
                cvBlocking.wait(ul);
            }
            // control returned back to calling thread upon return
        }

    protected:
        /**
         * mutex used to force thread safety. Note above that we are essentially
         * modifying built-in deque functions with mutex's 
         */
        std::mutex muxQueue;
        // Data stored in double ended queue
        std::deque<T> deqQueue;

        /**
         * 
         */
        std::condition_variable cvBlocking;
        std::mutex muxBlocking;
    };
}