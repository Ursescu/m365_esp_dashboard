idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS
        ${CMAKE_CURRENT_LIST_DIR}
        $ENV{IDF_PATH}/components
    REQUIRES
        tft
)