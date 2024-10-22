# gd code, general code to manage data
file(GLOB external_gd ${CMAKE_SOURCE_DIR}/external/gd/*.cpp)
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
   ${CMAKE_SOURCE_DIR}/external/gd/gd_variant.cpp
   ${CMAKE_SOURCE_DIR}/external/gd/gd_variant_view.cpp
)

# catch2 code, general code to manage data
file(GLOB external_catch2 ${CMAKE_SOURCE_DIR}/external/catch2/*.cpp)


# imgui code, user interface that works in both linux and windows
file(GLOB external_imgui 
   ${CMAKE_SOURCE_DIR}/external/imgui/*.cpp
)

# source root classes
file(GLOB source_application_root ${CMAKE_SOURCE_DIR}/source/application/root/*.cpp)

