.RECIPEPREFIX:=>

# This makefile takes the following variables:
#
#	CC            - C compiler.
#   CXX           - C++ compiler.
# 
#   BUILD_DIR     - Library build directory.
#
#	_CFLAGS       - C compiler flags.
#   _CXXFLAGS     - C++ compiler flags.
#   _CWARN_OPTS   - C compiler warnings.
#   _CXXWARN_OPTS - C++ compiler warnings.
#   _LDFLAGS      - Linker flags.


CXXFLAGS=${_CXXFLAGS}
CXXWARN_OPTS=${_CXXWARN_OPTS} -Wno-unused-parameter -Wno-misleading-indentation\
             -Wno-class-memaccess
LDFLAGS=${_LDFLAGS}

SOURCES_LIB=error.cc c_iface.cc lockable.cc opt.cc map_area_set.cc
OBJECTS_LIB=${SOURCES_LIB:%.cc=${BUILD_DIR}/%.o}

SHARED=libscry.so
STATIC=libscry.a


shared: ${SHARED}
> mkdir -p ${BUILD_DIR}
> mv ${SHARED} ${BUILD_DIR}

static: ${STATIC}
> mkdir -p ${BUILD_DIR}
> mv ${STATIC} ${BUILD_DIR}

${SHARED}: ${OBJECTS_LIB}
> ${CXX} ${CXXFLAGS} -shared -o $@ $^ ${LDFLAGS}

${STATIC}: ${OBJECTS_LIB}
> ar rcs $@ $^

${BUILD_DIR}/%.o: %.cc
> ${CXX} ${CXXFLAGS} ${CXXWARN_OPTS} -c $< -o $@

clean:
> -rm -v ${BUILD_DIR}/${SHARED}
> -rm -v ${BUILD_DIR}/${STATIC}
> -rm -v ${OBJECTS_LIB}
