#ifndef TIMER_H
#define TIMER_H

#include "PouEngine/Types.h"


namespace pou
{

template<typename T>
class SyncAttribute;

class Timer
{
    public:
        Timer();
        virtual ~Timer();

        virtual void reset(float time, bool looping = false);
        virtual void reset(const Time &time = Time(0), bool looping = false);
        virtual int update(float elapsedTime);
        virtual int update(const Time &elapsedTime);

        Time remainingTime();

        bool isActive();

    protected:

    private:
        Time   m_maxTime;
        Time   m_elapsedTime;
        bool        m_isLooping;
};

}

#endif // TIMER_H
