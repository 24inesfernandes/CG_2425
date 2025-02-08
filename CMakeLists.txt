cmake_minimum_required(VERSION 3.30)
project(CG_2425)

set(CMAKE_CXX_STANDARD 20)

# Adiciona o diretório do "generator"
add_subdirectory(generator)

# Executável principal
add_executable(CG_2425 main.cpp)

# Linka o executável principal com os alvos necessários (se houver dependências no futuro)
target_link_libraries(CG_2425 generator)
