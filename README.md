# reVision

This project is a work-in-progress game engine. It features real-time multi-threaded asset loading, and physically based 3D graphics.  The engine uses a custom built OpenGL 4.5 deferred renderer equipped with shader support for PBR materials.  
In addition, most of the engine logic is built upon an entity-component-systems (ECS) framework.  

The active area of development is the in-engine Level Editor, and improvements to the ECS module.

## Getting Started

The branch labeled "active" is updated almost every day, but may be left unstable.
The most stable versions can be found under the branch labeled "master".
Binaries will be found under the "Releases" section, starting at version 5.0.0.

### Requirements
 * CMake + Git
 * Compiler supporting C++ 17
 * GPU supporting OpenGL 4.5 + GLSL 460
 * Windows 10 (haven't tried any other operating systems yet)
 
## Usage

**- Step 1:** Pull a copy of this repository [Revision](https://github.com/Yattabyte/reVision.git)

**- Step 2:** Download [CMake](https://cmake.org/), I recommend using the GUI application

**- Step 3:** Run CMake  
Fill out the fields indicating where the engine's root directory is located.  
**Note:** This engine depends on several third party open-source or otherwise free libraries.   
***However,*** I've configured this cmake project to download and configure them automatically. If you wish, you may supply specific directories to your own versions of these libraries, otherwise leave them blank.
![CMake Configuration](https://i.imgur.com/fKUdpKz.png)  
Once those fields are filled out as desired, hit the configure, generate, and open project buttons.

**- Step 4:** Building the Dependencies  
If you've chosen to let CMake handle the external dependencies for you, build all the external projects first like so:
![VS2017 Build Example](https://i.imgur.com/HJQAXra.png)  
After, return to CMake and hit configure + generate one last time, adding a few more files that were just generated to the reVision project.

**- Step 5:** Build the rest of the solution.

## Versioning

The engine version goes as such:  
``(Major #).(Minor #).(Patch #).``  
For example, Version: 1.2.3, where the major version is 1, minor version is 2, and the patch version is 3.  
The major version increments following the completetion of a major milestone, plus changes are pushed into the master branch and a new version is released.  
The minor version increments when a large planned feature is implemented or a category of related changes have been completed.  
The patch version increments when a patch, hotfix, or small feature has been implemented.  

I try to commit every time the version changes, including the new version in the summary.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
This engine relies on several (free) libraries, though they're not bundled with the source code here.  
 * [ASSIMP - Model Importing](http://www.assimp.org/)
 * [Bullet - Physics Back-end](http://bulletphysics.org/wordpress/)
 * [FreeImage - Image Importing](http://freeimage.sourceforge.net/)
 * [GLEW - OpenGL](http://glew.sourceforge.net/)
 * [GLFW - Windowing](http://www.glfw.org/)
 * [GLM - Mathematics](https://glm.g-truc.net/0.9.9/index.html)
 * [SoLoud - Sound](https://github.com/jarikomppa/soloud)
 
## Acknowledgments

Thanks to everyone who has helped me over the years!

Special thanks to all the helpful members of the former [Facepunch programming subforum](https://forum.facepunch.com/f/) as well as those who moved on to [Knockout](https://knockout.chat/), the helpful community on the Stack Exchange, [OGLDev](http://ogldev.atspace.co.uk/index.html), [Learn OpenGL](https://learnopengl.com), and the many other people who run their own tutorial series and programming blogs.

## Recent Media
![editor UI](https://i.imgur.com/NBzqoQB.png)
![file browser](https://i.imgur.com/72YECGP.gif)
![menu](https://i.imgur.com/tIWcFf7.gif)
