Only static library is built. I assume that dependencies are also static libraries.
Paths for includes and necessary macros definitions are in Macros.props

Dependencies:
=============
curl, libiconv, libxml2

Patches for dependencies:
=========================
curl: There was an issue with it using SSL feature based on SDK version
instead of checking availability at runtime or depending on host system.

libiconv: win32 is not supported, included patch is based on
https://sites.google.com/site/kontr0kontradiktion/software/patches

libxml2: It requires a few defines to compile with VC14.

Patched projects for libxml2 and libiconv are still in VC10 directories,
which might be technically incorrect. I only tested them with VS2015.


How to build dependencies:
==========================
curl build is hassle-free on windows.
Use nmake from winbuild directory.
Like this: 'nmake /f Makefile.vc mode=static DEBUG=yes VC=14'.

libiconv: Patch; don't do anything else.

libxml2: Visual Studio solution is part of the package. It also references libiconv project patched in
in the step above. Patch, build solution.

