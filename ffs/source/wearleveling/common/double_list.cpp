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
 * $FileName: DoubleList.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains implementation of the doulbly linked list class.
 *
 *END************************************************************************/

#include "wl_common.h"
#include "double_list.h"

#if defined(__ghs__)
#pragma ghs section text=".init.text"
#endif /* defined(__ghs__) */

#if defined(__CODEWARRIOR__)
// TODO: #pragma ghs section text=".init.text"
#endif /* defined(__CODEWARRIOR__) */

DoubleList::DoubleList()
:   m_head(0),
    m_tail(0),
    m_size(0)
{
}

#if defined(__ghs__)
#pragma ghs section text=default
#endif /* defined(__ghs__) */

#if defined(__CODEWARRIOR__)
// TODO: pragma ghs section text="default"
#endif /* defined(__CODEWARRIOR__) */

DoubleList::Node::Node()
:   m_prev(NULL),
    m_next(NULL)
{
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : insertAfter
* Returned Value   : void
* Comments         :
*   This function insert a node into a list after another node
*
*END*--------------------------------------------------------------------*/
void DoubleList::insertAfter
(
    /* [IN] A new node */
    Node * node, 
    
    /* [IN] Another node */
    Node * insertPos
)
{ /* Body */
    assert(node);

    if (!insertPos)
    {
        /* Inserting at the head/LRU of the list. */
        node->m_prev = NULL;
        node->m_next = m_head;
        
        /* Update list head. */
        if (m_head)
        {
            m_head->m_prev = node;
        } /* Endif */
        m_head = node;
        
        /* Update tail. Special case for single item list. */
        if (!m_tail)
        {
            m_tail = node;
        } /* Endif */
    }
    else
    {   
        /* Insert after insertPos. */
        node->m_next = insertPos->m_next;
        if (node->m_next)
        {
            node->m_next->m_prev = node;
        } /* Endif */
        insertPos->m_next = node;
        node->m_prev = insertPos;
        
        /* Update list tail. */
        if (insertPos == m_tail)
        {
            m_tail = node;
        } /* Endif */
    } /* Endif */
    
    ++m_size;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : insertBefore
* Returned Value   : void
* Comments         :
*   This function insert a node into a list before another node
*
*END*--------------------------------------------------------------------*/
void DoubleList::insertBefore
(
    /* [IN] A new node */
    Node * node, 
    
    /* [IN] Another node */
    Node * insertPos
)
{ /* Body */
    if (insertPos)
    {
        /* Insert after the node previous to the given position. */
        insertAfter(node, insertPos->getPrevious());
    }
    else
    {
        /* The insert position was NULL, so put at the end of the list. */
        insertBack(node);
    } /* Endif */
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : remove
* Returned Value   : void
* Comments         :
*   This function removes a node from list
*
*END*--------------------------------------------------------------------*/
void DoubleList::remove
(
    /* [IN] Removed node */
    Node * node
)
{
    /* Disconnect from list. */
    if (node->m_prev)
    {
        node->m_prev->m_next = node->m_next;
    } /* Endif */
    if (node->m_next)
    {
        node->m_next->m_prev = node->m_prev;
    } /* Endif */
    
    if (m_head == node)
    {
        m_head = node->m_next;
    } /* Endif */
    
    if (m_tail == node)
    {
        m_tail = node->m_prev;
    } /* Endif */
    
    /* Clear node links. */
    node->m_next = NULL;
    node->m_prev = NULL;
    
    --m_size;
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : clear
* Returned Value   : void
* Comments         :
*   This function deletes all node from list
*
*END*--------------------------------------------------------------------*/
void DoubleList::clear()
{
    m_head = NULL;
    m_tail = NULL;
    m_size = 0;
}

/* EOF */
