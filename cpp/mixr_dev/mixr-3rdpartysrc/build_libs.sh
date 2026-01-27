#
# To use the functions defined in this script, type the following:
#
# $ source build_libs.sh
# $ help
#
# To compile and install everything in the location defined by
# $MIXR_3RD_PARTY_ROOT
#
# $ install_all
#

function help
{
   echo "Commands available";
   echo "init_paths";
   echo "install_osg";
   echo "install_cigi";
   echo "install_jsbsim";
   echo "install_protobuf";
   echo "install_hla";
   echo "install_zeromq";
   echo "install_ftgl";
   echo "install_freeglut";
   echo "install_flex";
   echo "install_bison";
   echo "install_all";
}

# initialize the location to install libraries
function init_paths
{
   # check for location to install 3rd party libs
   if [ -z "$MIXR_3RD_PARTY_ROOT" ]; then
      echo "Need to source setenv from within mixr"
      exit 1
   fi

   # location of install libs
   rm -rf $MIXR_3RD_PARTY_ROOT
   mkdir $MIXR_3RD_PARTY_ROOT
   # temporary location to build libs
   rm -rf tmp
   mkdir tmp

   export PATH=$MIXR_3RD_PARTY_ROOT/bin:$PATH
}

# OpenSceneGraph (OSG)
function install_osg
{
# check for cmake, needed to compile OSG libs
   if [ -x "$(command -v cmake)" ]; then
      echo 'cmake found, OpenSceneGraph (OSG) being compiled...';
      pushd .
      tar xzvf OpenSceneGraph-3.6.3.tar.gz --directory tmp
      mv tmp/OpenSceneGraph-OpenSceneGraph-3.6.3 tmp/osg
      mkdir tmp/osg-build
      cd tmp/osg-build
      cmake -DCMAKE_INSTALL_PREFIX=$MIXR_3RD_PARTY_ROOT \
            -DDYNAMIC_OPENSCENEGRAPH:BOOL="0" \
            -DDYNAMIC_OPENTHREADS:BOOL="0" \
            -DOSG_USE_QT:BOOL="0" \
            ../osg
      make -j 4
      make install
      popd
   else
      echo 'cmake NOT found, OpenSceneGraph (OSG) not compiled!' >&2
   fi
}

# cigi
function install_cigi
{
   echo 'CIGI being compiled...' >&2
   pushd .
   tar xzvf ccl_3_3_3a.tar.gz --directory tmp
   cd tmp/ccl
   ./configure --prefix=$MIXR_3RD_PARTY_ROOT --disable-shared
   make -j all
   make install
   popd
}

# jsbsim
function install_jsbsim
{
   echo 'JSBSim being compiled...' >&2
   pushd .
   tar xzvf jsbsim_cvs_v2016_0203.tgz --directory tmp
   cd tmp/jsbsim
   ./autogen.sh --prefix=$MIXR_3RD_PARTY_ROOT --enable-libraries
   make -j all
   make install
   popd
}

# protobuf
function install_protobuf
{
   echo 'Google Protobuf being compiled...' >&2
   pushd .
   tar xzvf protobuf-2.6.1.tar.gz --directory tmp
   cd tmp/protobuf-2.6.1
   ./configure --prefix=$MIXR_3RD_PARTY_ROOT --disable-shared
   make -j all
   make install
   popd
}

# OpenRTI
function install_hla
{
   # check for cmake, needed to compile HLA libs
   if [ -x "$(command -v cmake)" ]; then
      echo 'cmake found, OpenRTI (HLA) being compiled...' >&2
      pushd .
      tar jxvf OpenRTI-0.9.0.tar.bz2 --directory tmp
      mkdir tmp/openrti-build
      cd tmp/openrti-build
      cmake -DCMAKE_INSTALL_PREFIX=$MIXR_3RD_PARTY_ROOT \
            -DOPENRTI_ENABLE_PYTHON_BINDINGS:BOOL=OFF \
            -DOPENRTI_BUILD_SHARED:BOOL=OFF \
            ../OpenRTI-0.9.0
      make -j all
      make install
      popd
   else
      echo 'cmake NOT found, OpenRTI (HLA) not compiled!' >&2
   fi
}

# zeromq
function install_zeromq
{
   echo 'ZeroMQ being compiled...' >&2
   pushd .
   tar xzvf zeromq-4.2.5.tar.gz --directory tmp
   cd tmp/zeromq-4.2.5
   ./configure --prefix=$MIXR_3RD_PARTY_ROOT
   make -j all
   make install
   popd
}

# ftgl (depends upon freetype)
function install_ftgl
{
   echo 'FTGL being compiled...' >&2
   pushd .
   tar xzvf ftgl-2.1.3-rc5.tar.gz --directory tmp
   cd tmp/ftgl-2.1.3~rc5
   ./configure --prefix=$MIXR_3RD_PARTY_ROOT --disable-shared
   make -j all
   make install
   popd
}

# freeglut
# requires XInput.h (e.g., libxi-dev to be installed)
function install_freeglut
{
   # check for cmake, needed to compile freeglut lib
   if [ -x "$(command -v cmake)" ]; then
      echo 'cmake found, FreeGlut being compiled...' >&2
      pushd .
      tar xvf freeglut-3.0.0.tar.gz --directory tmp
      mkdir tmp/freeglut-build
      cd tmp/freeglut-build
      cmake -DCMAKE_INSTALL_PREFIX=$MIXR_3RD_PARTY_ROOT \
            ../freeglut-3.0.0
      make -j all
      make install
      popd
   else
      echo 'cmake NOT found, FreeGlut not compiled!' >&2
   fi
}

# flex
function install_flex
{
   echo 'flex being compiled...' >&2
   pushd .
   tar xzvf parser-utils/flex-2.5.39.tar.gz --directory tmp
   cd tmp/flex-2.5.39
   ./configure --prefix=$MIXR_3RD_PARTY_ROOT --disable-shared
   make -j all
   make install
   popd
}

# bison
function install_bison
{
   echo 'bison being compiled...' >&2
   pushd .
   tar xzvf parser-utils/bison-3.2.2.tar.gz --directory tmp
   cd tmp/bison-3.2.2
   ./configure --prefix=$MIXR_3RD_PARTY_ROOT --disable-shared
   make -j all
   make install
   popd
}

# install all libraries
function install_all
{
   init_paths
   install_osg
   install_cigi
   install_jsbsim
   install_protobuf
   install_hla
   install_zeromq
   #install_ftgl
   #install_freeglut
   install_bison
   install_flex
}


