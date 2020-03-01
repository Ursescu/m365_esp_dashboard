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
        preferences
        wifi
        fs
)

# Create a SPIFFS image from the contents of the 'spiffs_image' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash'. 
spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
