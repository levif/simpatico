#-----------------------------------------------------------------------
# This makefile file defines path variables that specify local installation
# paths
#-----------------------------------------------------------------------

# if HOOMD_FLAG is defined, the path to the HOOMD installation needs to be 
# set here
HOOMD_INSTALL_PATH=${HOME}/local

# the path to python needs to be set here
PYTHON_INCLUDE_PATH=${HOME}/local/include/python2.7

# If HOOMD_FLAG is defined, the path to cuda needs to be set here
CUDA_INSTALL_PATH=/usr/local/cuda

# the path to the boost libraries needs to be set here (if not in the
# standard paths)
BOOST_INCLUDE_PATH=${HOME}/local/include

#-----------------------------------------------------------------------
# everything below this needs not to be changed
#-----------------------------------------------------------------------

INCLUDES+= -I$(HOOMD_INSTALL_PATH)/include -I$(CUDA_INSTALL_PATH)/include -I$(PYTHON_INCLUDE_PATH)
ifdef BOOST_INCLUDE_PATH
INCLUDES+= -I$(BOOST_INCLUDE_PATH)
endif
HOOMD_LIB=$(HOOMD_INSTALL_PATH)/lib/hoomd/python-module/hoomd.so
LIBS+=$(HOOMD_LIB)
MCMD_DEFS+= -DENABLE_CUDA -DSINGLE_PRECISION

