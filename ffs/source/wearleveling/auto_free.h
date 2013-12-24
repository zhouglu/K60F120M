#ifndef __auto_free_h__
#define __auto_free_h__
/*HEADER**********************************************************************
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Freescale Confidential and Proprietary - use of this software is
 * governed by the Freescale MQX RTOS License distributed with this
 * material. See the MQX_RTOS_LICENSE file distributed for more
 * details.
 *
 *****************************************************************************
 *
 * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************
 *
 * $FileName: auto_free.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   Defines a class to ensure release of memory allocated
 *
 *END************************************************************************/

#include <stdlib.h>

/*
** Type definitions
*/

/*
** Template helper class to automatically free memory.
** When the instance of this class falls out of scope, it will free the pointer it
** owns with the standard C library free() API.
*/

template <typename T>
class auto_free
{
    typedef T * t_ptr_t;
    typedef const T * const_t_ptr_t;
    typedef T & t_ref_t;
    typedef const T & const_t_ref_t;
    
    /* The pointer we own. */
    t_ptr_t m_ptr;
    
public:
    /* Default constructor, sets pointer to NULL. */
    inline auto_free() : m_ptr(0) {}
    
    /* Constructor taking a pointer to own. */
    inline auto_free(t_ptr_t p) : m_ptr(p) {}
    
    /* Constructor taking a void pointer to own. */
    inline auto_free(void * p) : m_ptr(reinterpret_cast<t_ptr_t>(p)) {}
    
    /* Copy constructor. */
    inline auto_free(auto_free<T> & o) : m_ptr(o.release()) {}
    
    /* Copy constructor for compatible pointer type. */
    template <typename O>
    inline auto_free(auto_free<O> & o) : m_ptr(o.release()) {}
    
    /* Destructor. */
    inline ~auto_free()
    {
        free();
    }
    
    /* Clear the pointer we own. */
    inline void reset()
    {
        m_ptr = 0;
    }
    
    /* Return the pointer and clear it. */
    inline t_ptr_t release()
    {
        t_ptr_t tmp = m_ptr;
        reset();
        return tmp;
    }
    
    /* 
    ** Free the memory occupied by the pointer.
    ** Does nothing if the pointer is NULL. 
    */
    inline void free()
    {
        if (m_ptr)
        {
            ::free(m_ptr);
            reset();
        }
    }
    
    /* 
    ** Get and set
    ** Return the owned pointer. 
    */
    inline t_ptr_t get() { return m_ptr; }
    
    /* Return the owned pointer. */
    inline const_t_ptr_t get() const { return m_ptr; }
    
    /* 
    ** Changed the owned pointer to a new value.
    ** If there was a previous pointer, it will be freed first. If p is NULL or the same
    ** as the currently owned pointer then nothing will be done. 
    */
    inline void set(t_ptr_t p)
    {
        if (p && p != m_ptr)
        {
            free();
        }
        m_ptr = p;
    }
    
    /* Variant of set() taking a void pointer. */
    inline void set(void * p) { set(reinterpret_cast<t_ptr_t>(p)); }
    
    /* Conversion Operators */
    inline operator t_ptr_t () { return m_ptr; }

    inline operator const_t_ptr_t () const { return m_ptr; }
    
    inline operator bool () const { return m_ptr != NULL; }
    
    /* Access operators */
    inline t_ref_t operator * () { return *m_ptr; }
    
    inline const_t_ref_t operator * () const { return *m_ptr; }
    
    inline t_ptr_t operator -> () { return m_ptr; }
    
    inline const_t_ptr_t operator -> () const { return m_ptr; }
    
    /* Assignment operator. */
    inline auto_free<T> & operator = (t_ptr_t p)
    {
        set(p);
        return *this;
    }
    
    /* Compatible assignment operator. */
    template <typename O>
    inline auto_free<T> & operator = (auto_free<O> & o)
    {
        set(o.release());
        return *this;
    }

};

/*
** Template helper class to automatically free memory.
** When the instance of this class falls out of scope, it will free the pointer it
** owns with the delete operator. This class is much like std::auto_ptr but adds
** some useful functionality. Plus, it resembles auto_free.
*/
template <typename T>
class auto_delete
{
    typedef T * t_ptr_t;
    typedef const T * const_t_ptr_t;
    typedef T & t_ref_t;
    typedef const T & const_t_ref_t;
    
    /* The pointer we own. */
    t_ptr_t m_ptr;
    
public:
    /* Default constructor, sets pointer to NULL. */
    inline auto_delete() : m_ptr(0) {}
    
    /* Constructor taking a pointer to own. */
    inline auto_delete(t_ptr_t p) : m_ptr(p) {}
    
    /* Constructor taking a void pointer to own. */
    inline auto_delete(void * p) : m_ptr(reinterpret_cast<t_ptr_t>(p)) {}
    
    /* Copy constructor. */
    inline auto_delete(auto_delete<T> & o) : m_ptr(o.release()) {}
    
    /* Copy constructor for compatible pointer type. */
    template <typename O>
    inline auto_delete(auto_delete<O> & o) : m_ptr(o.release()) {}
    
    /* Destructor. */
    inline ~auto_delete()
    {
        free();
    }
    
    /* 
    ** Pointer control
    ** Clear the pointer we own without deleting it. 
    */
    inline void reset()
    {
        m_ptr = 0;
    }
    
    /* Return the pointer and clear it. */
    inline t_ptr_t release()
    {
        t_ptr_t tmp = m_ptr;
        reset();
        return tmp;
    }
    
    /* 
    ** Free the memory occupied by the pointer.
    ** Does nothing if the pointer is NULL. 
    */
    inline void free()
    {
        if (m_ptr)
        {
            delete m_ptr;
            reset();
        }
    }
    
    /* Return the owned pointer. */
    inline t_ptr_t get() { return m_ptr; }
    
    /* Return the owned pointer. */
    inline const_t_ptr_t get() const { return m_ptr; }
    
    /* 
    ** Changed the owned pointer to a new value.
    ** If there was a previous pointer, it will be freed first. If \a p is NULL or the same
    ** as the currently owned pointer then nothing will be done. 
    */
    inline void set(t_ptr_t p)
    {
        if (p && p != m_ptr)
        {
            free();
        }
        m_ptr = p;
    }
    
    /* Variant of set() taking a void pointer. */
    inline void set(void * p) { set(reinterpret_cast<t_ptr_t>(p)); }
    
    /* Conversion Operators */
    inline operator t_ptr_t () { return m_ptr; }

    inline operator const_t_ptr_t () const { return m_ptr; }
    
    inline operator bool () const { return m_ptr != NULL; }
    
    /* Access operators */
    inline t_ref_t operator * () { return *m_ptr; }
    
    inline const_t_ref_t operator * () const { return *m_ptr; }
    
    inline t_ptr_t operator -> () { return m_ptr; }
    
    inline const_t_ptr_t operator -> () const { return m_ptr; }
    
    /* Assignment operator. */
    inline auto_delete<T> & operator = (t_ptr_t p)
    {
        set(p);
        return *this;
    }
    
    /* Compatible assignment operator. */
    template <typename O>
    inline auto_delete<T> & operator = (auto_free<O> & o)
    {
        set(o.release());
        return *this;
    }

};

#endif /* __auto_free_h__ */

/* EOF */
