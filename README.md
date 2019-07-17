# reVision

This is a work-in-progress game engine, with the bulk of its focus on asset loading and real-time 3D rendering.  
The engine runs using an OpenGL 4.5 + GLSL 460 deferred renderer equipped for a PBR material workflow.  
The engine also uses an ECS architecture.

## Getting Started

The branch labeled "active" is updated frequently and contains the most recent builds, but may be unstable.  
The most stable versions can be found under the branch labeled "master".  
Binaries can be found under the "Releases" section, and is updated every time I complete a major version.  

The project has been designed with CMake in mind, as it depends on several external libraries to compile.

Note: the engine requires a graphics card capable of supporting OpenGL 4.5 and GLSL 460 to run.

### Required Libraries

This project requires the following (free) libraries:
* [ASSIMP - Model Importing](http://www.assimp.org/)
* [Bullet - Physics Back-end](http://bulletphysics.org/wordpress/)
* [FreeImage - Image Importing](http://freeimage.sourceforge.net/)
* [GLEW - OpenGL](http://glew.sourceforge.net/)
* [GLFW - Windowing](http://www.glfw.org/)
* [GLM - Mathematics](https://glm.g-truc.net/0.9.9/index.html)

## Versioning

The engine version goes as such:  
``(Major #).(Minor #).(Patch #).``  
For example, Version: 1.2.3, where the major version is 1, minor version is 2, and the patch version is 3.  
The major version increments following the completetion of a major milestone, plus changes are pushed into the master branch and a new version is released.  
The minor version increments when a large planned feature is implemented or a category of related changes have been completed.  
The patch version increments when a patch, hotfix, or small feature has been implemented.  

I try to commit every time the version changes, including the new version in the summary.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

Thanks to everyone who has helped me over the years!

Special thanks to all the helpful members of the former [Facepunch programming subforum](https://forum.facepunch.com/f/) as well as those who moved on to [Knockout](https://knockout.chat/), the helpful community on the Stack Exchange, [OGLDev](http://ogldev.atspace.co.uk/index.html), [Learn OpenGL](https://learnopengl.com), and the many other people who run their own tutorial series and programming blogs.


## Installation

Step 1: Download [CMake](https://cmake.org/)

```
Download CMake from the link above and install it
```

Step 2: Running CMake

```
Here, we want to configure it for our project.
First, fill in the "Where is the source code" field to the main project directory, and the second "build" field to where you want to build the project.
Second, fill in the main directories for all the required external libraries, like assimp, bullet, etc.
Third, hit the configure button and choose the compiler you want to generate the solution for. Then hit the generate button after.
```

Step 3: Building the project

```
The project comes as a single solution for the engine, and a separate solution for generating the optional Doxygen documentation.
To avoid duplicating the engine assets for multiple builds (debug, release, x32/x64, etc) they are kept within the 'app' folder. If the executable doesn't have it set already, change it to start in the app folder.
```