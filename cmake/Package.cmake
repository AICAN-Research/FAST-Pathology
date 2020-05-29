
# Install pathology application
install(
    TARGETS fastPathology
    DESTINATION bin
)
# License file
install(
    FILES LICENSE.md
    DESTINATION licenses/fastPathology
)

# Install FAST dependency (need to include plugins.xml for OpenVINO as well)
if(WIN32)
    install(
        DIRECTORY ${FAST_BINARY_DIR}
        DESTINATION bin
        FILES_MATCHING PATTERN "*.dll" PATTERN "*plugins.xml"
    )
else()
    install(
        DIRECTORY ${FAST_BINARY_DIR}
        DESTINATION bin
        FILES_MATCHING PATTERN "*.so" PATTERN "*plugins.xml"
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
set(FILE_CONTENT "TestDataPath = @ROOT@/data/
KernelSourcePath = @ROOT@/kernels/
KernelBinaryPath = @ROOT@/kernel_binaries/
DocumentationPath = @ROOT@/doc/
PipelinePath = @ROOT@/pipelines/
LibraryPath = @ROOT@/bin/
QtPluginsPath = @ROOT@/plugins/")

# Write file
file(WRITE ${PROJECT_BINARY_DIR}/fast_configuration_install.txt ${FILE_CONTENT})

# Install file
install(
    FILES ${PROJECT_BINARY_DIR}/fast_configuration_install.txt
    DESTINATION bin
    RENAME fast_configuration.txt
)

set(CPACK_PACKAGE_NAME "FAST-Pathology")
set(CPACK_PACKAGE_CONTACT "Andre Pedersen andre.pedersen@sintef.no")
set(CPACK_PACKAGE_VENDOR "SINTEF and NTNU")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FastPathology is an open-source platform for artificial intelligence-based digital pathology created by SINTEF Medical Technology and the Norwegian University of Science and Technology (NTNU).")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.md) # @TODO somehow concatenate all licences to this file..
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_FILE_NAME "FastPathology")
set(CPACK_COMPONENT_FAST_REQUIRED ON)
#set(CPACK_PACKAGE_EXECUTABLES fastPathology "FastPathology")

if(WIN32 AND NOT UNIX)

    ## Windows
    # Create windows installer (Requires NSIS from http://nsis.sourceforge.net)
    set(CPACK_GENERATOR NSIS)

    set(CPACK_PACKAGE_INSTALL_DIRECTORY "FastPathology")
else()
    ## UNIX

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

    # Select components to avoid some cmake leftovers from built dependencies
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/FastPathology") # @FIXME: files were created directly in /opt and not in the correct directory if I didnt do this
    set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

    #set(CPACK_COMPONENTS_ALL FastPathology)

    set(CPACK_DEBIAN_FAST_FILE_NAME "fast_${DISTRO_NAME}${DISTRO_VERSION}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.deb")
    #set(CPACK_COMPONENTS_ALL FastPathology)
    #set(CPACK_DEBIAN_FAST_FILE_NAME "FastPathology_0.1.0.deb")
    set(CPACK_DEBIAN_FAST_PACKAGE_NAME "FastPathology")
    include(CPack)
endif()