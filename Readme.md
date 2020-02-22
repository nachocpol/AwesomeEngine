Awesome Stuff
-------------

This is my DX12 rendering framework.

**Requirements:**
* Visual Studio 2017
* Windows SDK 10.0.16299.0
* MSBuild 15.0 (it should be added to your PATH)
* CMake

**To build:**

1. First, you will need to generate the assimp solution using CMake GUI. Set generator to: **Visual Studio 15 2017 Win64**. Then go ahead and generate the solution.

2. Go into Depen/bullet and run *build_visual_studio_vr_pybullet_double.bat*.

3. Run *BuildDepen.bat* this will compile the dependencies.

4. And finally, run *GenSolution.bat* this will generate the VS solution.


 ![Clouds](Assets/Pics/Clouds.PNG?raw=true "Cloud rendering")
