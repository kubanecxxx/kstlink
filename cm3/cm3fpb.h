#ifndef CM3FPB_H
#define CM3FPB_H

#include <QVector>
#include <inttypes.h>

class QArm3;
class cm3FPB
{
public:
    cm3FPB(QArm3 & father);

    void EnableFPB();
    int CodeComparatorCount() {return breaks.count();}
    int LiteralComparatorCount() {return lit_count;}


    bool IsFreeBreakpoint() const;

protected:
    bool LoadBreakPoint (uint32_t address);
    typedef struct
    {
        bool active;
        uint32_t address;
        uint32_t address_trunced;
        uint32_t REG_COMP;
    } break_t;


private:
    QArm3 & par;

    void CodeComparatorCount_();
    void LiteralComparatorCount_();
    int GetFreeBreakpoint() const;

    int lit_count;



    QVector<break_t> breaks;
};

#endif // CM3FPB_H
