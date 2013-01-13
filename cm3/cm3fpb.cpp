#include "cm3fpb.h"
#include "armConstants.h"

cm3FPB::cm3FPB(QArm3 & father):
    par(father)
{
   CodeComparatorCount_();
   LiteralComparatorCount_();

   for (int i = 0 ; i < breaks.count(); i++)
   {
       breaks[i].active = false;
       breaks[i].address = 0;
       breaks[i].address_trunced = 0;
       breaks[i].REG_COMP = REGS.FP_COMP0 + (i * 4);
   }
}

void cm3FPB::CodeComparatorCount_()
{
    uint32_t temp = par.ReadMemoryWord(REGS.FP_CTRL);

    temp = (temp >> 4 & 0xF);

    par.Properties.CodeComparators = temp;
    breaks.resize(temp);
}

void cm3FPB::LiteralComparatorCount_()
{
    uint32_t temp = par.ReadMemoryWord(REGS.FP_CTRL);

    temp = (temp >> 8) & 0xF;

    par.Properties.LitComparators = temp;
    lit_count = temp;
}

void cm3FPB::EnableFPB()
{
    par.WriteRamByte(REGS.FP_CTRL, 3);
}

bool cm3FPB::LoadBreakPoint(uint32_t address)
{
    if (!IsFreeBreakpoint())
        return false;

    int b = GetFreeBreakpoint();

    breaks[b].active = true;
    breaks[b].address = address;

#if 0
    QByteArray tx,rx;
    tx.append(0xb);
    tx.append(b);
    par.FillArrayEndian32(tx,address);
    tx.append(3);

    par.CommandDebug(tx,rx,2);
    uint32_t test = par.ReadMemoryWord(breaks[b].REG_COMP);
#else
    uint32_t temp = breaks[b].address  << 2; //set address
    temp |= 1;  //enable
    temp |= 0b01 << 30;    //enable breakpoint
    par.WriteRamWord(breaks[b].REG_COMP,temp);
#endif
    asm("nop");
    return true;
}

int cm3FPB::GetFreeBreakpoint() const
{
    for (int i = 0 ; i < breaks.count(); i++)
    {
        if (breaks[i].active == false)
            return i;
    }

    return -1;
}

bool cm3FPB::IsFreeBreakpoint() const
{
    if (GetFreeBreakpoint() == -1)
        return false;
    else
        return true;
}
