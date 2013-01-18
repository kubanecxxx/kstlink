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

    typedef struct
    {
        QString Reg;
        uint32_t val;
    } reg_t;
    typedef QVector<reg_t> regs_t;
    const regs_t& ReadAllRegisters(void);

    void ReadAllRegisters(uint32_t * rawData);
    void EraseMass() throw (QString);
    void EraseRange(uint32_t start, uint32_t stop, bool verify = false) throw (QString);
    virtual void WriteFlash(uint32_t start , const QByteArray & data) throw (QString);

protected:
    QStLink & par;
    const pages_t pages;
    bool locked;
    bool busy;
    QByteArray loader;
    regs_t registers;

    void FlashUnlock();
    void FlashLock();
    bool IsBusy();
    bool IsLocked() ;
    void ErasePage(int pageNumber) throw (QString);

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

    virtual void ErasePageSetup(int PageNumber);

private:
    //arm stm32 register set
    typedef struct
    {
        uint32_t r[15];
        uint32_t pc;
        //r15 - program counter
        //uint32_t s[32];
        uint32_t xpsr;
        uint32_t main_sp;
        uint32_t process_sp;
        uint8_t primask;
        uint8_t control;
        uint8_t basepri;
        uint8_t faultmask;
        uint32_t fpscr;
        uint32_t vata[32];
    } cm3_regs_t;

    cm3_regs_t regs_human;
};

#endif // STM100_H
