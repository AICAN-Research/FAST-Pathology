
# Install pathology application
install(
    TARGETS fastpathology
    DESTINATION bin
)

# License file
install(
    FILES LICENSE.md
    DESTINATION licenses/fastpathology
)

# Install FAST dependency (need to include plugins.xml for OpenVINO as well)
if(WIN32)
    install(
        DIRECTORY ${FAST_BINARY_DIR}
        DESTINATION bin
        FILES_MATCHING PATTERN "*.dll" PATTERN "*plugins.xml"
    )
else()
    #[[
    install(
        DIRECTORY ${FAST_BINARY_DIR}
        DESTINATION lib
        FILES_MATCHING PATTERN "*.so*" PATTERN "*plugins.xml"
    )
    ]]

    install(
        DIRECTORY ${FAST_BINARY_DIR}/../lib/
        DESTINATION lib  # lib or bin?
        FILES_MATCHING PATTERN "*.so*" PATTERN "*plugins.xml"
    )
endif()
install(
    DIRECTORY ${FAST_BINARY_DIR}/../kernels/
    DESTINATION kernels
)
install(
    DIRECTORY ${FAST_BINARY_DIR}/../plugins/
    DESTINATION plugins
)
install(
    DIRECTORY ${FAST_BINARY_DIR}/../doc/
    DESTINATION doc
)
install(
    DIRECTORY ${FAST_BINARY_DIR}/../licenses/
    DESTINATION licenses
)
install(
    DIRECTORY
    DESTINATION kernel_binaries
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE GROUP_READ GROUP_EXECUTE GROUP_WRITE WORLD_READ WORLD_WRITE WORLD_EXECUTE
)

# add Data folder for storing saved models, icons, pipelines and other stuff, and move necessary folders
install(
    DIRECTORY ${PROJECT_BINARY_DIR}/../data/Icons
    DESTINATION data
)


# Setup fast_configuration.txt file
if(WIN32)
# windows
set(FILE_CONTENT "KernelSourcePath = @ROOT@/kernels/
DocumentationPath = @ROOT@/doc/
LibraryPath = @ROOT@/bin/
QtPluginsPath = @ROOT@/plugins/")
else()
# UNIX
set(FILE_CONTENT "KernelSourcePath = @ROOT@/kernels/
DocumentationPath = @ROOT@/doc/
LibraryPath = @ROOT@/lib/
QtPluginsPath = @ROOT@/plugins/")
endif()

# Write file
file(WRITE ${PROJECT_BINARY_DIR}/fast_configuration_install.txt ${FILE_CONTENT})

# Install file
install(
    FILES ${PROJECT_BINARY_DIR}/fast_configuration_install.txt
    DESTINATION bin
    RENAME fast_configuration.txt
)


# setup .desktop file
if (UNIX)
set(APP_CONFIG_CONTENT "[Desktop Entry]
Name=FastPathology
Comment=FastPathology
Exec=/opt/fastpathology/bin/fastpathology
Terminal=false
Type=Application
Icon=/opt/fastpathology/data/Icons/fastpathology_logo.png
Categories=public.app-categorical.medical")

# write
file(WRITE ${PROJECT_BINARY_DIR}/fastpathology.desktop ${APP_CONFIG_CONTENT})

# install
install(
    FILES ${PROJECT_BINARY_DIR}/fastpathology.desktop
    DESTINATION /usr/share/applications/
    PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE GROUP_READ GROUP_EXECUTE GROUP_WRITE WORLD_READ WORLD_WRITE WORLD_EXECUTE
)
endif()


set(CPACK_PACKAGE_NAME "fastpathology")
set(CPACK_PACKAGE_CONTACT "Andre Pedersen andre.pedersen@sintef.no")
set(CPACK_PACKAGE_VENDOR "SINTEF and NTNU")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "fastpathology is an open-source platform for artificial intelligence-based digital pathology created by SINTEF Medical Technology and the Norwegian University of Science and Technology (NTNU).")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.md) # @TODO somehow concatenate all licences to this file..
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_FILE_NAME "fastpathology")
set(CPACK_COMPONENT_FAST_REQUIRED ON)
#set(CPACK_PACKAGE_EXECUTABLES fastpathology "fastpathology")

if(WIN32 AND NOT UNIX)

    ## Windows
    # Create windows installer (Requires NSIS from http://nsis.sourceforge.net)
    set(CPACK_GENERATOR NSIS)

    set(CPACK_PACKAGE_INSTALL_DIRECTORY "fastpathology")
    include(CPack)
else()
    ## UNIX

    # attempt to fix .so-dependency of FAST by copying them to fastpathology
    #[[
    install(
            DIRECTORY ${FAST_BINARY_DIR}/../lib/
            DESTINATION bin
            FILES_MATCHING PATTERN "*.so*" PATTERN "*plugins.xml"
    )
    ]]

    # Get distro name and version
    find_program(LSB_RELEASE_EXEC lsb_release)
    execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
            OUTPUT_VARIABLE DISTRO_NAME
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(TOLOWER ${DISTRO_NAME} DISTRO_NAME)
    execute_process(COMMAND ${LSB_RELEASE_EXEC} -rs
            OUTPUT_VARIABLE DISTRO_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Create debian package
    set(CPACK_GENERATOR "DEB")

    # add libopenslide dependency here! Something like this (see WSI module in FAST, GitHub):
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libopenslide0")

    # Select components to avoid some cmake leftovers from built dependencies
    set(CPACK_DEB_COMPONENT_INSTALL OFF)
    #set(CPACK_PACKAGING_INSTALL_PREFIX "$HOME/fastpathology") # @FIXME: files were created directly in /opt and not in the correct directory if I didnt do this
    set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/fastpathology")  #"/opt/")
    set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

    #set(CPACK_PACKAGE_INSTALL_DIRECTORY "fastpathology")
    #set(CPACK_COMPONENTS_ALL fastpathology)

    set(CPACK_DEBIAN_FILE_NAME "fastpathology_${DISTRO_NAME}${DISTRO_VERSION}_v${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}.deb")
    #set(CPACK_COMPONENTS_ALL fastpathology)
    #set(CPACK_DEBIAN_FAST_FILE_NAME "fastpathology_0.1.0.deb")
    set(CPACK_DEBIAN_fastpathology_PACKAGE_NAME "fastpathology")

    include(CPack)
endif()