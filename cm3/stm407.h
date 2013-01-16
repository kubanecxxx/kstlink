#ifndef STM407_H
#define STM407_H

#include "stm100.h"

class stm407 : public stm100
{
public:
    stm407(QStLink & par, const pages_t & Pages);
};

#endif // STM407_H
