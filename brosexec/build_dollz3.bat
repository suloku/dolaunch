rm boot.dol
rm boot.gci
make clean
make
dollz3.exe brosexec.dol boot.dol
dol2gci.exe boot.dol boot.gci