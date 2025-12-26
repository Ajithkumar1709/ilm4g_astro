# Copyright (C) 2020 QUECTEL Technologies Limited and/or its affiliates("QUECTEL").
# All rights reserved.
#

set(target ql_app_mqtt)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC)
target_include_directories(${target} PUBLIC inc)
#target_link_libraries(${target} PRIVATE kernel driver ql_api_common)
target_include_directories(${target} PUBLIC inc fsDriver)
target_include_directories(${target} PUBLIC inc modules)
target_include_directories(${target} PUBLIC inc otaProcess)
target_include_directories(${target} PUBLIC inc  protobuff)
target_include_directories(${target} PUBLIC inc  task)

target_sources(${target} PRIVATE
	mqtt_demo.c
	modules/acParam.c
	modules/coControllerProcess.c 
	modules/faultChecker.c 
	modules/gnss.c 
	modules/memoryHandler.c 
	modules/powerFailHandler.c 
	modules/publishData.c 
	modules/rpcHandler.c 
	modules/rtc.c 
	modules/astronomy.c 
	tasks/autoUploadProcess.c
	tasks/ilmProcess.c
	tasks/mqttProcess.c
	tasks/onBootProcess.c
	tasks/periodicProcess.c
	protobuff/getMsg.pb.c
	protobuff/pb_common.c
	protobuff/pb_decode.c
	protobuff/pb_encode.c
	protobuff/rpcNew.pb.c
	protobuff/telemetry.pb.c
	fsDriver/fsDriver.c
	otaProcess/fota_ftp.c
	
)

relative_glob(srcs include/*.h src/*.c inc/*.h)
beautify_c_code(${target} ${srcs})
