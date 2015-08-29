Configurable sd dol launcher 0.1 by suloku  '15
***********************************************

This file takes 13 blocks in the memory card.

sdgecko:/autoboot/autoconf.txt: configuration file, up to 12 buttons can be configured for dol booting.

Timer value: seconds until program boots default dol. If there is no default dol specified in the config file, sdgecko:/autoexec.dol will be launched.

If a button with no assigned dol is pressed, default dol will be launched instead, if there isn't one, autoexec.dol will be boot.

If a button is configured, but the dol file isn't in the sdcard, default dol will be launched instead, if there isn't one, autoexec.dol will be boot.



NOTE: dol loading code thanks to FIX94's source code from gbiargs.

_____________________
suloku 15'