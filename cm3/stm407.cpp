#include "stm407.h"
#include "stm32f4xx.h"

stm407::stm407(QStLink & par, const pages_t & Pages):
    stm100(par,Pages)
{

}
