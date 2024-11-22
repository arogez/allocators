#ifndef POW_H
#define POW_H

/* round up to the next highest power of 2 of a 32-bit integer */
const unsigned pow2_roundup(unsigned a)
{
        a--;
        a |= a >> 1;
        a |= a >> 2;
        a |= a >> 4;
        a |= a >> 8;
        a |= a >> 16;
        a++;

        return a;
}

#endif
