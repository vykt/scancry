.RECIPEPREFIX:=>


#[set as required]
INSTALL_DIR=/usr/local/lib
INCLUDE_INSTALL_DIR=/usr/local/include
LD_DIR=/etc/ld.so.conf.d

CC=gcc
CXX=g++

FLAGS=-flto -Wno-sign-compare
FLAGS_TEST=-ggdb3 -O0
WARN_OPTS=-Wall -Wextra

CFLAGS=
CXXFLAGS=
CFLAGS_TEST=
CXXFLAGS_TEST=
CWARN_OPTS=
CXXWARN_OPTS=
LDFLAGS=-pthread -lmcry -lcmore


#[build constants]
LIB_DIR=./src/lib
TEST_DIR=./src/test
BUILD_DIR=${shell pwd}/build
PACKAGE_DIR=./package


#[installation constants]
SHARED=libscry.so
STATIC=libscry.a
HEADER=scancry.h


#[set build options]
ifeq ($(build),debug)
	FLAGS      += -O0 -ggdb3 -fsanitize=address -DDEBUG
	FLAGS_TEST += -DDEBUG
 	LDFLAGS    += -static-libasan
else
	CFLAGS += -O2
endif

#[enable tracing]
ifeq ($(trace_worker),on)
	FLAGS      += -DTRACE -DTRACE_WORKER
	FLAGS_TEST += -DTRACE -DTRACE_WORKER
endif

ifeq ($(trace_ptrscan),on)
	FLAGS      += -DTRACE -DTRACE_PTRSCAN
	FLAGS_TEXT += -DTRACE -DTRACE_PTRSCAN
endif



#[set static analysis options]
ifeq ($(fanalyzer),true)
	FLAGS += -fanalyzer
endif


#[set final flags]
CFLAGS        += $(FLAGS)
CXXFLAGS      += $(FLAGS)
CFLAGS_TEST   += $(FLAGS_TEST)
CXXFLAGS_TEST += $(FLAGS_TEST)
CWARN_OPTS    += $(WARN_OPTS)
CXXWARN_OPTS  += $(WARN_OPTS)


#[process targets]
.PHONY prepare:
> mkdir -p ${BUILD_DIR}/test ${BUILD_DIR}/lib ${PACKAGE_DIR}

test: shared
> $(MAKE) -C ${TEST_DIR} tests CC='${CC}' CXX='${CXX}' \
                               _CFLAGS='${CFLAGS_TEST}' \
                               _CXXFLAGS='${CXXFLAGS_TEST}' \
                               _CWARN_OPTS='${CWARN_OPTS}' \
                               _CXXWARN_OPTS='${CXXWARN_OPTS}' \
                               BUILD_DIR='${BUILD_DIR}/test' \
                               LIB_BIN_DIR='${BUILD_DIR}/lib'

all: shared static

shared:
> $(MAKE) -C ${LIB_DIR} shared CXX='${CXX}' _CXXFLAGS='${CXXFLAGS} -fPIC' \
	                           _CXXWARN_OPTS='${CXXWARN_OPTS}' \
	                           _LDFLAGS='${LDFLAGS}' \
	                           BUILD_DIR='${BUILD_DIR}/lib'

static:
> $(MAKE) -C ${LIB_DIR} static CXX='${CXX}' _CXXFLAGS='${CXXFLAGS}' \
	                           _CXXWARN_OPTS='${CXXWARN_OPTS}' \
	                           _LDFLAGS='${LDFLAGS}' \
	                           BUILD_DIR='${BUILD_DIR}/lib'

clean:
> $(MAKE) -C ${TEST_DIR} clean BUILD_DIR='${BUILD_DIR}/test'
> $(MAKE) -C ${LIB_DIR} clean BUILD_DIR='${BUILD_DIR}/lib'
> -rm ${PACKAGE_DIR}/*

install:
> mkdir -pv ${INSTALL_DIR}
> cp -v ${BUILD_DIR}/lib/${SHARED} ${INSTALL_DIR}
> cp -v ${BUILD_DIR}/lib/${STATIC} ${INSTALL_DIR}
> mkdir -pv ${INCLUDE_INSTALL_DIR}
> cp -v ${LIB_DIR}/${HEADER} ${INCLUDE_INSTALL_DIR}
> echo "${INSTALL_DIR}" > ${LD_DIR}/90scancry.conf
> ldconfig

uninstall:
> -rm -v ${INSTALL_DIR}/{${SHARED},${STATIC}}
> -rm -v ${INCLUDE_INSTALL_DIR}/${HEADER}
> -rm ${LD_DIR}/90scancry.conf
> ldconfig

package: all
> -cp ${BUILD_DIR}/lib/${SHARED} ${PACKAGE_DIR}
> -cp ${BUILD_DIR}/lib/${STATIC} ${PACKAGE_DIR}
> -cp ${LIB_DIR}/memcry.h ${PACKAGE_DIR}
> -tar cvjf ${PACKAGE_DIR}/memcry.tar.bz2 ${PACKAGE_DIR}/*
