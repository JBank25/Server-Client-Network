#!/usr/bin/env bash

BASE_MODULE_DIR="src/NewModule"

MODULE_CMAKE_TEXT="set(MODULE_TARGET \"\")
set(MODULE_TEST_TARGET \"\")
set(MODULE_TEST_SUITE \"\")"

CMAKE_LISTS_TEXT="include(Module.cmake)

message(\"*****************BUILDING NEW MODULE*****************\")
message(\"Building module:				 				\${MODULE_TARGET}\")
message(\"Building module test target:	 				\${MODULE_TEST_TARGET}\")
message(\"Test Suite for \${MODULE_TARGET}: 				\${MODULE_TEST_SUITE}\")
message(\"*****************************************************\")

add_library(\${MODULE_TARGET})

target_link_libraries(\${MODULE_TARGET} )

target_include_directories(\${MODULE_TARGET}
        # NEED A VISIBILITY
		include/)
"

mkdir "$BASE_MODULE_DIR"
mkdir "$BASE_MODULE_DIR/include"
mkdir "$BASE_MODULE_DIR/src" 
mkdir "$BASE_MODULE_DIR/test"
touch "$BASE_MODULE_DIR/CMakeLists.txt" "$BASE_MODULE_DIR/Module.cmake"

echo "$MODULE_CMAKE_TEXT" > "$BASE_MODULE_DIR/Module.cmake"
echo "$CMAKE_LISTS_TEXT" > "$BASE_MODULE_DIR/CMakeLists.txt"