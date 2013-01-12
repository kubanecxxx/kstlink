#ifndef STM100_H
#define STM100_H

#include <inttypes.h>

class QArm3;
class stm100
{
public:
    stm100(QArm3 & par, int pageSize, int pageCount);
    bool ErasePage(int pageNumber);
    bool EraseMass();
    bool EraseRange(uint32_t start, uint32_t stop, bool verify = false);
    bool IsLocked() ;
    bool VerifyErased(int PageNum = -1);

private:
    QArm3 & par;
    const int Size;
    const int Count;
    bool locked;
    bool busy;
    bool IsBusy();

    void FlashUnlock();
    void FlashLock();
};

#endif // STM100_H
