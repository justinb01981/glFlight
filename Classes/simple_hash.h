//
//  simple_hash.h
//  gl_flight
//
//  Created by jbrady on 10/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_simple_hash_h
#define gl_flight_simple_hash_h

#include "simple_list.h"

typedef unsigned long simple_hash_key_t;

struct simple_hash_table_t
{
    unsigned long n_buckets;
    // must be last element in struct
    twoway_list_node_t buckets[1];
};

typedef struct simple_hash_table_t simple_hash_table_t;

inline static unsigned long hash(simple_hash_table_t* tbl, unsigned long val)
{
    return val % tbl->n_buckets;
}

inline static simple_hash_table_t *
simple_hash_table_init(unsigned long n_buckets)
{
    size_t tbl_size = sizeof(simple_hash_table_t) + sizeof(twoway_list_node_t)*n_buckets;
    simple_hash_table_t* tbl = (simple_hash_table_t*) malloc(tbl_size);
    if(!tbl) return NULL;
    
    memset(tbl, 0, tbl_size);
    tbl->n_buckets = n_buckets;
    
    return tbl;
}

inline static void
simple_hash_table_destroy(simple_hash_table_t* tbl)
{
    for(unsigned long i = 0; i < tbl->n_buckets; i++)
    {
        twoway_list_node_t* pList = &tbl->buckets[i];
        while(pList->next)
        {
            twoway_list_node_t* pCur = pList->next;
            
            simple_list_remove(pCur, pList);
            simple_list_node_free(pCur);
        }
    }
    
    free(tbl);
}

inline static void *
simple_hash_table_find(simple_hash_table_t* tbl, simple_hash_key_t key)
{
    twoway_list_node_t* pList = &(tbl->buckets[hash(tbl, key)]);
    twoway_list_node_t* pCur = pList->next;
    
    while(pCur)
    {
        if(pCur->val == key)
        {
            return pCur->ptr;
        }
        pCur = pCur->next;
    }
    return NULL;
}

inline static int
simple_hash_table_add(simple_hash_table_t* tbl, void* val, simple_hash_key_t key)
{
    if(simple_hash_table_find(tbl, key) != NULL) return 0; // block duplicates
    
    twoway_list_node_t* pNode = simple_list_node_alloc();
    if(pNode)
    {
        pNode->ptr = val;
        pNode->val = key;
        twoway_list_node_t* pList = &tbl->buckets[hash(tbl, key)];
        
        simple_list_add(pNode, pList);
        return 1;
    }
    else
    {
        return 0;
    }
}

inline static void
simple_hash_table_remove(simple_hash_table_t* tbl, simple_hash_key_t key)
{
    twoway_list_node_t* pList = &tbl->buckets[hash(tbl, key)];
    twoway_list_node_t* pCur = pList->next;
    
    while(pCur)
    {
        twoway_list_node_t* pNext = pCur->next;
        if(pCur->val == key)
        {
            simple_list_remove(pCur, pList);
        }
        pCur = pNext;
    }
}

/*
inline static void
simple_hash_table_remove_2(simple_hash_table_t* tbl, void* ptr)
{
    for(unsigned long i = 0; i < tbl->n_buckets; i++)
    {
        twoway_list_node_t* pList = &tbl->buckets[i];
        simple_list_remove(ptr, pList);   
    }
}
 */

#endif
