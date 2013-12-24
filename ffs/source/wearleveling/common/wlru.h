#if !defined(__wlru_h__)
#define __wlru_h__
/*HEADER**********************************************************************
 *****************************************************************************
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Freescale Confidential and Proprietary - use of this software is
 * governed by the Freescale MQX RTOS License distributed with this
 * material. See the MQX_RTOS_LICENSE file distributed for more
 * details.
 *
 *****************************************************************************
 *
 ** THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
 ** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 ** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 ** IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 ** INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 ** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 ** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 ** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 ** STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 ** IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 ** THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************
 *
 ** $FileName: wlru.h$
 ** $Version : 3.8.0.1$
 ** $Date    : Aug-9-2012$
 *
 ** Comments:
 *
 **   This file contains declaration of a weighted LRU class.
 *
 *END************************************************************************/

#include "double_list.h"

/** Type definations */

/*
** Class to manage a weighted LRU list.
**
** This class maintains a doubly linked list of node sorted in LRU order, which is
** equivalent to FIFO order. In addition to strict LRU ordering, the class features
** support for weighted LRU ordering. That is, highly weighted objects have a
** higher "recency" than low weighted objects. This allows the user to retain objects
** with a high cost of loading or known high access frequency more than other objects.
**
** The constuctor takes a maximum window size parameter. If the computed window size
** is larger than the maximum, then weights will be scaled down to fit. Pass 0 to
** disable weighting entirely and enforce strict LRU ordering.
*/
class WeightedLRUList : public DoubleList
{
public:

    /*
    ** Abstract base class for a node in an LRU list.
    */
    class Node : public DoubleList::Node
    {
    public:

        /* Default constructor. */
        Node();
        
        /* Pure virtual method that is used to determine if the node is valid. */
        virtual bool isNodeValid() const = 0;
        
        /* Pure virtual method to return the node's weight value. */
        virtual int getWeight() const = 0;

    };

public:
    /* Constructor. */
    WeightedLRUList(int minWeight, int maxWeight, unsigned windowSize);
    
    /* 
    ** List operations 
    **
    ** Insert node into the list at or near the tail/MRU position.
    ** 
    ** The node is nominally inserted at the tail/MRU position, but can be moved
    ** further away from the tail by using the weighting scheme.
    ** If weights are being used in this LRU, then a weight of m_maxWeight on node will cause
    ** insertion at the tail, and a weight of zero on node will cause insertion
    ** at the head.
    */
    void insert(Node * node);
    
    /* Get the oldest entry (head/LRU) in the list. */
    Node * select();

    /* Put node back on the head/LRU of the list. */
    void deselect(Node * node);

protected:
    /* Maximum weight value. */
    int m_maxWeight;
    
    /* Scale multiplier. */
    int m_scaleNumerator;
    
    /* Scale divider. */
    int m_scaleDenominator;
};

#endif /** __wlru_h__ */

/* EOF */
