#ifndef DYNAMICVBO_H
#define DYNAMICVBO_H

#include "PouEngine/vulkanImpl/VBuffersAllocator.h"

namespace pou
{

template<class T> class DynamicVBO
{
    public:
        DynamicVBO(size_t chunkSize);
        virtual ~DynamicVBO();

        DynamicVBO( const DynamicVBO& ) = delete;
        DynamicVBO& operator=( const DynamicVBO& ) = delete;

        void push_back(const T &datum);

        size_t uploadVBO(); //return buffer size

        size_t  getSize();
        size_t  getUploadedSize();
        VBuffer getBuffer();

    protected:
        void cleanup();
        bool expand();

    private:
        size_t m_chunkSize;

        size_t m_curSize;
        size_t m_lastSize;
        std::vector<T> m_content;
        VBuffer m_buffer;
};

}
#include "../src/vulkanImpl/DynamicVBO.tpp"

#endif // DYNAMICVBO_H
