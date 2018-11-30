@echo off

D:
cd D:\dev\glxp

echo Setup Visual Studio 2015 Environment Variables
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

echo Set Tools Environment Variables
set MY_CMAKE_PATH=D:\dev\bin\cmake-3.12.2-win64-x64\bin
set PATH=%MY_CMAKE_PATH%;%PATH%

echo Set Librairies Environment Variables
set GLM_DIR=D:\dev\glm-0.9.9.0
set GLFW_DIR=D:\dev\glfw-3.2.1
set GLEW_DIR=D:\dev\glew-2.1.0

echo Make aliases
doskey dev=devenv build\glxp.sln
doskey cmaker=cmake -G"Visual Studio 14 2015 Win64" -Hsrc -Bbuild
