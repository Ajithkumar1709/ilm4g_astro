# Copyright (C) 2020 QUECTEL Technologies Limited and/or its affiliates("QUECTEL").
# All rights reserved.
#

set(target ql_app_wifi)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC)
target_include_directories(${target} PUBLIC inc)

include(./inc/wifi_demo_macro.cmake)

configure_file (./inc/wifi_demo_macro.h.in ${out_inc_dir}/wifi_demo_macro.h)

set(SRC_FILES_PATH)
#######################################
if(QL_WIFI_FC41D)
target_include_directories(${target} PUBLIC ./fc41d/inc)
list(APPEND SRC_FILES_PATH "./fc41d/src/*.c;./fc41d/src/*.C" )
endif()
########################################
file(GLOB SRC_FILES ${SRC_FILES_PATH})
message("${SRC_FILES}")

target_sources(${target} PRIVATE
	wifi_demo.c
	${SRC_FILES}
)

relative_glob(srcs include/*.h src/*.c inc/*.h)
beautify_c_code(${target} ${srcs})
