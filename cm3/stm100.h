#ifndef STM100_H
#define STM100_H

#include <inttypes.h>
#include <QByteArray>
#include <QString>
#include <QVector>
#include <QMap>

class QStLink;
class stm100
{
public:
    typedef QVector<uint32_t> pages_t;
    stm100(QStLink & par, const pages_t & Pages);

    void EraseMass() throw (QString);
    void EraseRange(uint32_t start, uint32_t stop, bool verify = false) throw (QString);
    virtual void WriteFlash(uint32_t start , const QByteArray & data) throw (QString);

protected:
    QStLink & par;
    const pages_t pages;
    bool locked;
    bool busy;
    QByteArray loader;

    void FlashUnlock() throw (QString);
    void FlashLock();
    bool IsBusy();
    bool IsLocked() ;

    void ErasePage(int pageNumber) throw (QString);
    virtual void ErasePageSetup(int PageNumber);

    int GetPage(uint32_t addr);
    uint32_t GetBaseAddr(int page);
    bool VerifyErased(int PageNum = -1);

    typedef struct
    {
        uint32_t BUSY;
    } flash_sr_bit_t;

    typedef struct
    {
        uint32_t MASS_ERASE;
        uint32_t PAGE_ERASE;
        uint32_t LOCK;
        uint32_t START;
        uint32_t PROG;
    } flash_cr_bit_t;

    typedef struct
    {
        volatile uint32_t * SR;
        volatile uint32_t * CR;
        volatile uint32_t * KEYR;
        flash_sr_bit_t SR_BITS;
        flash_cr_bit_t CR_BITS;
        uint32_t KEY1;
        uint32_t KEY2;
    } flash_t;

    flash_t FLASH_CONST;
private:
};

#endif // STM100_H
