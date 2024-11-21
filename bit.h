/* bit.h - operations on bits */

#ifndef BIT_H
#define BIT_H

#define bit(A)                  (1 << (A))
#define bit_set(A,B)            (A[(B/32)] |= 1 << (B%32))
#define bit_clear(A,B)          (A[(B/32)] &= ~(1 << (B%32)))
#define bit_check(A,B)          (A[(B/32)] & (1 << (B%32)))
#define bit_switch(A, B)        bit_check(A, B) ? bit_clear(A, B) : bit_set(A, B)

const uint8_t lowest_bit_index(const uint32_t a)
{
        /* hashtable. see https://en.wikipedia.org/wiki/De_Bruijn_sequence */
        static const uint32_t bit_position_lookup[32] = 
        {
                0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
                31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
        };
        
        return bit_position_lookup[((uint32_t)((a & -a) * 0x077CB531U)) >> 27];
}

#endif
