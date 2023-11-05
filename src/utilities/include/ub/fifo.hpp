#ifndef UB_UTILITIES_FIFO_H
#define UB_UTILITIES_FIFO_H

#include <cstdint>
#include <cstddef>

namespace ub {
    /**
     * Circular byte buffer.
     *
     * This class is not thread safe and must be synchronized externally.
     */
    class CircularBuffer {
    public:
        /** Create empty CircularBuffer instance which needs to be populated via `setBuffer` call. */
        explicit constexpr CircularBuffer(): m_buffer(nullptr), m_capacity(0), m_rdPtr(0), m_wrPtr(0), m_headPos(0) {}

        /** Create CircularBuffer instance with buffer set at construction time */
        explicit constexpr CircularBuffer(void *buffer, size_t size): CircularBuffer() {
            m_buffer = (uint8_t *) buffer;
            m_capacity = size;
        }

        /**
         * Set backing storage buffer location. All currently pending bytes are discarded.
         *
         * @param buffer Storage buffer
         * @param length Length of storage buffer
         */
        void setBuffer(void *buffer, size_t length);

        /**
         * Reset all internal state, discarding all currently pending data. Absolute byte counters are reset to zero.
         */
        void reset();

        /** @return FIFO head position (number of bytes popped from this FIFO since it's creation) */
        [[nodiscard]] inline size_t headPosition() const { return m_headPos; }

        /** @return FIFO tail position (number of bytes pushed to this FIFO since it's creation) */
        [[nodiscard]] size_t tailPosition() const { return m_headPos + pendingBytes(); }

        /** @return Number of currently pending bytes */
        [[nodiscard]] size_t pendingBytes() const;

        /** @return Number of free bytes */
        [[nodiscard]] size_t freeBytes() const;

        /** @return Buffer location to put new data */
        [[nodiscard]] uint8_t *writePtr() const;

        /** @return Buffer location to read pending data */
        [[nodiscard]] uint8_t *readPtr() const;

        /**
         * Returns maximum number of bytes that could be safely written. May be lower than number of free bytes due
         * to buffer discontinuity. In this case, write must be split into smaller parts.
         */
        [[nodiscard]] size_t writeLimit() const;

        /**
         * Returns maximum number of bytes that could be safely read. May be lower than number of pending bytes due
         * to buffer discontinuity. In this case, read must be split into smaller parts.
         */
        [[nodiscard]] size_t readLimit() const;

        /**
         * Discard first `n` bytes from the head and update the FIFO state. `n` must be less than or equal to
         * `pendingBytes` to ensure correct operation.
         *
         * This function only updates buffer state and does not perform any actual data transfers. To access actual
         * FIFO contents, use `readPtr()` and `readLimit()` methods, or use `readBytes(buf, len)` call which does
         * actual data transfer.
         */
        void readBytes(size_t n);

        /**
         * Copy first `n` bytes into the provided buffer and discard these from the queue. `n` must be less than or
         * equal to `pendingBytes` to ensure correct operation.
         */
        void readBytes(void *buffer, size_t n);

        /**
         * Append `n` bytes to the tail and update the FIFO state. `n` must be less than or equal to `freeBytes` to
         * ensure correct operation.
         *
         * This function only updates buffer state and does not perform any actual data transfers. To prepare actual
         * FIFO contents, use `writePtr()` and `writeLimit()` methods, or use `writeBytes(buf, len)` call which does
         * actual data transfer.
         */
        void writeBytes(size_t n);

        /**
         * Append `n` bytes to the tail and update the FIFO state. `n` must be less than or equal to `freeBytes` to
         * ensure correct operation.
         */
        void writeBytes(const void *buffer, size_t n);

    private:
        uint8_t     *m_buffer;
        size_t      m_capacity;
        size_t      m_rdPtr;
        size_t      m_wrPtr;
        size_t      m_headPos;
    };
}

#endif // UB_UTILITIES_FIFO_H
