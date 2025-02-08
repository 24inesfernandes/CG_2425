cmake_minimum_required(VERSION 3.10)
project(generator)

set(CMAKE_CXX_STANDARD 11)

# Cria uma biblioteca para ser usada no projeto principal
add_library(generator generator.cpp model.cpp)
