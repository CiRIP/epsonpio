# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2026 Ciprian Ionescu <me@ciprian-ionescu.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(epsonpio C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(epsonpio epsonpio.c usb_descriptors.c )

pico_set_program_name(epsonpio "epsonpio")
pico_set_program_version(epsonpio "0.1")

# Generate PIO header
pico_generate_pio_header(epsonpio ${CMAKE_CURRENT_LIST_DIR}/clk_div.pio)
pico_generate_pio_header(epsonpio ${CMAKE_CURRENT_LIST_DIR}/dsio_tx.pio)
pico_generate_pio_header(epsonpio ${CMAKE_CURRENT_LIST_DIR}/dsio_rx.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(epsonpio 0)
pico_enable_stdio_usb(epsonpio 0)

# Add the standard library to the build
target_link_libraries(epsonpio
        pico_stdlib
        hardware_pio
        tinyusb_device
        tinyusb_board
        )

# Add the standard include files to the build
target_include_directories(epsonpio PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(epsonpio 
        hardware_pio
        )

pico_add_extra_outputs(epsonpio)

