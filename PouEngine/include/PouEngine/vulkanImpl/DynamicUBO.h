#ifndef DYNAMICUBO_H
#define DYNAMICUBO_H

#include <list>

#include "PouEngine/vulkanImpl/VInstance.h"
#include "PouEngine/vulkanImpl/VBuffersAllocator.h"

namespace pou
{

class DynamicUBO
{
    public:
        DynamicUBO(size_t objectSize, size_t chunkSize );
        virtual ~DynamicUBO();

        DynamicUBO( const DynamicUBO& ) = delete;
        DynamicUBO& operator=( const DynamicUBO& ) = delete;

        bool    allocObject(size_t &index);
        bool    freeObject(size_t index);
        bool    isFull();

        bool    updateObject(size_t index, void *data);

        void expandBuffers(bool destroyOldBUffers = true);

        uint32_t getDynamicOffset(size_t index);

        VBuffer        getBuffer();
        size_t          getBufferVersion();

    protected:
        void computeDynamicAlignment();
        void createBuffers();
        void cleanup();

    private:
        size_t m_objectSize;
        size_t m_chunkSize;
        size_t m_dynamicAlignment;

        VBuffer         m_buffer;
        VkDeviceSize    m_bufferSize;
        size_t          m_totalSize;

        std::list<size_t> m_availableIndices;

        size_t m_bufferVersion;
        bool m_firstTime;
};

}

#endif // DYNAMICUBO_H
