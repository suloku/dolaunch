rm boot.dol
rm boot.gci
make clean
make
dolxz.exe brosexec.dol boot.dol -cube
dol2gci.exe boot.dol boot.gci