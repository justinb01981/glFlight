//
//  worldElem.m
//  gl_flight
//
//  Created by jbrady on 8/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "gameDebug.h"
#include "worldElem.h"
#include "simple_hash.h"

static void hack_remove_elem_from_all(WorldElem* pElem);

volatile WorldElemListNode* world_elem_list_remove_watch_elem = NULL;

void
world_elem_adjust_geometry_pointers(WorldElem* pElem)
{
    if(pElem->size == sizeof(WorldElem))
    {
        pElem->coords = pElem->coords_;
        pElem->texcoords = pElem->texcoords_;
        pElem->indices = pElem->indices_;
    }
    else
    {
        pElem->coords = ((model_data_extension_t*) &pElem->model_data_extension_stub)->coords_;
        pElem->texcoords = ((model_data_extension_t*) &pElem->model_data_extension_stub)->texcoords_;
        pElem->indices = ((model_data_extension_t*) &pElem->model_data_extension_stub)->indices_;
    }
}

WorldElem*
world_elem_alloc_core(size_t size)
{
    WorldElem *pElem = malloc(size);
    if(pElem) memset(pElem, 0, size);
    
    pElem->size = size;
    
    world_elem_adjust_geometry_pointers(pElem);
    
    return pElem;
}

WorldElem*
world_elem_alloc()
{
    return world_elem_alloc_core(sizeof(WorldElem));
}

WorldElem*
world_elem_alloc_extended_model()
{
    WorldElem* e;
    e = world_elem_alloc_core(sizeof(WorldElem) + sizeof(model_data_extension_t));
    if(e)
    {
        world_elem_adjust_geometry_pointers(e);
    }
    return e;
}

WorldElem*
world_elem_clone(WorldElem* a)
{
    WorldElem *pNew = world_elem_alloc_core(a->size);
    if(pNew)
    {
        memcpy(pNew, a, a->size);
        pNew->size = a->size;
        
        // HACK: copy everything, nulling some fields
        memset(&pNew->stuff, 0, sizeof(pNew->stuff));
        pNew->linked_elem = NULL;
        memset(&pNew->listRefHead, 0, sizeof(pNew->listRefHead));
        pNew->stuff.btree_node[0] = pNew->stuff.btree_node[1] = NULL;
        pNew->elem_id = WORLD_ELEM_ID_INVALID;
        
        world_elem_adjust_geometry_pointers(pNew);
    }
    return pNew;
}

void
world_elem_free(WorldElem* pElem)
{
    int i;
    
    if(pElem->listRefHead.listNext != NULL)
    {
        WorldElemListNode* pNode = pElem->listRefHead.listNext->listElem;
        
        // LEAK
        DBPRINTF(("***WARNING (leak) freeing element STILL remaining in list*** (pNode->type:%d list:%02x)\n",
                  pNode->type, (unsigned int) pElem->listRefHead.listNext->listElem));
        
        // HACK: shouldn't be necessary, should only be in pending-freed list, fix this
        hack_remove_elem_from_all(pElem);
    }
    
    // remove from trees
    for(i = 0; i < 2; i++) if(pElem->stuff.btree_node[i]) ((world_elem_btree_node*) pElem->stuff.btree_node[i])->elem = NULL;
    
    if(pElem->stuff.nametag)
    {
        char *pfree = pElem->stuff.nametag;
        pElem->stuff.nametag = NULL;
        free(pfree);
    }
    
    free(pElem);
}

void
world_elem_replace_fix(WorldElem* pElem)
{
    int i;
    for(i = 0; i < 2; i++) if(pElem->stuff.btree_node[i]) ((world_elem_btree_node*) pElem->stuff.btree_node[i])->elem = pElem;
}

static int
world_elem_add_list_ref(WorldElem* pElem, WorldElemListNode* pHeadNode)
{
    ListNode_t* listCur = &pElem->listRefHead;
    ListNode_t* listCheck = listCur->listNext;
    
    while(listCheck)
    {
        if(listCheck->listElem == pHeadNode)
        {
            DBPRINTF(("***WARNING object in same list TWICE! (%p)***\n", pHeadNode));
        }
        listCheck = listCheck->listNext;
    }
    
    while(listCur->listNext) listCur = listCur->listNext;
    
    listCur->listNext = malloc(sizeof(ListNode_t));
    if(listCur->listNext)
    {
        listCur = listCur->listNext;
        listCur->listElem = pHeadNode;
        listCur->listNext = NULL;
    }
    else
    {
        return 0;
    }
    
    return 1;
}

static int
world_elem_remove_list_ref(WorldElem* pElem, WorldElemListNode* pHeadNode)
{
    ListNode_t* listCur = (ListNode_t*) &pElem->listRefHead;
    ListNode_t* pFree;
    int found = 0;
    
    while(listCur->listNext)
    {
        if(listCur->listNext->listElem == pHeadNode)
        {
            pFree = listCur->listNext;
            listCur->listNext = pFree->listNext;
            free(pFree);
            found = 1;
            continue;
        }
        
        listCur = listCur->listNext;
    }
    
    return found;
}

WorldElemListNode*
world_elem_get_member_list_head(WorldElem* pElem, unsigned int n)
{
    ListNode_t* listCur = pElem->listRefHead.listNext;
    int i = 0;
    while(listCur)
    {
        if(i == n && listCur) return listCur->listElem;
        
        listCur = listCur->listNext;
        i++;
    }
    return NULL;
}

void
world_elem_list_clear(WorldElemListNode* pHeadNode)
{
    WorldElemListNode* pCur = pHeadNode->next;
    
    pHeadNode->next = NULL;
    
    while(pCur)
    {
        WorldElemListNode* pFree = pCur;
        
        if(pFree == world_elem_list_remove_watch_elem) world_elem_list_remove_watch_elem = NULL;
        
        world_elem_remove_list_ref(pCur->elem, pHeadNode);
        
        pCur = pCur->next;
        free(pFree);
    }
}

WorldElemListNode*
world_elem_list_add_core(WorldElem* pElem, WorldElemListNode* pHeadNode, int check, ListType type)
{
    // first see if elem exists in the list...
    if(check)
    {
        WorldElemListNode* found = world_elem_list_find_elem(pElem, pHeadNode);
        if(found) return found;
    }
    
    WorldElemListNode* pNode = (WorldElemListNode*) malloc(sizeof(WorldElemListNode));
    if(!pNode) return NULL;
    
    memset(pNode, 0, sizeof(*pNode));
    
    // disabing for now, don't look at the head node to determine list type
    //pHeadNode->type = type;
    
    pNode->elem = pElem;
    pNode->next = pHeadNode->next;
    pNode->type = type;
    pHeadNode->next = (struct WorldElemListNode*) pNode;
    
    // add pointer to containing list in elem
    world_elem_add_list_ref(pElem, pHeadNode);
    
    // add to hash, if this list has a hash-table assigned
    if(pHeadNode->hash_ptr && pElem->elem_id > 0)
    {
        simple_hash_table_add(pHeadNode->hash_ptr, pNode, pElem->elem_id);
    }
    
    return pNode;
}

WorldElemListNode*
world_elem_list_add(WorldElem* pElem, WorldElemListNode* pHeadNode)
{
    // TODO: dont require checking when adding an object, really slows things down
    return world_elem_list_add_core(pElem, pHeadNode, 0, LIST_TYPE_UNKNOWN);
}

WorldElemListNode*
world_elem_list_add_fast(WorldElem* pElem, WorldElemListNode* pHeadNode, ListType type)
{
    return world_elem_list_add_core(pElem, pHeadNode, 0, type);
}

WorldElemListNode*
world_elem_list_find_elem(WorldElem* pElem, WorldElemListNode* pHeadNode)
{
    WorldElemListNode *pCur = pHeadNode->next;
    while(pCur)
    {
        if(pCur->elem == pElem) return pCur;
        pCur = pCur->next;
    }
    return NULL;
}

void
world_elem_list_remove(WorldElem* pElem, WorldElemListNode* pHeadNode)
{
    WorldElemListNode* pPrev = pHeadNode;
    WorldElemListNode* pCur = pHeadNode->next;
    int removed = 0;
    
    while(pCur)
    {
        if(pCur->elem == pElem)
        {
            WorldElemListNode* pFree = pCur;
            
            if(world_elem_list_remove_watch_elem == pFree) world_elem_list_remove_watch_elem = NULL;
            
            if(pHeadNode->hash_ptr) simple_hash_table_remove(pHeadNode->hash_ptr, pElem->elem_id);
            
            world_elem_remove_list_ref(pCur->elem, pHeadNode);
            
            pPrev->next = pCur->next;
            
            free(pFree);
            removed++;
            
            break;
        }
        
        pPrev = pCur;
        pCur = pCur->next;
    }
    
    if(removed > 1)
    {
        printf("%s:%d WARNING: Object in list TWICE!\n", __FILE__, __LINE__);
    }
}

WorldElemListNode*
world_elem_list_find(int elem_id, WorldElemListNode* pHeadNode)
{
    WorldElemListNode* pCur = pHeadNode->next;
    
    if(pHeadNode->hash_ptr)
    {
        return simple_hash_table_find(pHeadNode->hash_ptr, elem_id);
    }
    
    while(pCur)
    {
        if(pCur->elem->elem_id == elem_id) return pCur;
        
        pCur = pCur->next;
    }
    return NULL;
}

WorldElemListNode* world_elem_list_sort_1_resume_elem = NULL;

void
world_elem_list_sort_1(WorldElemListNode* pHeadNode,
                       world_elem_list_sort_compare_func compare_func,
                       int resume_sort, int max_steps)
{
    WorldElemListNode* pPrev = pHeadNode;
    WorldElemListNode* pCur = pHeadNode->next;
    
    if(!resume_sort) world_elem_list_sort_1_resume_elem = NULL;
    
    if(world_elem_list_sort_1_resume_elem)
    {
        // resume previous partial sort
        pPrev = world_elem_list_sort_1_resume_elem;
        pCur = world_elem_list_sort_1_resume_elem->next;
    }
    else pCur = pHeadNode->next;
    
    int n = 0;
    
    while(pCur && n < max_steps)
    {
        if(pCur->next && !(*compare_func)(pCur->elem, pCur->next->elem))
        {
            // swap
            WorldElemListNode* pTmp = pCur->next;
            pCur->next = pCur->next->next;
            pPrev->next = pTmp;
            pTmp->next = pCur;
            
            pCur = pPrev->next;
        }
        
        pPrev = pPrev->next;
        pCur = pCur->next;
        n++;
    }
    
    world_elem_list_sort_1_resume_elem = pPrev;
    // HACK: list is not completely sorted yet
}

void
world_elem_list_add_sorted(WorldElemListNode* pHeadNode,
                           WorldElemListNode* pSkipListNode,
                           WorldElem* pElem,
                           world_elem_list_sort_compare_func compare_func)
{
    WorldElemListNode* pInsert;
    WorldElemListNode* pSearchBegin = pHeadNode;

    if(pSkipListNode)
    {
        WorldElemListNode* pSkip = pSkipListNode->next;
        while(pSkip)
        {
            WorldElemListNode* pOrigListNode = (WorldElemListNode*) pSkip->elem;
            
            if(compare_func(pElem, pOrigListNode->elem)) break;
            
            // skip elems < pElem
            pSearchBegin = pOrigListNode;
            pSkip = pSkip->next;
        }
    }
    
    WorldElemListNode* pDest = pSearchBegin;
    
    // skip elems < pElem
    while(pDest->next && !compare_func(pElem, pDest->next->elem))
    {
        pDest = pDest->next;
    }
    
    pInsert = world_elem_list_add(pElem, pHeadNode);
    if(pInsert)
    {
        pHeadNode->next = pInsert->next;
        pInsert->next = pDest->next;
        pDest->next = pInsert;
    }
}

void
world_elem_list_sort_1_with_max(WorldElemListNode* pHeadNode, world_elem_list_sort_compare_func compare_func,
                                int max_steps)
{
    WorldElemListNode* pPrev = pHeadNode;
    WorldElemListNode* pCur = pHeadNode->next;
    
    while(pCur && max_steps > 0)
    {
        if(pCur->next && !(*compare_func)(pCur->elem, pCur->next->elem))
        {
            // swap
            WorldElemListNode* pTmp = pCur->next;
            pCur->next = pCur->next->next;
            pPrev->next = pTmp;
            pTmp->next = pCur;
            
            pCur = pPrev->next;
        }
        
        pPrev = pPrev->next;
        pCur = pCur->next;
        max_steps--;
    }
    // HACK: list is not completely sorted yet
}

void
world_elem_list_sort_first_elem(WorldElemListNode* pHeadNode, world_elem_list_sort_compare_func compare_func)
{
    WorldElemListNode* pPrev = pHeadNode;
    WorldElemListNode* pCur = pHeadNode->next;
    
    while(pCur)
    {
        if(pCur->next && !(*compare_func)(pCur->elem, pCur->next->elem))
        {
            // swap
            WorldElemListNode* pTmp = pCur->next;
            pCur->next = pCur->next->next;
            pPrev->next = pTmp;
            pTmp->next = pCur;
            
            pCur = pPrev->next;
        }
        else
        {
            break;
        }
        
        pPrev = pPrev->next;
        pCur = pCur->next;
    }
    // HACK: list is not completely sorted yet
}

static void
hack_remove_elem_from_all(WorldElem* pElem)
{
    // walk list-back references and remove from all LIST_TYPE_REGION lists
    WorldElemListNode* pListHead = NULL;
    int i = 0;
    
    do
    {
        pListHead = world_elem_get_member_list_head(pElem, i);
        if(pListHead)
        {
            world_elem_list_remove(pElem, pListHead);
        }   
        
        i++;
    } while(pListHead);
}

void
world_elem_list_build_skip_list(WorldElemListNode* pHeadNode, WorldElemListNode* pSkipListHead, int nSkip)
{
    WorldElemListNode* pCur = pHeadNode->next;
    WorldElemListNode* pDest = pSkipListHead;
    int n = nSkip;
    
    while(pCur)
    {
        if(n == 0)
        {
            n = nSkip;
            pDest->next = malloc(sizeof(WorldElemListNode));
            if(!pDest->next) break;
            
            memset(pDest->next, 0, sizeof(WorldElemListNode));
            pDest->next->elem = (WorldElem*) pCur;
            pDest = pDest->next;
        }
        else
        {
            n--;
        }
        pCur = pCur->next;
    }
}

void
world_elem_list_remove_skip_list(WorldElemListNode* pSkipListHead, WorldElem* pElem)
{
    WorldElemListNode* pCur = pSkipListHead;
    
    while(pCur)
    {
        if(pCur->next)
        {
            WorldElem* pCompareElem = ((WorldElemListNode*) pCur->next->elem)->elem;
            
            if(pCompareElem == pElem)
            {
                WorldElemListNode* pFree = pCur->next;
                pCur->next = pCur->next->next;
                free(pFree);
            }
        }
        pCur = pCur->next;
    }
    
}

void
world_elem_list_release_skip_list(WorldElemListNode* pSkipListHead)
{
    while(pSkipListHead->next)
    {
        world_elem_list_remove_skip_list(pSkipListHead, ((WorldElemListNode*) pSkipListHead->next->elem)->elem);
    }
}

void
world_elem_set_nametag(WorldElem* elem, char* tag)
{
    char* nametag = malloc(sizeof(char)*32);
    if(nametag)
    {
        int i;
        for(i = 0; i < 31 && tag[i]; i++)
            nametag[i] = tag[i];
        nametag[i] = '\0';
        if(elem->stuff.nametag)
        {
            char *pfree = elem->stuff.nametag;
            elem->stuff.nametag = nametag;
            free(pfree);
        }
        else
        {
            elem->stuff.nametag = nametag;
        }
        
    }
}

volatile unsigned int world_elem_btree_ptr_idx = 0;

unsigned int
world_elem_btree_ptr_idx_set(unsigned int i)
{
    unsigned int prev = world_elem_btree_ptr_idx;
    world_elem_btree_ptr_idx = i;
    return prev;
}

void
world_elem_btree_insert(world_elem_btree_node* root, WorldElem* elem, float order)
{
    if(elem->stuff.btree_node[world_elem_btree_ptr_idx] != NULL) return;
    
    if(order > root->order)
    {
        if (root->left) world_elem_btree_insert(root->left, elem, order);
        else
        {
            root->left = (world_elem_btree_node*) malloc(sizeof(world_elem_btree_node));
            if(root->left)
            {
                root->left->left = NULL;
                root->left->right = NULL;
                root->left->elem = elem;
                root->left->order = order;
                elem->stuff.btree_node[world_elem_btree_ptr_idx] = root->left;
            }
        }
    }
    else {
        if (root->right) world_elem_btree_insert(root->right, elem, order);
        else
        {
            root->right = (world_elem_btree_node*) malloc(sizeof(world_elem_btree_node));
            if(root->right)
            {
                root->right->left = NULL;
                root->right->right = NULL;
                root->right->elem = elem;
                root->right->order = order;
                elem->stuff.btree_node[world_elem_btree_ptr_idx] = root->right;
            }
        }
    }
}

struct world_elem_btree_walk_core_state
{
    void ((*walk_func)(WorldElem* elem, float order));
          
} g_world_elem_btree_walk_core_state;

void
world_elem_btree_walk_core(world_elem_btree_node* root)
{
    if(root->left) world_elem_btree_walk_core(root->left);
    //if(root->elem && (root->elem->btree_node[0] == NULL && root->elem->btree_node[1] == NULL)) assert(0);
    
    if(root->elem) g_world_elem_btree_walk_core_state.walk_func(root->elem, root->order);
    if(root->right) world_elem_btree_walk_core(root->right);
}

void
world_elem_btree_walk(world_elem_btree_node* root, void ((*walk_func)(WorldElem* elem, float order)))
{
    g_world_elem_btree_walk_core_state.walk_func = walk_func;
    world_elem_btree_walk_core(root);
}

void
world_elem_btree_destroy(world_elem_btree_node* root)
{
    if(!root) return;
    if(root->left) world_elem_btree_destroy(root->left);
    if(root->right) world_elem_btree_destroy(root->right);
    
    if(root->elem) {
        root->elem->stuff.btree_node[world_elem_btree_ptr_idx] = NULL;
        root->elem = NULL;
    }
    
    free(root);
}

void
world_elem_btree_destroy_root(world_elem_btree_node* root)
{
    if(root->right) {
        world_elem_btree_destroy(root->right);
        root->right = NULL;
    }
    if(root->left) {
        world_elem_btree_destroy(root->left);
        root->left = NULL;
    }
}

void
world_elem_btree_remove(world_elem_btree_node* root, WorldElem* elem, float order)
{
    /*
    if(root->order == order && elem == root->elem)
    {
        // just invalidate for now
        root->elem = NULL;
        return;
    }
    
    if(order > root->order)
    {
        if(root->left)
        {
            world_elem_btree_remove(root->left, elem, order);
        }
    }
    else
    {
        if(order <= root->order)
        {
            if(root->right)
            {
                world_elem_btree_remove(root->right, elem, order);
            }
        }
    }
     */
    if(((world_elem_btree_node*)elem->stuff.btree_node[world_elem_btree_ptr_idx])) ((world_elem_btree_node*)elem->stuff.btree_node[world_elem_btree_ptr_idx])->elem = NULL;
    elem->stuff.btree_node[world_elem_btree_ptr_idx] = NULL;
}

void
world_elem_btree_remove_all(world_elem_btree_node* root, WorldElem* elem, float order)
{
    if(((world_elem_btree_node*)elem->stuff.btree_node[0])) ((world_elem_btree_node*)elem->stuff.btree_node[0])->elem = NULL;
    elem->stuff.btree_node[0] = NULL;
    
    if(((world_elem_btree_node*)elem->stuff.btree_node[1])) ((world_elem_btree_node*)elem->stuff.btree_node[1])->elem = NULL;
    elem->stuff.btree_node[1] = NULL;
}
