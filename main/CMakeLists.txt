file(GLOB_RECURSE IMAGE_SOURCES images/*.c)

idf_component_register(SRCS "main.cpp" "lvgl_demo_ui.c" ${IMAGE_SOURCES}
                       INCLUDE_DIRS "."
                       REQUIRES esp_timer driver lvgl uControllerI80)
