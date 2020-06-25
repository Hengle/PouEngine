#include "PouEngine/sync/BoolSyncElement.h"

namespace pou
{

BoolSyncElement::BoolSyncElement() : BoolSyncElement(false)
{
    //ctor
}

BoolSyncElement::BoolSyncElement(bool v) : AbstractSyncElement(&m_attribute),
    m_attribute(v,0)
{
    //ctor
}

BoolSyncElement::~BoolSyncElement()
{
    //dtor
}

void BoolSyncElement::setValue(bool v)
{
    if(m_attribute.setValue(v));
        this->updateLastUpdateTime();
}

bool BoolSyncElement::getValue()
{
    return m_attribute.getValue();
}

void BoolSyncElement::serializeImpl(Stream *stream, uint32_t clientTime)
{
    auto v = m_attribute.getValue();

    stream->serializeBool(v);

    if(stream->isReading())
        m_attribute.setValue(v, true);
}

}
