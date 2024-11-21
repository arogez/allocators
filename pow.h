#ifndef POW_H
#define POW_H

const unsigned pow2_ceil(unsigned a)
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
