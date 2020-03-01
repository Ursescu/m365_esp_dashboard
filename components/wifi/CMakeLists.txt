FILE(GLOB SOURCES *.c)
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS "."
                       REQUIRES utils nvs_flash esp_websocket_client)
