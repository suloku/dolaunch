BrosexecConf: Configurable SD dol launcher 0.1 by suloku  '15
*************************************************************

This is a dol loader that uses a configuration file to assign each GameCube button to a dol file. Autobooting a dol file after a set amount of time is also possible.

Launched dol files can have command line arguments trough the use of .cli files (read its section in this readme file).

There are two version available: the text version and the back version:
	- Text version: takes 13 blocks when converted to gci, its purpose is to be used with the Home Bros. exploit and take less memory card space.
	- Back version: takes 21 blocks when converted to gci. This file uses libPNGU by frontier to display a background image, opening customization possibilities since text displayed by brosexecconf can be disabled in the configuration file, thus it can be used to show a splash screen before autobooting, or using the background image as graphical menu to show which buttons are associated to each dol file.

My personal recommendation for the HomeBros exploit to keep memory card space at minimum is to use the simple Brosexec, that takes only 11 blocks, which will launch autoexec.dol from the SDGecko. BrosexecConf should be placed as autoexec.dol in the sdcard. The boot sequence would be: Smash Bros --> brosexec --> brosexec conf --> the desired dol.
Seems redundant, since booting is quick no significant amount of time is lost between smash bros and autoexec.dol launched by brosexec.

Uses libfat 10.0.14 and thus supports FAT32 and SDHC.
This program uses the dol loading code and cli file parsing from swiss by emu_kidid: https://github.com/emukidid/swiss-gc


Configuration:
-------------

sdgecko:/autoboot/autoconf.txt: configuration file, up to 12 buttons can be configured for dol booting, each with a title to be shown in the program. If there's no title, the path to the dol will be shown.

Timer value: seconds until program boots default dol. If there is no default dol specified in the config file, sdgecko:/autoexec.dol will be launched. Can be disabled with value -1.

Image path:


Some considerations:
--------------------

	- If a button with no assigned dol is pressed, default dol will be launched instead, if there isn't one, autoexec.dol will be boot.

	- If a button is configured, but the dol file isn't in the sdcard, default dol will be launched instead, if there isn't one, autoexec.dol will be boot in its place.


Command Line arguments:
----------------------

To pass command line arguments to launched dol, create a plain text file and rename it to the same filename as the dol to boot, with .cli extension.
For example:
	gbi.dol
	gbi.cli

The contents of the file are simple: each line is an argument to be passed to the launched dol. For example, we want to launch GB Interface with on screen display of and video mode to be pal 60, the cli file contents would be:

-noosd
-format=pal60

_____________________
suloku 15'