#ifndef STM100_H
#define STM100_H

#include <inttypes.h>
#include <QByteArray>
#include <QString>
#include <QVector>
#include <QMap>

#define KEY1    0x45670123
#define KEY2    0xCDEF89AB

class cm3FPB;
class QStLink;
class stm100
{
public:
    typedef QVector<uint32_t> pages_t;
    stm100(QStLink & par, const pages_t & Pages);

    virtual void EraseMass() throw (QString);
    virtual void EraseRange(uint32_t start, uint32_t stop, bool verify = false) throw (QString);
    virtual void WriteFlash(uint32_t start , const QByteArray & data) throw (QString);

    typedef struct
    {
        QString Reg;
        uint32_t val;
    } reg_t;
    typedef QVector<reg_t> regs_t;
    const regs_t& ReadAllRegisters(void);
    virtual void ReadAllRegisters(uint32_t * rawData);
protected:
    QStLink & par;
    const pages_t pages;
    bool locked;
    bool busy;
    QByteArray loader;
    regs_t registers;

    virtual void FlashUnlock();
    virtual void FlashLock();
    virtual bool IsBusy();
    virtual bool IsLocked() ;
    bool VerifyErased(int PageNum = -1);
    virtual void ErasePage(int pageNumber) throw (QString);

private:
    const uint32_t Size;
    const uint32_t Count;

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
