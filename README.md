# H2Codez
A mod for the H2EK that aims to restore use of existing broken or otherwise disabled functionality,
fix bugs/crashes, and add new features or re-add removed ones.

[![status](https://ci.appveyor.com/api/projects/status/github/Project-Cartographer/H2Codez?branch=H2OS_EK&svg=true)](https://ci.appveyor.com/project/num0005/h2codez-0w3n8)

## Changelog ##
* Restore tool commands used by bungie to distribute lightmapping over multiple machines.
* Real-time tag editing.
* Increased FPS in Sapien (hardware TCL/vertex processing).
* Unit playtest mode. Spawns a player letting you test how something would work with one/seeing the map from an in-game point of view. (no movement).
* Some H2tool commands restored. (model-collision, model-physics).
* Basic render model import method (BSP conversion).
* Shared tag removal made optional.
* Restored JMS import function. Can import both CE JMS and JMSv2.
* Tag restrictions removed in Guerilla.
* Some hardcoded limits have been increased/removed. (BSP 2D checks, BSP 3D checks, max 3D BSP depth)
* Open as text has been restored and will open a temp txt file with the source text inside. Buttons labeled as "open as text" have been relabeled as "export as text".
* Baggage.txt command now usable and no longer causes a crash.
* Hs_doc added to Sapien and Guerilla drop down menu. Modified to include script globals.
* New input box for commands in Sapien to replace console. Allows copy and paste.
* New command that allows use of lost misc commands. See extra-commands in H2Codez manual.
* Rich Presence has been added to the toolset. (Discord integration)
* Allows compiling of scenario types other than multiplayer.
* Support editing larger scripts.
* Add "New Instance" menu item to Guerilla and Sapien.
* Removed limitation on multiple Sapien instances.
* Added copy (ctrl + c), paste (ctrl + v) and clear (delete) support to Sapien console, paste replaces the whole line.
* Add configurable console history size.
* Add back support for the following HS types
  * AI behaviour
  * Conversation
  * AI orders
  * AI (starting locations referenced by block index)
  * Point reference
  * Style (internal ID only)
  * Navpoint (internal ID only)
  * AI command list (internal ID only)
  * HUD message (internal ID only)
* Use default OS style for file/folder selection dialog.
* In-game display settings option in Sapien.
* Custom director speed.
* Workaround for graphical artifacts on some Nvida cards.
* Some misc changes and fixes.

## Installation
* __Install Microsoft .net framework 4.__
* __Install the mapping toolkit from the DVD.__
* __[Download the launcher.](https://ci.appveyor.com/api/projects/num0005/h2-toolkit-launcher/artifacts/Launcher/bin/Release/H2CodezLauncher.exe
)__
* __Copy the launcher to the mapping toolkit install directory (optional).__
* __Run the launcher, elevate to admin privledges if prompted.__

## Contributing ##
Contributions are welcome, MSVC 2015 is currently used to build release DLLs, and is therefore the recommended IDE.
If you are not sure what you want to work on take a look at the [bug tracker](https://github.com/Project-Cartographer/H2Codez/issues).
### Compiling ###
* Clone and build the MSVC solution, if the toolkit is installed in an unusual location you might need to change the output directory.
* Follow the steps listed under [installation](#installation) to get patched exes or manually inject H2Codez.dll into the process at startup.
### Reporting Issues ###
* Report issues on the [bug tracker](https://github.com/Project-Cartographer/H2Codez/issues/new), please include an explanation of how you caused the issue.
* If the issue in question causes a crash, a minidump should be generated, hang on to it as it might be needed to fix rare bugs.
* If in doubt about including a detail, include it as more details about a bug make it easier to understand and fix.

## H2PC Project Cartographer Team ##
Visit: www.halo2.online 

Project Cartographer Repo: https://github.com/PermaNulled/cartographer

Our Discord: https://www.discord.gg/fqgj44m

Hope to see you in-game :)

## Credits ##
* [Kornman00](https://github.com/KornnerStudios) for his research in this field years ago as part of [OpenSauce](https://bitbucket.org/KornnerStudios/opensauce-release/wiki/Home), which has helped a lot with this project.
* [General-101](https://github.com/General-101), Ling Ling, Twinreaper and others, for testing the patches and suggesting new ones.
* Authors of all thrid party libraries used. 

## Acknowledgements ##
* Cyboryxmen for his modified JMS export script known as Waltzstreet 
* TheGhost/CtrlAltDestroy for the modified JMS exporter known as JMS_Exporter_v1-0-3
* [Grimdoomer](https://github.com/grimdoomer), for writing the guerilla layout viewer which was a huge help in understanding the tag defintion format.

## License ##
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/.
