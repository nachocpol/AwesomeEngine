echo off
echo "Building dependencies..."

echo "====== DirectXText ======"

MSBuild.exe "Depen/DirectXTex/DirectXTex_Desktop_2017_Win10.sln" /target:Build /p:Configuration=Debug /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /nologo

MSBuild.exe "Depen/DirectXTex/DirectXTex_Desktop_2017_Win10.sln" /target:Build /p:Configuration=Release /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /nologo


echo "====== ASSIMP ======"

MSBuild.exe "Depen/assimp/Assimp.sln" /target:Build /p:Configuration=Debug /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /nologo

MSBuild.exe "Depen/assimp/Assimp.sln" /target:Build /p:Configuration=Release /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /nologo

pause