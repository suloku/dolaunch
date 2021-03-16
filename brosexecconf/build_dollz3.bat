rm *.elf *.dol *.gci
cp Makefile.text Makefile
make clean
make
dollz3.exe brosexecconf-text.dol boot.dol
dol2gci.exe boot.dol boot.gci
mv boot.dol boot-text.dol
mv boot.gci boot-text.gci
cp Makefile.back Makefile
make clean
make
dollz3.exe brosexecconf-back.dol boot.dol
dol2gci.exe boot.dol boot.gci
mv boot.dol boot-back.dol
mv boot.gci boot-back.gci
rm Makefile