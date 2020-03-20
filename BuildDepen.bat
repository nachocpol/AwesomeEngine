:: Check the readme.md file first!

@echo off
echo "Building dependencies..."

echo "====== DirectXText ======"

set _CL_=/MT

MSBuild.exe "Depen/DirectXTex/DirectXTex_Desktop_2017_Win10.sln" /target:Build /p:Configuration=Debug /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /nologo

MSBuild.exe "Depen/DirectXTex/DirectXTex_Desktop_2017_Win10.sln" /target:Build /p:Configuration=Release /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /nologo


echo "====== ASSIMP ======"

MSBuild.exe "Depen/assimp/Assimp.sln" /target:Build /p:Configuration=Debug /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /nologo

MSBuild.exe "Depen/assimp/Assimp.sln" /target:Build /p:Configuration=Release /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /nologo

echo "Reactphysics3D"

::MSBuild.exe "Depen/reactphysics3d/REACTPHYSICS3D.sln" /target:Build /p:Configuration=Debug /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /p:PlatformToolset=v141 /nologo

::MSBuild.exe "Depen/reactphysics3d/REACTPHYSICS3D.sln" /target:Build /p:Configuration=Release /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /p:PlatformToolset=v141 /nologo

echo "====== PhysX ======"

MSBuild.exe "Depen/PhysX/physx/compiler/vc15win64/PhysXSDK.sln" /target:Build /p:Configuration=Debug /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /p:PlatformToolset=v141 /nologo

MSBuild.exe "Depen/PhysX/physx/compiler/vc15win64/PhysXSDK.sln" /target:Build /p:Configuration=Release /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /p:PlatformToolset=v141 /nologo


pause