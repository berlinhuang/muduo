set(base_SRCS
  AsyncLogging.cc
  Condition.cc
  CountDownLatch.cc
  Date.cc
  Exception.cc
  FileUtil.cc
  LogFile.cc
  Logging.cc
  LogStream.cc
  ProcessInfo.cc
  Timestamp.cc
  TimeZone.cc
  Thread.cc
  ThreadPool.cc
  )

add_library(muduo_base ${base_SRCS})#添加一个名为muduo_base的库文件，该库文件将会根据调用的命令里列出的源文件${base_SRCS}来创建
target_link_libraries(muduo_base pthread rt)

add_library(muduo_base_cpp11 ${base_SRCS})
target_link_libraries(muduo_base_cpp11 pthread rt)
set_target_properties(muduo_base_cpp11 PROPERTIES COMPILE_FLAGS "-std=c++0x")

install(TARGETS muduo_base DESTINATION lib)#有5中可以被安装的目标文件：ARCHIVE，LIBRARY，RUNTIME，FRAMEWORK和BUNDLE
install(TARGETS muduo_base_cpp11 DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/muduo/base)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
  add_subdirectory(tests)
endif()
