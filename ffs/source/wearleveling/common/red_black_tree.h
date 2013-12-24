#ifndef __redblacktree_h__
#define __redblacktree_h__
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
 * $FileName: red_black_tree.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains class declarations for a red black tree implementation.
 *
 *END************************************************************************/

#include "wl_common.h"
#include <math.h>
#include <limits.h>

/* 
** Type definations 
*/

/*
** Red black tree class.
**
** This tree class is specially optimized for the media cache and is not
** intended to be totally general purpose. For one, it assumes that all tree
** node objects, instances of RedBlackTreeNode, are pre-allocated and do not
** have to be freed when they are removed from the tree.
**
** A sentinel is used for root and for nil.  These sentinels are
** created when RedBlackTreeCreate is caled.  root->left should always
** point to the node which is the root of the tree.  nil points to a
** node which should always be black but has aribtrary children and
** parent and no key or info.  The point of using these sentinels is so
** that the root and nil nodes do not require special cases in the code
**
*/
class RedBlackTree
{
public:

    /* Type for the key values used in the red black tree. */
    typedef int64_t Key_t;

    /*
    ** Abstract base class for a node of the red black tree.
    **
    ** Subclasses of this node class must implement the getKey() virtual method
    ** to return the actual key value for that node. The key value must not
    ** change while the node is inserted in the tree, or unexpected behaviour
    ** will occur (i.e., the tree will get all screwed up).
    **
    */
    class Node
    {
    public:
        /* Constructor. */
        Node()
        :   m_isRed(0),
        m_left(0),
        m_right(0),
        m_parent(0)
        {
        }
        
        /* Copy constructor. */
        Node(const Node & other)
        :   m_isRed(other.m_isRed),
        m_left(other.m_left),
        m_right(other.m_right),
        m_parent(other.m_parent)
        {
        }
        
        /* Destructor. */
        virtual ~Node() {}
        
        /* Key value accessor. */
        virtual Key_t getKey() const = 0;
        
        /* Red status accessor. */
        inline bool isRed() const { return m_isRed; }
        
        /* Node link accessors */
        inline Node * getLeft() { return m_left; }
        inline Node * getRight() { return m_right; }
        inline Node * getParent() { return m_parent; }

    protected:
        /* If red=0 then the node is black. */
        int m_isRed; 
        Node * m_left;
        Node * m_right;
        Node * m_parent;

        /* The tree is our friend so it can directly access the node link pointers. */
        friend class RedBlackTree;
    };
    
public:
    /* Constructor. */
    RedBlackTree();
    
    /* Destructor. */
    virtual ~RedBlackTree();

    /* Tree operations */
    void remove(Node * z);
    Node * insert(Node * newNode);
    Node * getPredecessorOf(Node * x) const;
    Node * getSuccessorOf(Node * x) const;
    Node * find(Key_t key) const;
    
    /* Validate certain invariants. */
    void checkAssumptions() const;
    
protected:

    /* Internal node subclass for the root sentinel node. */
    class RootNode : public Node
    {
    public:
        virtual Key_t getKey() const { return LLONG_MAX; }
    };
    
    /* Internal subclass for the nil sentinel node. */
    class NilNode : public Node
    {
    public:
        virtual Key_t getKey() const { return LLONG_MIN; }
    };

    RootNode m_rootStorage;
    NilNode m_nilStorage;
    RootNode * m_root;
    NilNode * m_nil;
    
    void leftRotate(Node * x);
    void rightRotate(Node * y);
    void insertFixUp(Node * z);
    void deleteFixUp(Node * x);
};

#endif /* __redblacktree_h__ */

/* EOF */
