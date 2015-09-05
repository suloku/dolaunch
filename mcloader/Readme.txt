Brosexec 0.2 by suloku '15
**************************

This is a simple dol that will mount a memory card and scan for dol files created with dol2gci, it will list them and allow to boot them.

It's main purpose is to be used with the Home Bros. Exploit, as a way to boot multiple homebrew without owning an SDGecko adapter.

Currently the needed space is 9 blocks on the memory card.


Installing dol files to the memory card:
----------------------------------------

To install dol files to the memory card, convert them with dol2gci, then install them to the memory card with your preferred method. You can use GCMM for that: http://wiibrew.org/wiki/GCMM

note: the name of the file on the memory card will be that of the dol file BEFORE converting it with dol2gci. After the conversion is done, renaming the gci file will have no impact on the memory card filename.

Command line parameters
-----------------------
To pass command line parameters to the launched dol file, follow the following steps:
1.- Use dol2gci and convert yourfile.dol to yourfile.gci
2.- Create a blank text file named yourfile.cli (same filename as the dol file)
3.- With a text editor, edit the cli and add the parameters, one paramenter per newline
4.- Embed yourfile.cli to yourfile.gci using cliembed.exe

How does this work: dol2gci needs to pad the dol files when converting to gci, since even a 1 byte file will occupy 8912 bytes (1 block) in the memory card. dol2gci pads this additional space with 0xCD. cliembed searches for this padding area in the cli file, then adds the cli file with some magic strings to the very end of this padding.
McLoader just looks for this magic strings to read the cli file from there.

What if the dol file occupies all the blocks with no padding? This would be a very very rare situation, but if it happens cliembed will ask you if you want to create a padded gci file. If you select yes, an aditional block of padded data will be added to the gci file and the gci header will be updated, then the new padded file will be saved as yourfile-fat.gci. After that, use yourfile-fat.gci to embed the cli file to it. The downside is obvious: the file will require an aditional block in the memory card.


Changelog:
----------
[+] 0.2:
	- Added command line argument support by loading a embeded cli file inside the gci file created with dol2gci. Use attached cliembed.exe file for this. 

----------------------

- Check the Home Bros. exploit here: http://www.gc-forever.com/forums/viewtopic.php?f=38&t=3023

- Check swiss here: https://github.com/emukidid/swiss-gc

_____________________
suloku 15'