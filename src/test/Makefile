.RECIPEPREFIX:=>

# This makefile takes the following variables:
#
# CC            - C compiler.
# CXX           - C++ compiler.
# BUILD_DIR     - Unit test build directory.
# LIB_BIN_DIR   - Library artifact directory.
#
# _CFLAGS       - C compiler flags.
# _CXXFLAGS     - C++ compiler flags.
# _CWARN_OPTS   - C compiler warnings.
# _CXXWARN_OPTS - C++ compiler warnings.


#[parameters]
CFLAGS=${_CFLAGS} -fsanitize=address -fsanitize-recover=address
WARN_OPTS+=${_WARN_OPTS} -Wno-unused-variable -Wno-unused-but-set-variable
LDFLAGS=-L${LIB_BIN_DIR} \
        -Wl,-rpath=${LIB_BIN_DIR} -lscry -lmcry -lcmore -lpthread -static-libasan


#[build constants]
SOURCES_TEST=main.cc common.cc filters.cc memcry_helper.cc opt_helper.cc \
             scan_helper.cc target_helper.cc test_map_area_set.cc \
			 test_opt.cc test_worker_pool.cc test_serialiser.cc
OBJECTS_TEST=${SOURCES_TEST:%.cc=${BUILD_DIR}/%.o}
TARGET_DIR=${shell pwd}/target


#[targets]
TESTS=test

tests: tgt ${TESTS}
> mkdir -p ${BUILD_DIR}
> mv ${TESTS} ${BUILD_DIR}

${TESTS}: ${OBJECTS_TEST}
> ${CXX} ${CFLAGS} ${WARN_OPTS} -o $@ $^ ${LDFLAGS}

${BUILD_DIR}/%.o: %.cc
> ${CXX} ${CFLAGS} ${WARN_OPTS} -c $< -o $@

tgt:
> $(MAKE) -C ${TARGET_DIR} target CC='${CC}' BUILD_DIR='${BUILD_DIR}'

clean:
> ${MAKE} -C ${TARGET_DIR} clean BUILD_DIR='${BUILD_DIR}'
> -rm -v ${BUILD_DIR}/${TESTS}
> -rm -v ${OBJECTS_TEST}
