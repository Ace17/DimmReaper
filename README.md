DimmReaper
----------

A tiny multithreaded user-mode memory tester for Windows and GNU/Linux.

By Sebastien Alaiwan.

This is based on the test patterns of Charles Cazabon and Simon Kirby,
see tests.cpp.

This is a 50kb standalone executable file.

License
=======

Licensed under the terms of the GNU General Public License version 2 (only).
See the file COPYING for details.

Usage
=====

```
$ dimmreaper.exe <N> [BS]
```

Where N is the memory block count, and BS is the block size, in megabytes.
BS is set to 1Gb by default.

The program will spawn N threads, each one testing its own memory block.
It will run until a memory error is detected.
Thus if your RAM is flawless, it will run forever.

Depending on the configuration of your OS,
you might need to run this program as root/admin,
otherwise, the program might not be able to lock the requested memory.

Beware to leave some room for the operating system.
You might crash it if you allocate too much!

Examples:

If your computer has 16Gb physical RAM, let's allocate 14Gb :

```
$ dimmreaper.exe 14
```

Another example with 16 blocks of 256Mb each:

```
$ dimmreaper.exe 16 256
```


Build
=====

To do a native build:

```
$ make
```

To do a cross build,
just override the CXX environment variable:

Example (from GNU/Linux to Windows):

```
$ CXX=x86_64-w64-mingw32-g++ make
```
