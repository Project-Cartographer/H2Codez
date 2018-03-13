Hello friends! I am General_101 of the PCMT group. In this writeup I'll go through the new features added by H2Codez.  I'll also be giving you some tips and tricks that I found useful for myself when working with these things.
Some say it's dangerous to go alone so take this and let's begin.

---Index---

*** Instructions ***

1. Instructions

*** Tool *** 

2.  Tool - Links
3.  Collision Tags
4.  Physics_Model Tags
5.  Render Import(BSP Conversion)
6.  Model Tags
7.  Compiling The Above
8.  Extra Commands
9.  Unlocked Scenario Compiling
10. Shared Removal

*** Guerilla ***

11. Guerilla - Links
12. Advanced Shader View
13. New Instance
14. Unlocked Tags
15. Xbox Audio Codec
16. Baggage.txt
17. HS_doc

*** Sapien ***

18. Sapien - New Instance
19. Window
20. Scenario Type Strings
21. Run Commands
22. Script Execution

*** Credit ***
23. Credit

\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

1. --- Instructions ---
Firstly you will want to install the game itself and the dedicated server files if you wish and update to the latest version of Halo 2. 
After you have updated your game you will want to install the map editor and NOT update the map editor. 
H2Codez requires that you have the unpatched version of the tools so that it may patch the exes to use H2Codez. You can use the launcher to do this easily and quickly. 
Check the links below for the H2Codez launcher download

2. --- Tool - Links ---
https://mega.nz/#!448zXJRS!SmXYx1GNFhRMZJauOUh05RqZVPBDIHtPc32tvrb7q9A
A link to a pack of exporters you can use for your custom model goals. Halo 2 by default uses JMS files for importing all of our models. 
While JMS(8200/HaloCE) differs from JMSv2(8205-8210/Halo2[2004]) our tool still has support for the older format. 
I won't say it's perfect and that making an exporter that exports in the file format of JMSv2 isn't needed but it isn't imperative for now. 
Let me break down each file, MS is the file type for Max script that you would use for 3DS Max. These files should work from 3DS Max 5 to 3DS Max 2017. 
The PY are Python script files that you can use to export your models with Blender. Works with 2.79 and below at time of writing. You will notice that there are two files for each type with one abbreviated with "shitty". 
These files are modified exporters for the purpose of only exporting vertice count and vertice location which will be useful in the future for physics compiling.

https://ci.appveyor.com/api/projects/num0005/h2-toolkit-launcher/artifacts/Launcher/bin/Release/H2CodezLauncher.exe
https://github.com/num0005/H2-Toolkit-Launcher
A very useful tool that replaces the default map editor launcher that comes with the map editor. It has several new features not available in the default launcher. 
This launcher for starters will auto download the latest version of H2Codez whenever there is a commit added to the repo. The level compiling option can take both JMS/JMSv2 and ASS file types for level compilation. 
Model compile is one of the greatest features of H2Codez we will go into deeper detail later in this manual. 
Packaging has a new option below copying to the maps folder. This option should be obvious but the name is "Remove Shared Tags". 
When ticked this will scan the dependencies of the map you are packaging and take everything instead of using shared.map for it's assets. Do not use this unless you know what you are doing. 
Please report any issues you find on the Github.

https://github.com/Himanshu-01/H2Codez
An important tool that expands the capabilities of H2EK. Drop your stone tools and join the 21st century with this amazing DLL. 
This is the DLL that is injected into your toolset that allows us to open previously locked tags and entirely new features in the H2V toolset.

https://github.com/Himanshu-01/H2PC_TagExtraction
A very useful tool for extracting tags for study. It has some nifty features like "Extract Import Info". This command lets you extract the file used to compile the original tag for uncompiled tags. 
You can use this to view the original JMSv2 files for the models Bungie imported into the game for your own learning. It also has the ability to dump the taglist of a tag block to a txt file with datum index. 
This can be used in a txt file called commands.txt placed in your Halo 2 root directory for use with the spawn command. Make sure that the object you want to spawn is in a resource map or map the game has loaded. 
As of time of writing the tag extractor has several issues with extracted tags. Most things are only for study and will cause crashes ingame or in the editor. 
Shaders for example are completely broken and must be replaced to avoid the compiled shader data within. Replace tags with clean ones modeled after the extracted one to avoid issues. Recompile things like bitmaps when you can. 
Collisions and physics will be only viewable in Guerilla and will causes crashes in Sapien or ingame when the object is loaded.

https://mega.nz/#!BpFzHaQQ!zj1lH0FhUk83_6OarKwuTy5ueijLzD2mtFFM3AnOGAY
Example files for the render model importing. See #5

https://notepad-plus-plus.org/
A very useful to to edit txt files. Please take the time to download and install.

https://pastebin.com/NmZEekp0
A JMSv2 template for physics files. Use this to place in your models.

3. --- Collision Tags --- 
An important tag for your model. This is what you will need if you want to have something you can shoot at or use to block things. 
In order to import our custom collision model you will want to setup something using the Halo CE standard JMS exporter for either 3DS Max/Blender. 
Once you have a JMS file exported you will now need to do some crucial things in order to ensure it will be accepted by tool. 
You will have a file with a layout like this.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
8200
1
1
frame
-1
-1
0.000000	0.000000	0.000000	1.000000
0.000000	0.000000	0.000000
1
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Add a piece to your node like so.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
8200
1
1
b_frame
-1
-1
0.000000	0.000000	0.000000	1.000000
0.000000	0.000000	0.000000
1
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Next we will solve an error related to the entire model being imported as one region. You can fix this by adding a permutation and region under your material like so....
^^^^^^^^^^
1
metal
<none>
0
1
unnamed
^^^^^^^^^^
To this
^^^^^^^^^^
1
metal
base metal
0
2
base
metal
^^^^^^^^^^
base metal = (permutation) (region)
The final step is to change the encoding. Open up the text file in Notepad++ and change the encoding from whatever it currently is to UCS-2 LE BOM. Tool should now accept your JMS files without issues assuming they are valid.

4. --- Physics_Model Tags ---
Physics model files are a bit more tricky. You will need to do a couple of things to set these up properly so begin by exporting two files. 
Our regular JMS file using the regular exporter and a second JMS file using the "shitty" exporter. Take your template and get ready to fill it in for your physics model. 
You will want to first fill out the node section. Go to your filled out JMS file and take the node from there. Should look something like this.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
1
frame
-1
-1
0.000000	0.000000	0.000000	1.000000
0.000000	0.000000	0.000000
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Convert it to this and place it in your node list
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
;### NODES ###
1
;   <name>
;   <parent node index>
;   <default rotation <i,j,k,w>>
;   <default translation <x,y,z>>
 
;NODE 0
b_frame
-1
0.0000000000	0.0000000000	0.0000000000	1.0000000000
0.0000000000	0.0000000000	0.0000000000
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Now you want to assign a material with a region and permutation. Begin by adding this to your template.
^^^^^^^^^^^^^^^^^^^^^^
;### MATERIALS ###
1
;   <name>
;   <material name>

;MATERIAL 0
(global material name)
(permutation) (region)
^^^^^^^^^^^^^^^^^^^^^^^
Physics don't use shaders for their material names. They instead use material effects. Pick something you like.
^^^^^^^^^^^^^^^^^^^^^^
;### MATERIALS ###
1
;   <name>
;   <material name>

;MATERIAL 0
hard_metal_thick_hum
base default
^^^^^^^^^^^^^^^^^^^^^^^
Now that the material is setup lets get the actual geo for your physics model in. Grab that JMS that you exported with the shitty exporter. You should see a vertice count along with a ton of vertice locations.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
20298
-25.17850097136	-10.93558594447	1.883197579523
-27.11733181272	-10.73211953932	2.356396971319
-24.86580137328	-8.290422677639	1.291731673068
11.59661842818	-23.82216938133	105.8638305997
14.97394742063	-22.87263726843	104.0894662136
11.97188461252	-24.29693543778	105.9821971142
-22.34400461455	-8.462422456568	104.3313659027
-21.94563845990	-9.118888279478	106.3428633173
-21.89797185450	-8.365689247566	117.0702828627
-26.99223197351	2.764929779565	11.34675208267
-23.67920289841	3.494695508262	6.549091582442
-23.61610297952	4.087728079370	6.548391583341
-13.79571560168	10.63251966734	40.68371437582
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Should look like this. In order to use this data in our physics model we need to create a convex collision. Fill out this part of the template.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
;### CONVEX SHAPES ###
1
;	<name>
;	<parent>
;	<material>
;	<rotation <i,j,k,w>>
;	<translation <x,y,z>>
;	<vertex count>
;	<...vertices>

;CONVEX SHAPE 0
(object name)
0
0
0.0000000000	0.0000000000	0.0000000000	1.0000000000
0.0000000000	0.0000000000	0.0000000000
(vertice count)
(vertice list)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Create a convex shape entry in the list as ;CONVEX SHAPE # and give it a name. Node index starts at -1 so for example frame = -1. 
When we want to reference this index for our physics objects in the list we want to start at 0 however. So if frame was -1 and frame_b was 0 and we wanted to reference frame_b then physics object A would reference 1 like so
^^^^^^^^^^^^^
(object name)
1
0
^^^^^^^^^^^^^
The material index will depend on what effect you want for that object. The next part is a bit annoying. Basically what this part is is the centroid of the object that you are exporting. 
So while frame is at 0 0 0 our physics object may bee 3 units above frame so you would type 0 0 3 for the physics object for it's placement in relation to frame. 
I personally change the location of frame to reflect the center of the physics object I want to export and then copy the location of frame over to the physics object. You are pretty much done after this. 
You can use the tag extractor with extract import info to look at some of the physics_model.JMS files to get a good idea of what a JMSv2 file would want.

5. --- Render Import(BSP Conversion) ---
Render models are the tags for the object itself. There are currently 2 methods for importing your own custom render models. There is the GBXmodel Upgrader. The purpose of this tool is to transfer data from a GBXmodel tag from CE
to a render_model tag from Halo 2. While this generally works and can import rigged models it has many issues related to proper node location and can result in broken UVs. The second method is the one included in H2Codez which
is a BSP conversion method. A model is first compiled as a BSP using ASS and then the data is transfered over to a render_model file. This should result in a model with intact UVs though it is in beta currently
and requires testing. Please download the example render model file from links as an example. In order to import a working render model one must first export a valid ASS file. 
You will want to assign your shaders for the model somewhere in the shader_collection so that they are assigned to the BSP during compile.
This will help you later. A good method to prevent out of BSP errors is to surround your model in a box and give it a unique shader with the @ flag at
the end of the shader name. This will expand the space for the model as collision only meaning no render geometry is added to your render model. 
Give the rest of the model the ! flag at the end of the material name so that it only generates render geometry and nothing else. This will help with errors related to collisions and such. 
This won't solve everything due to the methods but this will help with most of your issues. 
You are now ready to use the render command. Once it is compiled you will need to now fix the result as it is not ready to be used. All this method does is transfer geometry data over. The rest must be filled in by you.
We will go over how to get it usable in Halo 2 now. Please download the render example pack so that you may follow along as I explain. Let's first begin from the bottom of the render model file starting with the materials block. 
Compare this with the materials block in the original BSP file that should have been placed in the same folder.
![BSP vs render model](https://i.imgur.com/GxEFJPs.png)
You can look at the image above to see where we are. Left being the render model and right being the original BSP. You will want to transfer over the materials in the exact same order. Do this until you are done and we can move on
to the next part of the model.
![BSP vs render model](https://i.imgur.com/O4UPAo7.png)
Here you will setup your model origin. Left is what an freshly imported render model looks like and left is what we want it to look like to work. You will want to add a node and then give it a name. Fill in the values with the
ones that are shown in the image. Not doing this can result in the model not appearing ingame.
![BSP vs render model](https://i.imgur.com/KFOGREM.png)
Next you will want to fill in section groups and invalid section pair bits. For invalid section pair bits just hit the add button once and you are done. For section groups just click the add button once and check all the boxes.
![BSP vs render model](https://i.imgur.com/x6OjOU8.png)
Next you will want to add a node map. Keep in mind that each section will have its own node map. Just copy the node map you make to the other sections. Just click add a single time and it should be good. Add more node maps and 
increase the value by 1 for each node you have in the node list.
![BSP vs render model](https://i.imgur.com/MYJApgf.png)
Now for the part about.... parts. Left being a fresh copy, middle being a modified copy, and right being the original BSP. You will want to go into parts and check override triangle list. You will also see here that the first
material you added will be assigned to all the parts. You can compare this to your BSP file to see what part had what assigned. This should be the same across the files. Keep in mind that there could be multiple parts in a model
and each section has its own parts.
![BSP vs render model](https://i.imgur.com/OCLa6Os.png)
Next you will want to change the geometry classification in section to rigid. You will want to do this to all sections you may have.
![BSP vs render model](https://i.imgur.com/atTGmNw.png)
Now you will want to add a region and a permutation. If you want multiple permutations you can change the section index to the represent the permutation you want. The first section you see being index 0 and the second in the list
being index 1.
![Naming the render model](https://i.imgur.com/Cz1hX1R.png)
Now name your render model. You should be done after this. 

Now you may have read this and now be wondering how multiple sections for permutations work. It's pretty simple.
https://i.imgur.com/FFQIQVw.png
Simply place multiple ASS files in the same directory and they will be added as new sections. The order they are in alphabetically will determine the section order. From here simply reference the proper index in the permutation 
section.

6. --- Model Tags ---
This is honestly better left for you to make it manually. All this command does is scan your model folder for render, collision, and physics model tag and creates a .model tag with references to those tags.

7. --- Compiling The Above ---
Now that you have all of your JMS/ASS files ready you will want to setup your folder structure in the data folder of the map editor. The root folder can be anywhere but but sure that it follows the following folder structure. 
Words in quotations are the name of the folder.

ROOT FOLDER "INSERT MODEL NAME HERE"
	COLLISION FOLDER "collision"
		COLLISION JMS HERE
	PHYSICS FOLDER "physics"
		PHYSICS JMS HERE
	RENDER FOLDER "render"
		RENDER JMS/ASS HERE
You can now use the all option in the model compile tab. Just select the object type and browse and select the models root folder. It will then spit out the compiled tags as it was in data but in tags in terms of folder location. 
Keep in mind that the collision command is a bit strange. While all your other tags will be inside the directory where they were compiled the collision tag will be placed outside the root folder. 
Just move it back inside afterwards and it will be fine.

8. --- Extra Commands ---
By typing in "h2tool extra-commands" you will get the option to use some of the dev commands in tool. You can use "h2tool extra-commands-list" but here they will be listed and explained if possible.
  fix-weapons
	description: your momma
		usage: Unknown
  convert-accel-screens
	description: loads all unit graphs and convert the acceleration screen data
		usage: Unknown
  update-animations
	description: loads all animation graphs to do any post-process updating needed
		usage: Unknown
  clear-animation-production-flags
	description: loads all animation graphs and clears their post process flag
		usage: Unknown
  print-weapon-labels
	description: converts old string_id's in weapons to class and label names
		usage: Unknown
  rename-weapon-labels
	description: converts old string_id's in weapons to class and label names
		usage: Unknown
  find-all-objects-inheriting-markers
	description: searches through all tags (ugh!) for object tags inheriting parent markers
		usage: Unknown
  update-scenario-ai-anims
	description: updates all tag references to old animation tags
		usage: Unknown
  update-scenario-anims
	description: updates all tag references to old animation tags
		usage: Unknown
  update-widget-anims
	description: updates all tag references to old animation tags
		usage: Unknown
  update-weapon-anims
	description: updates all tag references to old animation tags
		usage: Unknown
  update-sky-anims
	description: updates all tag references to old animation tags
		usage: Unknown
  fix-skies
	description: ...
		usage: Unknown
  fix-planar-fog
	description: ...
		usage: Unknown
  update-model-anims
	description: updates all tag references to old animation tags
		usage: Unknown		
  blank-render-models
	description: replaces all render_model tags with blank ones
		usage: Unknown	
  build-models
	description: builds model tags from objects
		usage: Unknown		
  find-blank-models
	description: builds model tags from objects
		usage: Unknown
  fix-collision-node-bsps
	description: fixes old collision models with bsps in their nodes
		usage: Unknown
  rename-collision-models
	description: renames old *.model_collision_geometry to *.collision_model
		usage: Unknown		
  find-old-objects
	description: finds objects with old tag references
		usage: Unknown		
  create-model-materials
	description: creates materials for old models
		usage: Unknown
  convert-lights
	description: convert all old (pre-merge) lights into new lights
		usage: Unknown
  fix-effects
	description: remove all effect parts with null tag references
		usage: Unknown		
  fix-lights
	description: ooo oo hho haaaaa! !!
		usage: Unknown
  model-shaders
	description: fixes render_model materials so they reference new shaders
		usage: Unknown	
  structure-shaders
	description: fixes structure materials so they reference new shaders
		usage: Unknown	
  fix-structures-for-fog
	description: fixes structure bsp cluster->scenario_atmospheric_fog_palette_index
		usage: Unknown		
  check-vehicles
	description: reports human plane vehicles
		usage: Unknown	
  fix-damage
	description: converts damage_resistance to damage_info
		usage: Unknown	
  check-shader-passes
	description: iterate over shader passes and do something useful
		usage: Unknown	
  check-shader-templates
	description: iterate over shader templates and do something useful
		usage: Unknown
  check-light-responses
	description: iterate over light responses and do something useful
		usage: Unknown	
  check-lens-flares
	description: iterate over lens flares and do something useful
		usage: Unknown
  fix-shader-passes
	description: iterate over shader passes and do something useful
		usage: Unknown
  fix-shader-templates
	description: iterate over shader templates and do something useful
		usage: Unknown
  fix-light-responses
	description: iterate over light responses and do something useful
		usage: Unknown
  fix-decals
    description: iterate over decals and do something useful
		usage: Unknown
  fix-shaders
	description: iterate over shaders and do something useful
		usage: Unknown
  fix-bitmaps
	description: iterate over bitmaps and do something useful
		usage: Unknown
  fix-model-damage2
	description: inverts a damage threshold
		usage: Unknown
  fix-physics-models
	description: upgrading to havok2.2
		usage: Unknown
  fix-render-model-compound-nodes
	description: taco salad LIVES!!
		usage: Unknown
  fix-scenery-objects
	description:
		usage: Unknown
  fix-structure-new-visibility
	description:
		usage: Unknown
  check-structure-materials
	description:
		usage: Unknown
  check-unused-variants
	description: look for models with nonzero pad in variants
		usage: Unknown
  fix-time-effects
	description: perform time conversion on all effects
		usage: Unknown
  fix-time-globals
	description: perform time conversion on game globals
		usage: Unknown
  fix-time-units
	description: perform time conversion on all units
		usage: Unknown
  fix-time-bipeds
	description: perform time conversion on all bipeds
		usage: Unknown
  fix-time-vehicles
	description: perform time conversion on all vehicles
		usage: Unknown
  fix-time-creatures
	description: perform time conversion on all creatures
		usage: Unknown
  count-scripts
	description: count number of script threads required
		usage: h2tool extra-commands count-scripts (path to scenario file without file extension ex. scenarios\multi\arena\arena)
  remove-weapon-huds
	description: count number of script threads required
		usage: Unknown		
  remove-biped-huds
	description: count number of script threads required
		usage: Unknown
  remove-vehicle-huds
	description: count number of script threads required
		usage: Unknown
  fix-lightmaps
	description: delete all lightmap groups
		usage: Unknown
  new-renderer-structure
	description: fixes structure bsps with data for the new renderer
		usage: Unknown
  new-renderer-render-models
	description: fixes render models with data for the new renderer
		usage: Unknown
  fix-particles
	description: loads and saves all particle tags to get around versioning bug
		usage: Unknown
  reimport-structure-bsps
	description: reimports all checked out structure bsps
		usage: Unknown
  reimport-collision-models
	description: reimports all checked out collision models
		usage: Unknown
  copy-out-pixel-shaders
	description: copies out pixel shaders within shader passes to separate files
		usage: Unknown
  report-bitmap-formats
	description: reports the format information for all bitmap tags
		usage: Unknown
  make-bitmaps-pc-safe
	description: fix bitmap tags to not use Xbox specific formats
		usage: Unknown
  unpalettize-lightmaps
	description: what it says
		usage: Unknown
  dx9-vertex-shader-refs
	description: make sure all vertex shaders are referenced out of the dx9 directory
		usage: Unknown

9. --- Unlocked Scenario Compiling ---
By default tool does not allow any other scenario type than multiplayer to be built. H2Codez allows you to package cache files using different scenario types like singeplayer, shared, single player shared, and mainmenu. 
I will go over how to switch your scenario type in the Sapien section at #20

10. --- Shared Removal ---
When a map is built tool checks the assets of the map against a database for shared and then does not package the assets that exist while leaving a reference to shared in the scenario file. 
H2Codez has an option to ignore this and instead package all dependencies on your map into your map file. This can be used to overwrite default files such as a custom globals.globals which allows for custom bipeds and weapons. 
Do not use this at the moment unless you understand what you are doing. Using this option will leave you with an unknown crash at time of writing. 
Work is being done to see if a cause can be identified in the tags or if it is related to the codebase.

11. --- Guerilla - Links ---
https://mega.nz/#!kl8HFCYQ!8SJbn89GCCHZcVqMaT4GQRbUW7t2hnswHO7d1xf1E_E
Download these files and place them in the bin folder of the map editor. Explanation at #15.

12. ---Advanced Shader View ---
When clicking on the edit button in the toolbar you will see a new option labeled "Advanced Shader View". By default you will see the default shader template system that lets you input bitmaps into named boxes. 
If you click on advanced shader view and open the shader again you will see the layout has changed allowing you to see compiled shader data if it was extracted. This can be useful for attempting to rebuild extracted shaders.

13. --- New Instance ---
When clicking file in Guerilla a new option will appear labeled "New Instance". This just simply opens a second instance of Guerilla.

14. --- Unlocked Tags ---
By default Guerilla can only view and create a few tags. Things like shaders and bitmaps. Even then fields in the tags you can view are hidden leaving you with a very limited tool. 
H2Codez allows us to open and see all the tags Halo 2 accepts. The only one I've found with major issues is the sound tag. 
While you can create, load, and play sound tags you can't actually edit the values in a sound tag. Doing so will crash Guerilla.

15. --- Xbox Audio Codec --- 
Placing this file in the bin folder allows Guerilla you to preview sounds from the sound tag. 
It can also be used as your primary audio codec to compile sound tags in Halo CE which can then be edited in H1 Guerilla and transfered over just fine to Halo 2. 
You can also use it to convert audio files extracted from the cache with tools like Gravemind into playable audio files.

16. --- Baggage.txt ---
Baggage.txt is a txt file that was output upon hitting CTRL+Shift+B in HCE Sapien. In Halo 2 Sapien this causes the program to crash. 
With H2Codez the file will be properly written for you to view. This file can be used to view the amount of data a certain type of tag is taking up allowing you to easily see issues related to overflow.

17. --- HS_doc ---
While the command still exists, it has been added as an option you can click in the menus to generate it. 
It will give you the needed scripting documentation for Halo 2. It has also been modified to output globals script functions.

18. --- Sapien - New Instance ---
If you click file then you will see a new option labeled "New Instance". This will simply let you start a second instance from the file menu.

19. --- Window ---
You will see a new option in the toolbar labeled "Window". It will has three options that you can use to organize the window panels. 
"Cascade" brings all the windows diagonally on the screen. "Tile Horizontally" puts each window into one quadrant of the screen. Very useful to organize your windows easily. "Arrange Icons" seems to do nothing.

20. --- Scenario Type Strings ---
You could choose the scenario type before however the strings for the scenario type were commented out. 
Click on the mission folder in "Hierarchy Pane" then you can change the type in the "Properties Palette". You can select your scenario type here and tool will compile what you decide.

21. --- Run Commands ---
Allows you to run commands from a new input box. Unlike the vanilla console this one lets you copy and paste. 

22. --- Script Execution ---
Clicking this button in the scenario tab will make Sapien run your scripts so that you may see how they work.

23. --- Credit ---
Kornmann00 for his original research into the toolset
Himanshu01 for the creation of H2Codez
Num005(Ale) for contributing to H2Codez and the creation of the H2Codez launcher
Cyboryxmen for his modified JMS export script known as Waltzstreet
TheGhost/CtrlAltDestroy for the modified JMS exporter known as JMS_Exporter_v1-0-3
