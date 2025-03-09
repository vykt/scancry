.RECIPEPREFIX:=>

# This makefile takes the following variables:
#
# CC          - C compiler.
# BUILD_DIR   - Unit test build directory.


CFLAGS=-O0 -ggdb
CWARN_OPTS=-Wno-unused-variable -Wno-unused-but-set-variable
LDFLAGS=-L${LIB_BIN_DIR} -Wl,-rpath=${LIB_BIN_DIR}

SOURCES_TARGET=unit_target.c
OBJECTS_TARGET=${SOURCES_TARGET:%.c=${BUILD_DIR}/%.o}

TARGET=unit_target

target: ${TARGET}
> mkdir -p ${BUILD_DIR}
> mv ${TARGET} ${BUILD_DIR}

${TARGET}: ${OBJECTS_TARGET}
> ${CC} ${CFLAGS} ${CWARN_OPTS} -o $@ $^ ${LDFLAGS}

${BUILD_DIR}/%.o: %.c
> ${CC} ${CFLAGS} ${CWARN_OPTS} -c $< -o $@

clean:
> -rm -v ${BUILD_DIR}/${TARGET}
> -rm -v ${OBJECTS_TARGET}
