
Mixed Reality Simulation Platform (MIXR) 3rd Party Source
=========================================================

The most current version of the mixed reality simulation platform can be found at [www.mixr-platform.org][mixr].

* This package includes the source code for MIXR's 3rd party dependency libraries.

* We recommend compiling and installing these libraries into a directory within your user account to avoid any potential clashes with installed system files. The script file `build_libs.sh` includes a number of bash shell functions to compile and install almost all required MIXR dependency libraries into your local user account. As written, it does assumes you have installed the developmental versions (library and header files) for `freetype`, `freeglut` and, `ftgl`. Make sure the environment variables defined for MIXR are set by running `source setenv.sh` within the `mixr` directory.  The variable MIXR_3RD_PARTY_ROOT defines where the 3rd party libaries will be placed.

* Make sure to have `autogen`, `automake`, `libtool`, `libtool-bin`, and `cmake` installed.  These are essential prerequisite build tools for the dependency libraries.

* It is not absolutely required, but recommended to install GDAL (e.g., apt-get install libgdal-dev) as it is an important OSG dependency for many of its plug-ins.

* If you plan to use the Qt library, download and install the complete Qt package; we make no attempt at bundling Qt as a 3rd party library as has a large footprint, especially if you include the entire ecosystem of supporting tools. At the time of this writting, we are using Qt 5.12.

* If you are going to use the Clang compiler, make sure to compile the provided libraries with that compiler and install them into a directory within your account; this keeps them away from your system directories. Google Protocol Buffers in particular, conflicts with other system tools if compiled with Clang and installed in `/usr/local/lib`. The Clang-compiled version of this library does not play nice with other GCC-compiled versions that might be installed in other places. Ignoring this advise might result in a non-working Linux system! To keep things clean and simple, we recommend keeping all compiled 3rd party libraries local.

* If you would like to use Clang as the default compiler for all projects, consider adding `export CC=clang` and `export CXX=clang++` to your .bashrc (or equivalent) file.  Almost all build tools are smart enough to use the compiler specified by environment variables `CC` and `CXX`.

* * *

The following is more explicit step-by-step instructions to build individual libraries.

* When using `configure`, individual libraries can be targeted for installation to a particular directory using the command line argument `--prefix=<path>`.  If this is not specified, the typical default path is `/usr/local`.

As an example, consider the following directory structure:

`/home/me` # location of my user account ('me')

`/home/me/mixr-platform` # the place where all MIXR related work is located

`/home/me/mixr-platform/mixr` # base platform code

`/home/me/mixr-platform/mixr-examples` # MIXR examples

`/home/me/mixr-platform/mixr-3rdpartysrc` # source code to 3rd party packages

`/home/me/mixr-platform/mixr-3rdparty` # desired location of where 3rd party dependencies will be installed (determined by MIXR_3RD_PARTY_ROOT environment variable)

After unzipping or untarring ccl, jsbsim, zeromq, protobuf, etc, enter each directory and run `configure` as follows to setup the build system:

`./configure --prefix=/home/me/mixr-platform/mixr_3rdparty` # for most libraries

`./autogen.sh --prefix=home/me/mixr-platform/mixr_3rdparty --enable-libraries` # for JSBSim

Next, use `make` to compile and create libraries, followed by `make install` to copy or install them to the directory specified.  The example will compile all libraries and install them to `/home/me/mixr-platform/mixr_3rdparty`.

* Several dependencies can be installed via `apt-get` or `yum` (e.g., FTGL, FreeType and FreeGlut) if Internet is available. Other libraries, such as cigi and JSBSim are not available in the public repositories, hence the need for these packages.

* For the libraries being manually compiled and installed, make sure to use the ones provided here.  As an example, the version of JSBSim included here, defines an API that MIXR is designed to use.  We do our best to ensure compatibility with a wide range of versions but not everything is 100% all the time; to be safe, use the ones provided.

[mixr]: http://www.mixr-platform.org
