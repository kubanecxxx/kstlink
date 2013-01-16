#ifndef STM407_H
#define STM407_H

#include "stm100.h"

class stm407 : public stm100
{
public:
    stm407(QStLink & par, const pages_t & Pages);

    void EraseMass() throw (QString);
    void WriteFlash(uint32_t start , const QByteArray & data) throw (QString);

private:
    void FlashUnlock();
    void FlashLock();
    bool IsBusy();
    bool IsLocked() ;
    void ErasePage(int pageNumber) throw (QString);

};

#endif // STM407_H
