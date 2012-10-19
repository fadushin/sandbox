# 
# Copyright (c) dushin.net
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of dushin.net nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY dushin.net ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL dushin.net BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
cmake_minimum_required(VERSION 2.8)

project(rabbitmq)

find_package(Boost COMPONENTS system program_options chrono REQUIRED) 



set(
    INCLUDE_DIRS
    "/opt/boost/include"
    "/opt/SimpleAmqpClient/include"
)

include_directories(${INCLUDE_DIRS})

set(
    HEADER_FILES 
)
set(
    SENDER_SOURCE_FILES 
    sender.cpp
)
set(
    RECEIVER_SOURCE_FILES 
    receiver.cpp
)

find_library(SimpleAmqpClient_LIBRARY
    NAMES SimpleAmqpClient
    HINTS "/opt/SimpleAmqpClient/lib"
)
message(STATUS "SimpleAmqpClient_LIBRARY: ${SimpleAmqpClient_LIBRARY}")
find_library(rabbitmqc_LIBRARY
    NAMES rabbitmq
    HINTS "/opt/rabbitmq-c/lib"
)
message(STATUS "rabbitmqc_LIBRARY: ${rabbitmqc_LIBRARY}")

add_executable(sender ${SENDER_SOURCE_FILES})
target_link_libraries(
    sender
    ${SimpleAmqpClient_LIBRARY} ${rabbitmqc_LIBRARY}
    ${Boost_SYSTEM_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

add_executable(receiver ${RECEIVER_SOURCE_FILES})
target_link_libraries(
    receiver
    ${SimpleAmqpClient_LIBRARY} ${rabbitmqc_LIBRARY}
    ${Boost_SYSTEM_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
