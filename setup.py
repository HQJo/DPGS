from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __str__(self):
        import pybind11
        return pybind11.get_include()


ext_modules = [
    Extension("DPGS",
              ["src/graph.cpp", "src/union_find.cpp", "src/summarize.cpp",
                  "src/utils.cpp", "python_bind.cpp"],
              include_dirs=[
                  "./include",
                  get_pybind_include()
              ],
              language='c++',
              extra_compile_args=['-std=c++11', '-O3']
              )
]

setup(
    name='DPGS',
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext}
)
