include_directories (${CMAKE_SOURCE_DIR}/include)

file(
        GLOB
        usage_examples
        *.cpp
        )

foreach(f ${usage_examples})
        get_filename_component(exampleName ${f} NAME_WE)
        add_executable (${exampleName} ${f})
        target_link_libraries(${exampleName} mapping)
        install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${exampleName}
                DESTINATION bin
                RENAME ${CMAKE_PROJECT_NAME}-${exampleName})
endforeach(f)
