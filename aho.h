#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;

#if AHO_DEBUG
#define DBPRINTF(x) printf x
#else
#define DBPRINTF(x)
#endif

class SearchTree
{
    public:
        
            SearchTree(char val);
            SearchTree(char val, int is_root);
        int AddSearchString(char *str);
        int SearchData(char *data, int len, SearchTree **search_data);
        int FinishTree();

    private:
        
        SearchTree *findLongestSuffix(SearchTree *cur);
        void finishTreeHelper(SearchTree *cur);
        void printPath(SearchTree *cur);
        
        int is_root;
        char v;
        SearchTree *sibling;
        SearchTree *child;
        SearchTree *parent;
        SearchTree *suffix;
        const static char pathTerminator;
};

const char SearchTree::pathTerminator = '$';

SearchTree::SearchTree(char val)
{
    v = val;
    is_root = 0;
    sibling = NULL;
    child = NULL;
    parent = NULL;
    suffix = NULL;
}

SearchTree::SearchTree(char val, int b_root)
{
    v = val;
    is_root = b_root;
    sibling = NULL;
    child = NULL;
    parent = NULL;
    suffix = NULL;
}


int SearchTree::AddSearchString(char *str)
{
    SearchTree *cur = this;
    char *p = str;
    
    while(*p != 0)
    {
        if(!cur->child)
        {
            DBPRINTF(("adding node: %c\n", *p));
            cur->child = new SearchTree(*p);
            cur->child->parent = cur;
            cur = cur->child;
        }
        else
        {
            SearchTree *match = cur->child;
            
            /* search siblings for *p */
            while(match)
            {
                if(match->v == *p)
                {
                    break;
                }
                else
                {
                    match = match->sibling;
                }
            }

            if(match)
            {
                DBPRINTF(("found existing node: %c\n", match->v));
                /* found matching node @ this level */
                cur = match;
            }
            else
            {
                SearchTree *sib = cur->child;
                DBPRINTF(("adding sibling to: %c - %c\n", sib->v, *p));
                match = new SearchTree(*p);
                while(sib->sibling != NULL) sib = sib->sibling;
                sib->sibling = match;
                sib->sibling->parent = sib->parent;
                cur = sib->sibling;
            }
        }
        
        p++;
    }

    /* add terminator */
    cur->child = new SearchTree(pathTerminator);
    cur->child->parent = cur;
    
    return 0;
}

void SearchTree::printPath(SearchTree *cur)
{
    if(cur->parent) printPath(cur->parent);
    DBPRINTF(("/%c", cur->v));
}

SearchTree *SearchTree::findLongestSuffix(SearchTree *cur)
{
    SearchTree *p = cur->parent;

    if(p == NULL)
    {
        return this; /* point at root */
    }
    
    p = p->suffix;
    if(p == NULL)
    {
        /* no suffix, points at root */
        p = this;
    }

    p = p->child;
    while(p)
    {
        if(p->v == cur->v && p != cur)
        {
            DBPRINTF(("found suffix for:"));
            printPath(cur);
            DBPRINTF((" - "));
            printPath(p);
            DBPRINTF(("\n"));

            return p;
        }
        p = p->sibling;
    }
    return this;    /* no suffix, point at root */
}

void SearchTree::finishTreeHelper(SearchTree *root)
{
    SearchTree *cur;

    cur = root;
    while(cur)
    {
        /* avoid cycle @ root */
        if(!cur->is_root)
        {
            cur->suffix = findLongestSuffix(cur);
        }
        cur = cur->sibling;
    }

    cur = root;
    while(cur)
    {
        if(cur->child)
        {
            finishTreeHelper(cur->child);
        }
        cur = cur->sibling;
    }
}

int SearchTree::FinishTree()
{
    finishTreeHelper(this);
    return 0;
}

int SearchTree::SearchData(char *data, int len, SearchTree **search_state)
{
    int di;
    int r = -1;
    SearchTree *cur;
   
    cur = *search_state;
    
    for(di = 0; di < len && cur; di++)
    {
        SearchTree *p;
        if(cur->is_root)
        {
            cur = this->child;
        }
        p = cur;

        while(p)
        {
            if(data[di] == p->v)
            {
                /* partial matching... */
                cur = p->child;
                if(cur->v == pathTerminator)
                {
                    DBPRINTF(("found match for path:"));
                    printPath(cur);
                    DBPRINTF(("\n"));
                    r = di;
                }
                break;
            }
            else
            {
                p = p->sibling;
            }
        }

        if(!p)
        {
            /* no match, reset */
            if(cur->suffix)
            {
                cur = cur->suffix;
            }
            else
            {
                cur = this;
            }
        }
    }

    *search_state = cur;

    return r;
}

#ifdef AHO_TEST

int main(int arg, char **argv)
{
    SearchTree root((char) 0, 1), *search_data;
    char *input1 = "thisthisisamatchingtestofmyalgo", *input2 = "rithm";
    int r;

    root.AddSearchString("test");
    root.AddSearchString("potest");
    root.AddSearchString("every");
    root.AddSearchString("algorithm");
    root.FinishTree();
    /* point state at root */
    search_data = &root;

    r = root.SearchData(input1, strlen(input1), &search_data);
    cout << "r=" << r << endl;
    r = root.SearchData(input2, strlen(input2), &search_data);
    cout << "r=" << r << endl;
    
    return 0;
}

#endif
