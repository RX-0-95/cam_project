
add_executable(person_detection)
target_include_directories(person_detection
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/.
)


target_link_libraries(person_detection 
    pico-tflmicro
    arducam 
    pico_stdlib
    free_rtos
    )

target_sources(person_detection
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/main.cpp
  ${CMAKE_CURRENT_LIST_DIR}/image_provider.h
  ${CMAKE_CURRENT_LIST_DIR}/image_provider.cpp
  #${CMAKE_CURRENT_LIST_DIR}/main_function.h
  #${CMAKE_CURRENT_LIST_DIR}/main_function.cpp
  ${CMAKE_CURRENT_LIST_DIR}/model_settings.h
  ${CMAKE_CURRENT_LIST_DIR}/model_settings.cpp
  ${CMAKE_CURRENT_LIST_DIR}/detection_responder.h
  ${CMAKE_CURRENT_LIST_DIR}/detection_responder.cpp
  ${CMAKE_CURRENT_LIST_DIR}/person_detection_model_data.h
  ${CMAKE_CURRENT_LIST_DIR}/tensorflow_models/person_detection_model_data.cpp
  ${CMAKE_CURRENT_LIST_DIR}/detect_config.h
  ${CMAKE_CURRENT_LIST_DIR}/motion_detection.h
  ${CMAKE_CURRENT_LIST_DIR}/motion_detection.cpp
  ${CMAKE_CURRENT_LIST_DIR}/tiny_cv.h
  ${CMAKE_CURRENT_LIST_DIR}/tiny_cv.cpp
  ${CMAKE_CURRENT_LIST_DIR}/task_person_detection.h
  ${CMAKE_CURRENT_LIST_DIR}/task_person_detection.cpp
  ${CMAKE_CURRENT_LIST_DIR}/task_pretask.h
  ${CMAKE_CURRENT_LIST_DIR}/task_pretask.cpp
  ${CMAKE_CURRENT_LIST_DIR}/task_capture_transfer.h
  ${CMAKE_CURRENT_LIST_DIR}/task_capture_transfer.cpp
)

pico_add_extra_outputs(person_detection)

pico_enable_stdio_usb(person_detection 0)
pico_enable_stdio_uart(person_detection 1)