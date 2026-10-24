# Is include in the main cmakelists on install

get_filename_component(target_directory "${target_file}" DIRECTORY)

message(STATUS "Deploying Qt...")

get_filename_component(qt_bin_dir "${QMAKE_FILEPATH}" DIRECTORY)

if (APPLE)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${qt_bin_dir}" REQUIRED)

    # Copies the Qt frameworks and plugins into the bundle; the bundle is ad-hoc signed afterwards by the build script
    execute_process(
            COMMAND "${MACDEPLOYQT_EXECUTABLE}" "${target_bundle}" -verbose=1
            COMMAND_ERROR_IS_FATAL ANY
    )
else ()
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${qt_bin_dir}")

    execute_process(
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            "${target_file}"
            --compiler-runtime
    )
endif ()

message(STATUS "Qt deployed.")
