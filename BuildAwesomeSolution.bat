call GenSolution.bat

echo "====== AwesomeSolution ======"

MSBuild.exe "AwesomeSolution.sln" /target:Build /p:Configuration=Debug /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /p:PlatformToolset=v141 /nologo

MSBuild.exe "AwesomeSolution.sln" /target:Build /p:Configuration=Release /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.16299.0 /p:PlatformToolset=v141 /nologo


pause