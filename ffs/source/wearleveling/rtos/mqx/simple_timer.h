#ifndef _simple_timer_h_
#define _simple_timer_h_
/**HEADER********************************************************************
* Copyright (c) 2013 Freescale Semiconductor, Inc. All rights reserved.
* 
* Freescale Semiconductor, Inc.
* Proprietary & Confidential
* 
* This source code and the algorithms implemented therein constitute
* confidential information and may comprise trade secrets of Freescale Semiconductor, Inc.
* or its associates, and any use thereof is subject to the terms and
* conditions of the Confidential Disclosure Agreement pursual to which this
* source code was originally received.
*****************************************************************************
* $FileName: simple_timer.h$
* $Version : 3.8.0.1$
* $Date    : Aug-9-2012$
*
* Comments:  Simple time operations
*END************************************************************************/

#define OS_MSECS_TO_TICKS wl_msecs_to_ticks

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uint_64 wl_hw_get_microsecond();
uint_64 wl_msecs_to_ticks(uint_32 msecs);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/*
 * Bare bones microsecond timer class.
 */
class SimpleTimer
{
public:
    /* Constructor; takes the start timestamp. */
    inline SimpleTimer()
    {
        m_start = wl_hw_get_microsecond();

    }
    
    /* Computes and returns the elapsed time since the object was constructed. */
    inline uint64_t getElapsed()
    {
        return wl_hw_get_microsecond() - m_start;
    }

protected:
    /* The start timestamp in microseconds. */
    uint64_t m_start;   
};

/*
** Struct used for computing average operation times.
*/
class AverageTime
{
public:
    /* Constructor to init counts to zero. */
    inline AverageTime() : accumulator(0), count(0), averageTime(0), minTime(~0), maxTime(0) {}
    
    /* Add time to the average. */
    inline void add(uint64_t amount, unsigned c=1)
    {
        accumulator += amount;
        count += c;
        averageTime = accumulator / count;
        maxTime = (unsigned)MAX(maxTime, amount/c);
        minTime = (unsigned)MIN(minTime, amount/c);
    }
    
    /* Overloaded operator to add time to the average. */
    inline AverageTime & operator += (uint64_t amount)
    {
        add(amount);
        return *this;
    }
    
    /* Accessor methods */
    inline unsigned getCount() const { return count; }
    inline unsigned getAverage() const { return averageTime; }
    inline unsigned getMin() const { return minTime; }
    inline unsigned getMax() const { return maxTime; }
    
    /* Clear the accumulator and counter and reset times. */
    inline void reset()
    {
        accumulator = 0;
        count = 0;
        averageTime = 0;
        minTime = ~0;
        maxTime = 0;
    }

protected:
    uint64_t accumulator;
    unsigned count;
    unsigned averageTime;
    unsigned minTime;
    unsigned maxTime;
};

#endif /* _simple_timer_h_ */

/* EOF */

