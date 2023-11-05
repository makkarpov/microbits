#include <ub/fifo.hpp>

#include <algorithm>
#include <cstring>

using namespace ub;

void CircularBuffer::setBuffer(void *buffer, size_t length) {
    m_headPos   += pendingBytes();
    m_buffer    = (uint8_t *) buffer;
    m_capacity  = length;
    m_rdPtr     = 0;
    m_wrPtr     = 0;
}

void CircularBuffer::reset() {
    m_headPos   = 0;
    m_rdPtr     = 0;
    m_wrPtr     = 0;
}

size_t CircularBuffer::pendingBytes() const {
    if (m_rdPtr <= m_wrPtr) {
        return m_wrPtr - m_rdPtr;
    } else {
        return m_capacity + m_wrPtr - m_rdPtr;
    }
}

size_t CircularBuffer::freeBytes() const  {
    if (m_rdPtr <= m_wrPtr) {
        return m_capacity - m_wrPtr + m_rdPtr - 1;
    } else {
        return m_rdPtr - m_wrPtr - 1;
    }
}

uint8_t *CircularBuffer::writePtr() const {
    return m_buffer + m_wrPtr;
}

size_t CircularBuffer::writeLimit() const {
    if (m_wrPtr < m_rdPtr) {
        return m_rdPtr - m_wrPtr - 1;
    } else if (m_rdPtr == 0) {
        return m_capacity - 1 - m_wrPtr;
    } else {
        return m_capacity - m_wrPtr;
    }
}

uint8_t *CircularBuffer::readPtr() const {
    return m_buffer + m_rdPtr;
}

size_t CircularBuffer::readLimit() const {
    if (m_rdPtr > m_wrPtr) {
        return m_capacity - m_rdPtr;
    } else {
        return m_wrPtr - m_rdPtr;
    }
}

void CircularBuffer::readBytes(size_t n) {
    m_rdPtr += n;
    m_headPos += n;

    if (m_rdPtr >= m_capacity) {
        m_rdPtr -= m_capacity;
    }
}

void CircularBuffer::writeBytes(size_t n) {
    m_wrPtr += n;

    if (m_wrPtr >= m_capacity) {
        m_wrPtr -= m_capacity;
    }
}

void CircularBuffer::readBytes(void *buffer, size_t n) {
    size_t offset = 0;
    while (offset < n) {
        size_t len = std::min(readLimit(), n - offset);
        std::memcpy((uint8_t *) buffer + offset, readPtr(), len);

        readBytes(len);
        offset += len;
    }
}

void CircularBuffer::writeBytes(const void *buffer, size_t n) {
    size_t offset = 0;
    while (offset < n) {
        size_t len = std::min(writeLimit(), n - offset);
        std::memcpy(writePtr(), (const uint8_t *) buffer + offset, len);

        writeBytes(len);
        offset += len;
    }
}
