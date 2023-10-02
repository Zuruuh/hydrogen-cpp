#pragma once

#include <cstddef>
#include <cstdlib>

class ArenaAllocator {
    public:
        inline explicit ArenaAllocator(size_t bytes): m_size(bytes)
    {
        m_buffer = static_cast<std::byte*>(malloc(m_size));
        m_offset = m_buffer;
    }

        template<typename TValue>
            inline TValue* alloc() {
                void* offset = m_offset;
                m_offset += sizeof(TValue);

                return static_cast<TValue*>(offset);
            }

        inline ArenaAllocator(const ArenaAllocator& other) = delete;

        inline ArenaAllocator operator=(const ArenaAllocator& other) = delete;

        inline ~ArenaAllocator()
        {
            free(m_buffer);
        }

    private:
        size_t m_size;
        std::byte* m_buffer;
        std::byte* m_offset;
};
