#ifndef STM407_H
#define STM407_H

#include "stm100.h"

class stm407 : public stm100
{
public:
    stm407(QStLink & par, const pages_t & Pages);
//    void WriteFlash(uint32_t start , const QByteArray & data) throw (QString);

private:
    void ErasePageSetup(int PageNumber);

};

#endif // STM407_H
