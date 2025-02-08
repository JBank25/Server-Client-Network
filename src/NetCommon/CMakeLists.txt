include(Module.cmake)

message("*****************BUILDING NEW MODULE*****************")
message("Building module:				 				${MODULE_TARGET}")
message("Building module test target:	 				${MODULE_TEST_TARGET}")
message("Test Suite for ${MODULE_TARGET}: 				${MODULE_TEST_SUITE}")
message("*****************************************************")

add_library(${MODULE_TARGET} INTERFACE)

target_link_libraries(${MODULE_TARGET}
						INTERFACE
						NetMessage)

target_include_directories(${MODULE_TARGET}
		INTERFACE 
		include/)


if(ENABLE_UNIT_TESTING)
	add_subdirectory(test/)
endif()