# gd code, general code to manage data
file(GLOB_RECURSE external_gd ${CMAKE_SOURCE_DIR}/external/gd/*.cpp)
file(GLOB external_gd_core 
   ${CMAKE_SOURCE_DIR}/external/gd/gd_arguments.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_arguments_shared.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_file.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_parse.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_sql_value.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_table.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_table_column.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_table_column-buffer.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_table_index.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_table_table.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_table_io.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_types.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_utf8.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_utf8_2.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_strings.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_variant.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_variant_view.cpp
)

# Define the base path for gd library
set(GD_BASE_PATH ${CMAKE_SOURCE_DIR}/external/gd)

# Collect all .cpp and .h files recursively and by component
file(GLOB_RECURSE GD_SOURCES_ALL ${GD_BASE_PATH}/*.cpp ${GD_BASE_PATH}/*.h)
file(GLOB GD_SOURCES_ROOT ${GD_BASE_PATH}/*.cpp ${GD_BASE_PATH}/*.h)
file(GLOB GD_SOURCES_COM ${GD_BASE_PATH}/com/*.cpp ${GD_BASE_PATH}/com/*.h)
file(GLOB GD_SOURCES_CONSOLE ${GD_BASE_PATH}/console/*.cpp ${GD_BASE_PATH}/console/*.h)
file(GLOB GD_SOURCES_MATH ${GD_BASE_PATH}/math/*.cpp ${GD_BASE_PATH}/math/*.h)
file(GLOB GD_SOURCES_DATABASE ${GD_BASE_PATH}/database/*.cpp ${GD_BASE_PATH}/database/*.h)
file(GLOB GD_SOURCES_EXPRESSION ${GD_BASE_PATH}/expression/*.cpp ${GD_BASE_PATH}/expression/*.h)
file(GLOB GD_SOURCES_IO ${GD_BASE_PATH}/io/*.cpp ${GD_BASE_PATH}/io/*.h)
file(GLOB GD_SOURCES_PARSE ${GD_BASE_PATH}/parse/*.cpp ${GD_BASE_PATH}/parse/*.h)


# catch2 code, general code to manage data
file(GLOB external_catch2 ${CMAKE_SOURCE_DIR}/external/catch2/*.cpp)


# imgui code, user interface that works in both linux and windows
file(GLOB external_imgui 
   ${CMAKE_SOURCE_DIR}/external/imgui/*.cpp
)

# source root classes
file(GLOB source_application_root ${CMAKE_SOURCE_DIR}/source/application/root/*.cpp)

# file(GLOB external_fmt ${CMAKE_SOURCE_DIR}/external/fmt/*.cc)
# file(GLOB external_gd ${CMAKE_SOURCE_DIR}/external/gd/*.cpp)
# file(GLOB external_lua ${CMAKE_SOURCE_DIR}/external/lua/*.c)
file(GLOB external_sqlite ${CMAKE_SOURCE_DIR}/external/sqlite/*.c)
file(GLOB external_pugixml ${CMAKE_SOURCE_DIR}/external/pugixml/*.cpp)

# file(GLOB external_miniz ${CMAKE_SOURCE_DIR}/external/miniz/*.c)

