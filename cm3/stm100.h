#ifndef STM100_H
#define STM100_H

#include <inttypes.h>
#include <QByteArray>

class QArm3;
class QStLink;
class stm100
{
public:
    stm100(QStLink & par, int pageSize, int pageCount);
    bool ErasePage(int pageNumber);
    bool EraseMass();
    bool EraseRange(uint32_t start, uint32_t stop, bool verify = false);
    bool IsLocked() ;
    bool VerifyErased(int PageNum = -1);

    bool WriteFlash(uint32_t start , const QByteArray & data);


private:
    QStLink & par;
    const int Size;
    const int Count;
    bool locked;
    bool busy;
    QByteArray loader;

    void FlashUnlock();
    void FlashLock();
    bool IsBusy();

};

#endif // STM100_H
