# OpenRW

OpenRW is an open source re-implementation of Rockstar Games' Grand Theft Auto III,
a classic 3D action game first published in 2001.

OpenRW requires a legitimate copy of the original PC game data in order to run.
Without this data it will not be possible to run openrw.

## Building

Dependencies:

* Bullet
* GLM (0.9.5+)
* SFML (2.0+)
* libmad
* Boost Test

Global Options:

* BUILD_TESTS — Build the test suite
* BUILD_VIEWER - Build the Qt GUI for viewing data
* BUILD_SCRIPT_TOOL - Build the script dissassembler

### Recomended build

```
$ mkdir build
$ cd build
$ cmake ../ -DCMAKE_BUILD_TYPE=Release
```

## Running

Once compiled, set the environment variable `OPENRW_GAME_PATH` to the directory containing "gta3.exe" and run the rwgame executable.

### rwgame

This is the game binary

* Options:
    * env: OPENRW\_GAME\_PATH, must be set to the folder containing "gta3.exe"
    * -w **n**, -h **n** sets initial window size
    * --newgame starts a new game automatically

### rwviewer

This is a Qt tool for opening the game data. It currently supports  looking at
objects and their models, and a primitive world viewer. It needs more work to
be useful for looking inside archives and viewing textures.

## Documentation

Run Doxygen on the included Doxyfile to generate documentation.

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

### Third Party Licenses

MAD is licensed under the GNU General Public License

* http://www.underbit.com/products/mad/

SFML is licensed under the zlib license

* http://www.sfml-dev.org/license.php

Bullet Physics is licensed under the zlib license

* http://bulletphysics.org/mediawiki-1.5.8/index.php/LICENSE
