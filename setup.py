from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

ext = Pybind11Extension(
    "matrix1",
    sources=["matrix1_py.cpp"],
    cxx_std=17,
)

setup(
    name="matrix1",
    ext_modules=[ext],
    cmdclass={"build_ext": build_ext},
)
