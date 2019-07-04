[![Build Status](https://travis-ci.org/tobspr/P3DModuleBuilder.svg?branch=master)](https://travis-ci.org/tobspr/P3DModuleBuilder)

# Toontown OTP CXX
A C++ implementation of the Toontown OTP's Message Director component for improved performance over the Python implementation with bindings for Python.

#### 3. Compile the module

To compile libotp, run `python build.py`.
When the compilation finished, there should now be a `libotp.pyd` / `libotp.so` (depending on your platform) generated. This file gets placed in the source repository of the game

####

## Requirements

- The Panda3D SDK (get it <a href="http://www.panda3d.org/download.php?sdk">here</a>)
- CMake 2.6 or higher (get it <a href="https://cmake.org/download/">here</a>)
- windows only: The thirdparty folder installed in the Panda3D sdk folder (See <a href="https://www.panda3d.org/forums/viewtopic.php?f=9&t=18775">here</a>)


**For compiling on Windows 32 bit:**

- Visual Studio 2010/2015

**For compiling on Windows 64 bit:**

- Visual Studio 2010/2015
- Windows SDK 7.1 (be sure to tick the VC++ 64 bit compilers option)


## Advanced configuration

**Please clean up your built directories after changing the configuration! You can
do so with passing `--clean` in the command line.**


### Command Line
Command line options are:

- `--optimize=N` to override the optimize option. This overrides the option set in the `config.ini`
- `--clean` to force a clean rebuild

### config.ini
Further adjustments can be made in the `config.ini` file:

- You can set `generate_pdb` to `0` or `1` to control whether a `.pdb` file is generated.
- You can set `optimize` to change the optimization. This has to match the `--optimize=` option of your Panda3D Build.
- You can set `require_lib_eigen` to `1` to require the Eigen 3 library
- You can set `require_lib_bullet` to `1` to require the Bullet library
- You can set `require_lib_freetype` to `1` to require the Freetype library
- You can set `verbose_igate` to `1` or `2` to get detailed interrogate output (1 = verbose, 2 = very verbose)
