# reVision

This project is a WIP game engine, with the primary focus being on real-time 3D rendering.
The engine supports OpenGL 4.5 + GLSL 460, deferred rendering, and a PBR workflow (materials and shaders).

## Getting Started

The branch "active" is updated frequently and contains the most recent builds, but may be unstable.
The most stable versions can be found under the branch "master".

The project is designed with CMake in mind, so clone the desired branch and run CMake to generate the project solution.

Note: the engine requires a graphics card capable of supporting OpenGL 4.5 and GLSL 460.

### Required Libraries

This project requires the following (free) libraries:
* [ASSIMP - Model Importing](http://www.assimp.org/)
* [Bullet - Physics Back-end](http://bulletphysics.org/wordpress/)
* [FreeImage - Image Importing](http://freeimage.sourceforge.net/)
* [GLEW - OpenGL](http://glew.sourceforge.net/)
* [GLFW - Windowing](http://www.glfw.org/)
* [GLM - Mathematics](https://glm.g-truc.net/0.9.9/index.html)


### Installation

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

## Versioning

We version our project in the following format: (Major #).(Minor #).(Patch Letter)
For example, Version: 0.171.D, where the major version is 0, minor version is 171, and the patch version is D.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

Thanks to everyone who has helped me over the years!

Special thanks to programming members of [Facepunch](https://forum.facepunch.com/f/), the helpful community on the Stack Exchange, [OGLDev](http://ogldev.atspace.co.uk/index.html), [Learn OpenGL](https://learnopengl.com), and the many other people who run their own tutorial series and programming blogs.