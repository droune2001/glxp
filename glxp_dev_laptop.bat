@echo off

cd C:\Users\Nicolas\home\dev\glxp

echo Setup Visual Studio 2017 Environment Variables
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

echo Set Tools Environment Variables
set MY_CMAKE_PATH=C:\Users\Nicolas\home\bin\cmake-3.12.2-win64-x64\bin
set PATH=%MY_CMAKE_PATH%;%PATH%

echo Set Librairies Environment Variables
set GLM_DIR=C:\Users\Nicolas\home\dev\glm-0.9.9.0
set GLFW_DIR=C:\Users\Nicolas\home\dev\glfw-3.2.1
set GLEW_DIR=C:\Users\Nicolas\home\dev\glew-2.1.0

echo Make aliases
doskey dev=devenv build\glxp.sln
doskey cmaker=cmake -G"Visual Studio 15 2017 Win64" -Hsrc -Bbuild
