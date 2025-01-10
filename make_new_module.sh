#!/usr/bin/env bash


mkdir src/NewModule
mkdir src/NewModule/include
mkdir src/NewModule/src
mkdir src/NewModule/test
touch src/NewModule/CMakeLists.txt src/NewModule/Module.cmake 

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

echo "$MODULE_CMAKE_TEXT" > src/NewModule/Module.cmake
echo "$CMAKE_LISTS_TEXT" > src/NewModule/CMakeLists.txt