NECO=stm100
all:
	arm-none-eabi-as $(NECO).S -mcpu=cortex-m3 -mthumb -gstabs -o $(NECO).o
	arm-none-eabi-ld -v -T ld.ld -nostartfiles -o $(NECO).elf $(NECO).o
	arm-none-eabi-objcopy -O binary $(NECO).elf $(NECO).bin

