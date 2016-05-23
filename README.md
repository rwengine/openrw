# OpenRW

OpenRW is an open source re-implementation of Rockstar Games' Grand Theft Auto III,
a classic 3D action game first published in 2001.

* IRC: #openrw @ chat.freenode.net

Check out the [FAQ](https://github.com/rwengine/openrw/wiki/FAQ) and the rest of the [wiki](https://github.com/rwengine/openrw/wiki). Report any bugs by creating an [issue](https://github.com/rwengine/openrw/issues).

OpenRW requires a legitimate copy of the original PC game data in order to run.
Without this data it will not be possible to run OpenRW.

## Building

Dependencies:

* Bullet
* GLM (0.9.7+)
* SFML (2.0+)
* libmad
* Boost Test

On Ubuntu these can be installed with

`sudo apt-get install libbullet-dev libsfml-dev libmad0-dev libglm-dev`

Mac users should install Xcode and those packages from Homebrew:

brew install boost bullet glm mad sfml

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

### rwgame

This is the game binary. Before running the game a config file is needed, by default
the game will look for ``~/.config/OpenRW/openrw.ini``, which should look like:

```
[game]
path=/opt/games/Grand Theft Auto 3/        ; Game data path

[input]
invert_y=0                                 ; Invert camera Y
```
Eventually the game will write this for you, but currently it must be done by
hand.

* Options:
    * -w **n**, -h **n** sets initial window size
    * --newgame starts a new game automatically
* Controls:
    * W/A/S/D: Move
    * Shift: Sprint
    * Space: Jump
    * F: Enter/Exit Vehicle
    * Mouse Wheel: Cycle weapons
    * Mouse 1: Fire Weapon

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
