# Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
# for details. All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE.md file.

.PHONY: all debug test clean

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
PLATFORM_INCLUDES=-I${JAVA_HOME}/include/linux
PLATFORM_LIB_SUFFIX=so
endif

ifeq ($(UNAME), Darwin)
PLATFORM_INCLUDES=-I${JAVA_HOME}/include/darwin
PLATFORM_LIB_SUFFIX=jnilib
endif

ARCH=64
FLETCH_BUILD=ReleaseX64

CPPFLAGS=-fPIC -m${ARCH} -I../../../../include -I${JAVA_HOME}/include \
	${PLATFORM_INCLUDES}
THIS_DIR=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
FLETCH_DIR=$(abspath ../../../..)
FLETCH_BUILD_DIR=$(abspath ${FLETCH_DIR}/out/${FLETCH_BUILD})
FLETCH=${FLETCH_BUILD_DIR}/fletch

JAVA=${JAVA_HOME}/bin/java
JAVAC=${JAVA_HOME}/bin/javac
JAVA_SOURCES:=$(wildcard *.java)
JAVA_CLASSES:=$(JAVA_SOURCES:.java=.class)

TEMP_DIR:=$(shell mktemp -u)

all: $(JAVA_CLASSES) DebugRunner.class fletch/conformance_service.jar \
	jni/libfletch.${PLATFORM_LIB_SUFFIX} conformance.snapshot

$(JAVA_CLASSES): $(JAVA_SOURCES) fletch/conformance_service.jar
	$(JAVAC) -classpath fletch/conformance_service.jar -d . -implicit:none \
	  -source 8 -target 8 -sourcepath none -Werror $(JAVA_SOURCES)

fletch/conformance_service.jar: ../conformance_service.idl
	$(FLETCH) x-servicec file ../conformance_service.idl out ..
	mkdir -p $(TEMP_DIR)
	javac -classpath none -d $(TEMP_DIR) -implicit:none -source 8 -target 8 \
		-sourcepath none -Werror fletch/*.java
	jar cf $@ -C $(TEMP_DIR) .
	rm -rf $(TEMP_DIR)

jni/libfletch.so: jni/fletch_api_wrapper.o jni/fletch_service_api_wrapper.o \
	jni/conformance_service_wrapper.o ${FLETCH_BUILD_DIR}/libfletch.a
	g++ ${CPPFLAGS} -shared -o $@ $^

jni/libfletch.jnilib: jni/fletch_api_wrapper.o \
	jni/fletch_service_api_wrapper.o jni/conformance_service_wrapper.o \
	${FLETCH_BUILD_DIR}/libfletch.a
	g++ ${CPPFLAGS} -framework CoreFoundation -shared -o $@ $^

conformance.snapshot: ../conformance_service_impl.dart
	cd ${FLETCH_DIR};
	${FLETCH_BUILD_DIR}/fletch export ${THIS_DIR}$< to ${THIS_DIR}$@

test: all
	LD_LIBRARY_PATH=jni \
	$(JAVA) -Djava.library.path=jni -enableassertions -classpath \
	  fletch/conformance_service.jar:. ConformanceTest conformance.snapshot

PORT=8123
debug: all
	LD_LIBRARY_PATH=jni \
	$(JAVA) -Djava.library.path=jni -ea -cp fletch/conformance_service.jar:.
	  ConformanceTest $(PORT)

clean:
	rm -f *.class fletch/*.class fletch/conformance_service.jar
	rm -f jni/*.o jni/*.$(PLATFORM_LIB_SUFFIX) conformance.snapshot
