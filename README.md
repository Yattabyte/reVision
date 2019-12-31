# reVision

This is a work-in-progress game engine that I work on during my spare time.  
It has multi-threaded asset loading, phyically based 3D graphics, and object physics.  
My engine uses a custom-built deferred renderer running on OpenGL 4.5, supporting PBR materials, some translucent materials, and skeletal animation.  
Additionally, game world-state is stored and accessed using an entity-component-systems (ECS) framework.


## Getting Started

Starting with version 5.0.0, all major development will now fall under the master branch for easier access.  
Stable binaries can be found under the "Releases" section.


### Requirements
 * CMake + Git
 * C++ 17 MSVC or Clang (haven't tested any other compilers)
 * GPU supporting OpenGL 4.5 + GLSL 460
 * Windows 10 (haven't tested any other x64 OS yet)
 
 
## Usage

**- Step 1:** Pull a copy of this repository: [reVision](https://github.com/Yattabyte/reVision.git)

**- Step 2:** Download [CMake](https://cmake.org/)

**- Step 3:** Run and configure CMake  
Fill out the fields indicating where the project root directory is located.  
**Note:** This project is configured to automatically acquire all it's dependencies if they are missing (GLFW, GLM, etc...)
Only specify manual directories if you really want to use your own versions.  
Once you're done making any changes to the configuration, hit both the CONFIGURE + GENERATE buttons, then open the project.  
![CMake Configuration](https://i.imgur.com/fKUdpKz.png)  

**- Step 4:** Building the Dependencies  
If you've chosen to let CMake handle acquiring all the project dependencies, then before you can compile the project, you must compile all the dependencies first.  
**After compiling, go back to CMake and once more press CONFIGURE + GENERATE to account for any new files produced when compiling this dependencies.**  
**Note:** Repeat this step for both "Debug" and "Release" builds.
![VS2017 Build Example](https://i.imgur.com/HJQAXra.png)  

**- Step 5:** Build the project


## Versioning

The engine version goes as such:  
``(Major #).(Minor #).(Patch #).``  
For example, Version: 1.2.3, where the major version is 1, minor version is 2, and the patch version is 3.  
The major version increments following the completetion of a major milestone, plus changes are pushed into the master branch and a new version is released.  
The minor version increments when a large planned feature is implemented or a category of related changes have been completed.  
The patch version increments when a patch, hotfix, or small feature has been implemented.  


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
![editor UI](https://i.imgur.com/zEh3eb0.png)
![file browser](https://i.imgur.com/72YECGP.gif)
![menu](https://i.imgur.com/tIWcFf7.gif)