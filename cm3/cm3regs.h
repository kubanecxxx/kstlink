#ifndef CM3REGS_H
#define CM3REGS_H

#include <inttypes.h>

class cm3Regs
{
public:
    cm3Regs();

    uint32_t r[16];
    uint32_t main_sp;
    uint32_t s;


private:
    int size;
    uint8_t * data;
};

#endif // CM3REGS_H
