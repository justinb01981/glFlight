#ifndef __BOUNDARY_HELPER_H__
#define __BOUNDARY_HELPER_H__

#define BSEARCH_MIN(x, y) ((x) < (y)? (x): (y))

static inline
char* mymemsearch(char* haystack, unsigned long haystack_len,
                  char* needle, unsigned long needle_len)
{
    unsigned long i = 0;

    while(i <= (haystack_len - needle_len))
    {
        if(memcmp(haystack + i, needle, needle_len) == 0)
        {
            return haystack + i;
        }
        i++;
    }

    return NULL;
}

static char *
memmem_boundary_helper(char *haystack, size_t hlen, char *needle, size_t nlen)
{
    int needle_first;
    char *p = haystack;
    size_t plen = hlen;

    if (!nlen)
        return NULL;

    needle_first = *(unsigned char *)needle;

    while (plen >= nlen && (p = (char *) memchr(p, needle_first, plen - nlen + 1)))
    {
        if (!memcmp(p, needle, nlen))
            return p;

        p++;
        plen = hlen - (p - haystack);
    }

    return NULL;
}

typedef void (*read_until_boundary_fp)(void* ptr, char* data, size_t datalen);

typedef struct {
    char *boundary;
    char *tmp;
    char *incbuf;
    size_t inclen;
    size_t tmplen;
    size_t blen;
    size_t skip;
    int found;
    size_t found_offset;
    read_until_boundary_fp func;
    void *ptr;
} boundary_search_state;

static int
read_until_boundary_init(boundary_search_state* state, char* boundary,
                         read_until_boundary_fp func, void *ptr)
{
    memset(state, 0, sizeof(*state));
    state->skip = state->blen = strlen(boundary);
    if(state->blen < 0) return -1;

    state->boundary = (char *) malloc(strlen(boundary)+1);
    if(!state->boundary) return -1;

    strcpy(state->boundary, boundary);
    state->func = func;
    state->tmplen = strlen(state->boundary)*2 + 1;
    state->tmp = (char *) malloc(state->tmplen);
    if(!state->tmp) return -1;

    /* hack */
    state->incbuf = (char *) malloc(1);
    if(!state->incbuf) return -1;
    state->inclen;

    state->tmp[state->tmplen-1] = '\0';

    state->ptr = ptr;

    return 0;
}

static void
read_until_boundary_uninit(boundary_search_state* state)
{
    /* flush */
    if(state->inclen > 0 && !state->found)
    {
        state->func(state->ptr, state->incbuf, state->inclen);
    }
    if(state->boundary) free(state->boundary);
    if(state->tmp) free(state->tmp);
    if(state->incbuf) free(state->incbuf);
}

static int
read_until_boundary(char *input, size_t input_len, boundary_search_state* s)
{
    char *newbuf = (char *) malloc(s->inclen + input_len);
    if(!newbuf) return -1;

    memcpy(newbuf, s->incbuf, s->inclen);
    memcpy(newbuf+s->inclen, input, input_len);
    free(s->incbuf);
    s->incbuf = newbuf;
    s->inclen = s->inclen + input_len;

    char *p = s->incbuf;
    while (p - s->incbuf < s->inclen)
    { 
        size_t remaining = s->inclen - (p - s->incbuf);
        size_t readlen = BSEARCH_MIN(s->blen, remaining);
        char *dst = s->tmp+s->blen;

        memset(dst, 0, s->blen);
        memcpy(dst, p, readlen);

        char *boundary_found =
            memmem_boundary_helper(s->tmp, s->tmplen-1, s->boundary, s->blen);
        if(boundary_found)
        {
            s->func(s->ptr, s->tmp, boundary_found - s->tmp);
            s->found = 1;
            return 1;
        }

        if(readlen < s->blen) break;

        size_t printlen = readlen;
        while(s->skip > 0 && printlen > 0)
        {
            s->skip--;
            printlen--;
        }

        if(s->skip == 0) s->func(s->ptr, s->tmp, printlen);

        memmove(s->tmp, s->tmp+readlen, (s->tmplen - 1) - readlen);
        p += readlen;
    }

    size_t consumed = p - s->incbuf;
    memmove(s->incbuf, s->incbuf+consumed, s->inclen-consumed);
    s->inclen -= consumed;

    return 0;
}

#endif
