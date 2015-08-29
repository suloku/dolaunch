# Dolaunch
Set of dol launching tools for GameCube

This tools are based on the dol loading code used in swiss by emu_kidid: https://github.com/emukidid/swiss-gc

· Brosexec: This is a simple dol that will load autoexec.dol from the root of a SD card in a SDGecko adapter. It's main purpose is to be used with the Home Bros. Exploit, as a wai to boot homebrew on unmodifyied consoles. One of its golas is to keep used space in the memory card as low as possible.
	- Check the Home Bros. exploit here: http://www.gc-forever.com/forums/viewtopic.php?f=38&t=3023

· Brosexecconf: this is a simple configurable dol launcher, the configuration file can assing a dol to each button. Supports a timer to autoboot a default dol after a configurable amount of time passes. Also supports passing command line arguments to loaded dol files in the form of .cli files.
	- A text only version and a version supporting customizable png background are available (on screen text can be disabled so the background can be used as splash screen for autoboot for example).
	- Uses lib PNGU by frontier: http://wiibrew.org/wiki/PNGU

· McLoader: this simple dol can launch any dol present in a memory card created with dol2gci. It¡s main use would be for those not having an SDGecko adapter.