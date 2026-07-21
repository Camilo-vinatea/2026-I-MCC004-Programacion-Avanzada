#!/usr/bin/env bash
# Build demovtk con toolchain MSYS2 MINGW64. Correr: make demovtk-msys
set -e
cd "$(dirname "$0")"
which gcc cmake
rm -rf build-vtk
cmake -S . -B build-vtk -G Ninja
cmake --build build-vtk
echo "OK -> build-vtk/demovtk.exe"
