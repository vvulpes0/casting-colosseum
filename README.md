# Casting Colosseum

## Description

This is a game made for the Pass The Jam game jam
using SDL2, pocketmod, and the stb image library.
For a while I've been wanting to demonstrate the
usefulness of MOD files for games, especially for
syncing actions to the beat of the music.  This is
essentially a demo of that technique.

I might revisit this at some point and clean up the
code, now that the timed portion is out of the way,
but for now its purpose has been served!

## Dependencies

Pocketmod and stb are included as git submodules;
use `git submodule update --init --recursive` to
check out these (single-header) libraries.  See
the respective submodules for their licenses.
(Both are available under the MIT license.)

SDL2 is also required, and can be obtained through
https://www.libsdl.org or, in many cases, your system's
package manager.  It is available under the zlib license.

The code is written in a C99-compliant fashion, and
while I'm likely to modify it for C89-compliance at
some point, that point is not today.  What this means
is that it has not been tested with nor should it be
assumed to work under a C++ compiler.
