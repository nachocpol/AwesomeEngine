Awesome Stuff
-------------

This is my DX12 rendering framework.

**Requirements:**
* Visual Studio 2017 (> 15.4.5)
* Windows SDK 10.0.16299.0
* MSBuild 15.0 (it should be added to PATH)
* CMake (also added to PATH)

**Dependencies:** 
* Premake
* DirectX Tex
* GLM
* STB
* Assimp
* ReactPhysics3D

**How to build:**

1. Generate the assimp solution using CMake.

2. For PhysX there is a bat file to help with the process: Depen\PhysX\physx\generate_projects.bat. I had some trouble with this step, [my solution][1].

3. Run *BuildDepen.bat* this will compile the dependencies.

4. And finally, run *GenSolution.bat* this will generate the VS solution.


 ![Clouds](Assets/Pics/Clouds.PNG?raw=true "Cloud rendering")

 [1]:https://github.com/NVIDIAGameWorks/PhysX/issues/252
