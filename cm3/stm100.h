#ifndef STM100_H
#define STM100_H

#include <inttypes.h>
#include <QByteArray>
#include <QString>
#include <QVector>
#include <QMap>
#include "stmabstract.h"

class QStLink;
class stm100 : public stmAbstract
{
public:
    stm100(QStLink & par, const pages_t & Pages);

protected:
    void ErasePageSetup(int PageNumber);


private:
};

#endif // STM100_H
