# OpenRW

This is an attempt at re-implementing the GTA III game binary, adding support for more platforms
and input methods, while fixing some issues that plauge the original release.

**REQURIES A COPY OF GTA III PC**.
Without an original copy of the game, it will not be possible to run openrw.

## Building

Dependencies:

* Bullet
* GLM (0.9.5+)
* SFML (2.0+)
* Boost Test

Options:

* BUILD_TESTS — Builds the test suite
* BUILD_TOOLS — Builds the rwviewer application
* BUILD\_OLD\_TOOLS – Builds old, unmaintained tools (datadump & analyzer)

### Recomended build

```
$ mkdir build
$ cd build
$ cmake ../
```

## Running

Once compiled, set `OPENRW_GAME_PATH` to the directory containing "gta3.exe" and run rwgame.

### rwgame

* Options:
    * env: OPENRW\_GAME\_PATH, must be set to the folder containing "gta3.exe"
    * -w **n**, -h **n** sets initial window size

### rwviewer

Intended to be a tool for viewing the various game data types, such as objects and models. Currently awaiting refactoring.

## Documentation

Run Doxygen on the included Doxyfile to generate documentation.

## Things to do

* Finish rwng reorganisation
	* Split basic functionality from rwengine into core library.
	* Clean up more of GameObject's member variables.
	* Implement script debugging.
	* Fix rwviewer for new paradigm.
* Fix water rendering artefacts.

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
