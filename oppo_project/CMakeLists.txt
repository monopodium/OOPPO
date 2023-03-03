cmake_minimum_required(VERSION 3.15)
project(oppo_project)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 -DASIO_STANDALONE")

find_package(Threads REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/grpc")
set(spdlog_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
message(STATUS "gRPC_DIR = ${gRPC_DIR}")
message(STATUS "gRPC_INCLUDE_DIRS = ${gRPC_INCLUDE_DIRS}")
message(STATUS "gRPC_LIBS = ${gRPC_LIBS}")

set(_GRPC_GRPCPP gRPC::grpc++)
if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

# Proto file
get_filename_component(cn_proto "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/coordinator.proto" ABSOLUTE)
get_filename_component(dn_proto "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/proxy.proto" ABSOLUTE)
get_filename_component(cn_proto_path "${cn_proto}" PATH)
message("${cn_proto}")
message("${cn_proto_path}")

set(cn_proto_srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/coordinator.pb.cc")
set(cn_proto_hdrs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/coordinator.pb.h")
set(cn_grpc_srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/coordinator.grpc.pb.cc")
set(cn_grpc_hdrs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/coordinator.grpc.pb.h")
set(dn_proto_srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/proxy.pb.cc")
set(dn_proto_hdrs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/proxy.pb.h")
set(dn_grpc_srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/proxy.grpc.pb.cc")
set(dn_grpc_hdrs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/proxy.grpc.pb.h")

add_custom_command(
        OUTPUT ${cn_proto_srcs} ${cn_proto_hdrs} ${cn_grpc_srcs} ${cn_grpc_hdrs} ${dn_proto_srcs} ${dn_proto_hdrs} ${dn_grpc_srcs} ${dn_grpc_hdrs}
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto"
        --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto"
        -I "${CMAKE_CURRENT_SOURCE_DIR}/src/proto"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${cn_proto}" "${dn_proto}"
        DEPENDS "${cn_proto}" "${dn_proto}")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/proto")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/libmemcached/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/gf-complete/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/jerasure/include/jerasure)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/jerasure/include)

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party/libmemcached/lib")
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party/jerasure/lib")
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party/gf-complete/lib")
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(Boost_USE_STATIC_LIBS ON)
#find_package(Boost COMPONENTS thread REQUIRED)

# set(testsrc ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp)
# add_executable(unittest ${testsrc})
# target_link_libraries(unittest libJerasure.a libgf_complete.a)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/   PROJECT_SRC)

# Targets (client|server)
foreach (_target
        run_client run_coordinator run_proxy run_datanode test_tools)
    add_executable(${_target}
            ${cn_proto_srcs}
            ${cn_grpc_srcs}
            ${dn_proto_srcs}
            ${dn_grpc_srcs}
            
            ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/coordinator.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/proxy.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/run_cpp/${_target}.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/ToolBox.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/tinyxml2.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/datanode.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/azure_lrc.cpp
            #${_target}.cpp
            )
    target_link_libraries(${_target}
            ${_REFLECTION}
            ${_GRPC_GRPCPP}
	    ${_GRPC_GRPCPP_UNSECURE}
            ${_PROTOBUF_LIBPROTOBUF}
            -lmemcached
            -lpthread
            -lJerasure -lgf_complete
            )
endforeach ()