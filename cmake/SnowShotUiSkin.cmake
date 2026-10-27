# Fork-only UI skin selection (see fork/macshot-alignment.md).
# "snow" keeps upstream behavior; "macshot" switches the capture toolbar to a
# macshot-style look and placement. Every source hook is marked FORK(macshot).
set(SNOW_SHOT_UI_SKIN "snow" CACHE STRING "Snow Shot UI skin: snow or macshot")
set_property(CACHE SNOW_SHOT_UI_SKIN PROPERTY STRINGS snow macshot)

if(SNOW_SHOT_UI_SKIN STREQUAL "macshot")
    add_compile_definitions(SNOW_SHOT_UI_SKIN_MACSHOT=1)
elseif(NOT SNOW_SHOT_UI_SKIN STREQUAL "snow")
    message(FATAL_ERROR "SNOW_SHOT_UI_SKIN must be snow or macshot, got '${SNOW_SHOT_UI_SKIN}'")
endif()
message(STATUS "Snow Shot UI skin: ${SNOW_SHOT_UI_SKIN}")

# Skin unit tests do not depend on the active skin, so they always build.
# Deferred to the end of snow_shot/CMakeLists.txt so Qt and enable_testing() exist.
function(snow_shot_register_ui_skin_tests)
    if(NOT SNOW_SHOT_BUILD_TESTS)
        return()
    endif()
    add_executable(snow-shot-ui-skin-tests tests/ui_skin_placement_tests.cpp)
    target_include_directories(snow-shot-ui-skin-tests PRIVATE include)
    target_link_libraries(snow-shot-ui-skin-tests PRIVATE Qt6::Core)
    add_test(NAME snow-shot-ui-skin-tests COMMAND snow-shot-ui-skin-tests)
    set_tests_properties(snow-shot-ui-skin-tests PROPERTIES LABELS "unit" TIMEOUT 15)
endfunction()
cmake_language(DEFER CALL snow_shot_register_ui_skin_tests)
