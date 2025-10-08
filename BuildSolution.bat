call GenSolution.bat

echo "====== AwesomeSolution ======"

MSBuild.exe "AwesomeSolution.sln" /target:Build /p:Configuration=Debug /p:Platform=x64 /nologo -m

MSBuild.exe "AwesomeSolution.sln" /target:Build /p:Configuration=Release /p:Platform=x64 /nologo -m

pause