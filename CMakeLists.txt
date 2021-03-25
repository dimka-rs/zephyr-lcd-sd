cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lcd-sd)

target_sources(app PRIVATE
  src/main.c
  src/lcd.c
)

target_include_directories(app PRIVATE inc)

zephyr_library_include_directories(.)
