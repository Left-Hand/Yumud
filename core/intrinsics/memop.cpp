#include "memop.h"
#include <stddef.h>

void memset_word_aligned(uint32_t * s, uint32_t c, unsigned int quantity){

}

void *memset(void *s, int c, unsigned int count){

    static constexpr int LBLOCKSIZE      = (int)(sizeof(long));

    #define UNALIGNED(X)    ((long)X & (LBLOCKSIZE - 1))
    #define TOO_SMALL(LEN)  ((LEN) < LBLOCKSIZE)

    unsigned int i;
    char *m = (char *)s;
    unsigned long buffer;
    unsigned long *aligned_addr;
    unsigned int d = c & 0xff;  /* To avoid sign extension, copy C to an
                                unsigned variable.  */

    if (!TOO_SMALL(count) && !UNALIGNED(s))
    {
        /* If we get this far, we know that n is large and m is word-aligned. */
        aligned_addr = (unsigned long *)s;

        /* Store D into each char sized location in BUFFER so that
         * we can set large blocks quickly.
         */
        if constexpr(LBLOCKSIZE == 4){
            buffer = (d << 8) | d;
            buffer |= (buffer << 16);
        }else{
            buffer = 0;
            for (i = 0; i < LBLOCKSIZE; i ++)
                buffer = (buffer << 8) | d;
        }

        while (count >= LBLOCKSIZE * 4){
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            count -= 4 * LBLOCKSIZE;
        }

        while (count >= LBLOCKSIZE){
            *aligned_addr++ = buffer;
            count -= LBLOCKSIZE;
        }

        /* Pick up the remainder with a bytewise loop. */
        m = (char *)aligned_addr;
    }

    while (count--){
        *m++ = (char)d;
    }

    return s;

#undef UNALIGNED
#undef TOO_SMALL
}


void *memcpy(void * __restrict dst, const void * __restrict src, unsigned int count){
    #define UNALIGNED(X, Y) \
        (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

    static constexpr int BIGBLOCKSIZE    = (sizeof (long) << 2);
    static constexpr int LITTLEBLOCKSIZE = (sizeof (long));

    char *dst_ptr = (char *)dst;
    const char *src_ptr = (const char *)src;
    long *aligned_dst;
    const long *aligned_src;
    int len = count;

    /* If the size is small, or either SRC or DST is unaligned,
    then punt into the byte copy loop.  This should be rare. */
    if (!(len < BIGBLOCKSIZE) && !UNALIGNED(src_ptr, dst_ptr)){
        aligned_dst = (long *)dst_ptr;
        aligned_src = (const long *)src_ptr;

        /* Copy 4X long words at a time if possible. */
        while (len >= BIGBLOCKSIZE){
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            len -= BIGBLOCKSIZE;
        }

        /* Copy one long word at a time if possible. */
        while (len >= LITTLEBLOCKSIZE){
            *aligned_dst++ = *aligned_src++;
            len -= LITTLEBLOCKSIZE;
        }

        /* Pick up any residual with a byte copier. */
        dst_ptr = (char *)aligned_dst;
        src_ptr = (const char *)aligned_src;
    }

    while (len--)
        *dst_ptr++ = *src_ptr++;

    return dst;
#undef UNALIGNED
}




void *memmove(void *dst, const void *src, size_t count) {
    #define UNALIGNED(X, Y) \
        (((long)(X) & (sizeof(long) - 1)) | ((long)(Y) & (sizeof(long) - 1)))
    
    static const int BIGBLOCKSIZE = (sizeof(long) << 2);
    static const int LITTLEBLOCKSIZE = sizeof(long);
    
    char *d = (char *)dst;
    const char *s = (const char *)src;
    int len = (int)count;
    
    /* 反向拷贝：目标在源后面且重叠 */
    if (d > s && d < s + count) {
        /* 从尾部开始 */
        d += len;
        s += len;
        
        /* 块拷贝优化（反向） */
        if (!(len < BIGBLOCKSIZE) && !UNALIGNED(s - count, d - count)) {
            long *ld = (long *)d;
            const long *ls = (const long *)s;
            
            /* 处理不对齐的尾部 */
            int tail = len % LITTLEBLOCKSIZE;
            while (tail--) {
                *--d = *--s;
                len--;
            }
            
            /* 重新赋值指针 */
            ld = (long *)d;
            ls = (const long *)s;
            
            /* 4倍块反向拷贝 */
            while (len >= BIGBLOCKSIZE) {
                *--ld = *--ls;
                *--ld = *--ls;
                *--ld = *--ls;
                *--ld = *--ls;
                len -= BIGBLOCKSIZE;
            }
            
            /* 单块反向拷贝 */
            while (len >= LITTLEBLOCKSIZE) {
                *--ld = *--ls;
                len -= LITTLEBLOCKSIZE;
            }
            
            d = (char *)ld;
            s = (const char *)ls;
        }
        
        /* 逐字节反向拷贝 */
        while (len--) {
            *--d = *--s;
        }
    } 
    /* 正向拷贝：不重叠或目标在源前面 */
    else {
        /* 块拷贝优化（正向） */
        if (!(len < BIGBLOCKSIZE) && !UNALIGNED(s, d)) {
            long *ld = (long *)d;
            const long *ls = (const long *)s;
            
            while (len >= BIGBLOCKSIZE) {
                *ld++ = *ls++;
                *ld++ = *ls++;
                *ld++ = *ls++;
                *ld++ = *ls++;
                len -= BIGBLOCKSIZE;
            }
            
            while (len >= LITTLEBLOCKSIZE) {
                *ld++ = *ls++;
                len -= LITTLEBLOCKSIZE;
            }
            
            d = (char *)ld;
            s = (const char *)ls;
        }
        
        /* 逐字节正向拷贝 */
        while (len--) {
            *d++ = *s++;
        }
    }
    
    return dst;
    #undef UNALIGNED
}