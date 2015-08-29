rm *.o
rm brosexec.s
bin2s ../boot.dol > brosexec.s
powerpc-eabi-gcc -O2 -c startup.s
powerpc-eabi-gcc -O2 -c brosexec.s
powerpc-eabi-gcc -O2 -c main.c
powerpc-eabi-ld -o sdloader.elf startup.o main.o brosexec.o --section-start .text=0x81700000
rm *.o
powerpc-eabi-objcopy -O binary sdloader.elf SDLOADER.BIN
rm *.elf
rm brosexec.s