//
//  simple_list.h
//  gl_flight
//
//  Created by jbrady on 10/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_simple_list_h
#define gl_flight_simple_list_h

#include <string.h>
#include <stdlib.h>

struct twoway_list_node_t
{
    struct twoway_list_node_t* next;
    struct twoway_list_node_t* prev;
    
    void* ptr;
    unsigned long val;
};

typedef struct twoway_list_node_t twoway_list_node_t;

inline static void
simple_list_init(twoway_list_node_t* head)
{
    memset(head, 0, sizeof(*head));
}

inline static twoway_list_node_t*
simple_list_node_alloc()
{
    twoway_list_node_t* n = (twoway_list_node_t*) malloc(sizeof(*n));
    if(n) memset(n, 0, sizeof(twoway_list_node_t));
    return n;
}

inline static void
simple_list_node_free(twoway_list_node_t* p)
{
    free(p);
}

inline static void
simple_list_add(twoway_list_node_t* n, twoway_list_node_t* head)
{
    twoway_list_node_t* prev = head;
    twoway_list_node_t* cur = head->next;
    
    while(cur)
    {
        prev = prev->next;
        cur = cur->next;
    }
    
    prev->next = n;
    n->prev = prev;
    n->next = NULL;
}

inline static void
simple_list_remove(twoway_list_node_t* n, twoway_list_node_t* head)
{
    twoway_list_node_t* prev = head;
    twoway_list_node_t* cur = head->next;
    
    while(cur)
    {
        if(cur == n)
        {
            prev->next = cur->next;
            cur = cur->next;
        }
        else
        {
            prev = prev->next;
            cur = cur->next;
        }
    }
}

inline static twoway_list_node_t*
simple_list_find_ptr(void* n, twoway_list_node_t* head)
{
    twoway_list_node_t* cur = head->next;
    
    while(cur)
    {
        if(cur->ptr == n) return cur;
        
        cur = cur->next;
    }
    
    return NULL;
}

inline static twoway_list_node_t*
simple_list_find_val(unsigned long val, twoway_list_node_t* head)
{
    twoway_list_node_t* cur = head->next;
    
    while(cur)
    {
        if(cur->val == val) return cur;
        
        cur = cur->next;
    }
    
    return NULL;
}
                              
#endif
