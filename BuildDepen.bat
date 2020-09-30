:: Check the readme.md file first!

:: /p:WindowsTargetPlatformVersion=%WindowsVersion%
:: /p:PlatformToolset=v141

@echo off
echo "Building dependencies..."

set WindowsVersion = 10.0.16299.0

echo "====== DirectXText ======"

set _CL_=/MT

MSBuild.exe "Depen/DirectXTex/DirectXTex_Windows10_2019.sln" /target:Build /p:Configuration=Debug /p:Platform=x64 /nologo

MSBuild.exe "Depen/DirectXTex/DirectXTex_Windows10_2019.sln" /target:Build /p:Configuration=Release /p:Platform=x64 /nologo


echo "====== ASSIMP ======"

MSBuild.exe "Depen/assimp/Assimp.sln" /target:Build /p:Configuration=Debug /p:Platform=x64 /nologo

MSBuild.exe "Depen/assimp/Assimp.sln" /target:Build /p:Configuration=Release /p:Platform=x64  /nologo


echo "====== PhysX ======"

MSBuild.exe "Depen/PhysX/physx/compiler/vc16win64/PhysXSDK.sln" /target:Build /p:Configuration=Debug /p:Platform=x64  /nologo

MSBuild.exe "Depen/PhysX/physx/compiler/vc16win64/PhysXSDK.sln" /target:Build /p:Configuration=Release /p:Platform=x64 /nologo


pause