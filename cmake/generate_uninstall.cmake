if(EXISTS "build/install_manifest.txt")
    file(READ
        build/install_manifest.txt
        FILES
    )

    configure_file(
        cmake/uninstall.cmake.in
        uninstall.cmake
        @ONLY
    )
endif()
