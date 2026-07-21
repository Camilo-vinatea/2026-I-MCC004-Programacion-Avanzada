CXX = g++
CXXFLAGS = -std=c++2b -Wall -g -pthread # Añadido -pthread
LDFLAGS = -pthread # Añadido -pthread

TARGET = main
SRCS = main.cpp util.cpp \
       complex.cpp \
       shapes/shape.cpp \
       shapes/rectangle.cpp \
       shapes/circle.cpp \
       shapes/triangle.cpp \
       shapes/square.cpp \
       polimorfismo.cpp \
       BitSigno.cpp \
       Pointers.cpp \
       array1.cpp \
       performance.cpp \
       regexdemo.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

python_module:
	python setup.py build_ext --inplace

# VTK: correr desde shell MSYS2 MINGW64. Ver build-vtk.sh (cmake Ninja).
demovtk:
	bash build-vtk.sh

run-demovtk: demovtk
	./build-vtk/demovtk.exe

# GPU: compila performance.cpp con CUDA. Requiere CUDA Toolkit (nvcc) + MSVC.
# Correr desde "x64 Native Tools Command Prompt for VS" (setea INCLUDE/LIB de cl.exe).
# RTX 5060 = Blackwell = sm_120. VS 18/MSVC nuevo -> -allow-unsupported-compiler.
NVCC = nvcc
CUDA_ARCH = sm_120
gpu:
	$(NVCC) -O2 -std=c++17 -x cu -arch=$(CUDA_ARCH) -allow-unsupported-compiler \
	        performance.cpp -o performance_gpu
	@echo "Ejecutar: ./performance_gpu"

.PHONY: all clean python_module demovtk run-demovtk gpu