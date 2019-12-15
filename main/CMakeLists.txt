set(SOURCES main.c)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_LIST_DIR}
        $ENV{IDF_PATH}/components
    REQUIRES
        tft
        adc
        buzzer
        comm
        display
        proto
        utils
)