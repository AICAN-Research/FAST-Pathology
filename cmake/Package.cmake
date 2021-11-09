
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
elseif(APPLE)
    install(
        DIRECTORY ${FAST_BINARY_DIR}/../lib/
        DESTINATION lib
        FILES_MATCHING PATTERN "*.dylib*" PATTERN "*plugins.xml"
    )
else()
    install(
        DIRECTORY ${FAST_BINARY_DIR}/../lib/
        DESTINATION lib
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
        DIRECTORY ${FAST_BINARY_DIR}/../pipelines/
        DESTINATION pipelines
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
if(WIN32 AND NOT UNIX)
    # windows
    set(FILE_CONTENT "KernelSourcePath = @ROOT@/kernels/
    DocumentationPath = @ROOT@/doc/
    LibraryPath = @ROOT@/bin/
    QtPluginsPath = @ROOT@/plugins/")
    # move data folder to specific location
    #file(MAKE_DIRECTORY $ENV{HOME}/fastpathology/data/Icons)
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
if (UNIX AND NOT APPLE)
    set(APP_CONFIG_CONTENT "[Desktop Entry]
    Name=FastPathology
    Comment=FastPathology
    Exec=/opt/fastpathology/bin/fastpathology
    Terminal=false
    Type=Application
    Icon=/opt/fastpathology/data/Icons/fastpathology_logo_large.png
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
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "fastpathology is an open-source platform for deep learning-based digital pathology created by SINTEF Medical Technology and the Norwegian University of Science and Technology (NTNU).")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.md) # @TODO somehow concatenate all licences to this file..

set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_FILE_NAME "fastpathology")
set(CPACK_COMPONENT_FAST_REQUIRED ON)

SET(CPACK_PACKAGE_EXECUTABLES "fastpathology" "fastpathology")

# Speed up compression:
set(CPACK_ARCHIVE_THREADS 0)
set(CPACK_THREADS 0)

if(WIN32)

    ## Windows
    # Create windows installer (Requires NSIS from http://nsis.sourceforge.net)
    set(CPACK_GENERATOR NSIS)

    set(CPACK_PACKAGE_INSTALL_DIRECTORY "FastPathology")
    set(CPACK_PACKAGE_FILE_NAME "fastpathology_win10_v${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MENU_LINKS "bin\\\\fastpathology.exe" "FastPathology")
    set(CPACK_CREATE_DESKTOP_LINKS "fastpathology")

    # Icon stuff
    set(CPACK_NSIS_MODIFY_PATH OFF)
    #set(CPACK_NSIS_MUI_ICON ${PROJECT_SOURCE_DIR}/data/Icons/fastpathology_icon_large.ico)  # @TODO: find a way to add icon to installer
    #set(CPACK_NSIS_MUI_UNICON ${PROJECT_SOURCE_DIR}/data/Icons/fastpathology_icon_large.ico)
    #set(CPACK_CREATE_DESKTOP_LINKS ON)
    set(CPACK_NSIS_INSTALLED_ICON_NAME bin\\\\fastpathology.exe)
    set(CPACK_NSIS_INSTALL_DIRECTORY ${CPACK_NSIS_INSTALL_ROOT}/FastPathology) #${CPACK_PACKAGE_INSTALL_DIRECTORY})

elseif(APPLE)
    ## macOS
    set(CPACK_GENERATOR "TXZ")
    set(CPACK_PACKAGE_FILE_NAME "fastpathology_macosx_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

    # # start by defining the Bundle Layout
    # file(MAKE_DIRECTORY ${directory})


    # ## macOS
    # configure_file(${PROJECT_SOURCE_DIR}/README.md ${PROJECT_BINARY_DIR}/README.txt COPYONLY)
    # set(CPACK_RESOURCE_FILE_README ${PROJECT_BINARY_DIR}/README.txt)

    # configure_file(${PROJECT_SOURCE_DIR}/LICENSE.md ${PROJECT_BINARY_DIR}/LICENSE.txt COPYONLY)
    # set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_BINARY_DIR}/LICENSE.txt)

    # set(MACOSX_BUNDLE_ICON_FILE fastpathology_logo_macosx.icns)

    # set_source_files_properties(myAppImage.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    # set(CPACK_GENERATOR "DragNDrop")

    # set(CPACK_MACOSX_BUNDLE "TRUE")
    # set(CPACK_DMG_VOLUME_NAME "fastpathology")
    # set(CPACK_DMG_FORMAT "ODZO")  # zlib compression
    # set(CPACK_PACKAGE_FILE_NAME "fastpathology_macosx_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
    # set(CPACK_PACKAGE_NAME "fastpathology_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

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

    # add libopenslide dependency
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libopenslide0")

    # Select components to avoid some cmake leftovers from built dependencies
    set(CPACK_DEB_COMPONENT_INSTALL OFF)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/fastpathology")  #"/opt/")  $HOME/fastpathology
    set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
    set(CPACK_DEBIAN_FILE_NAME "fastpathology_ubuntu_v${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}.deb")
    set(CPACK_DEBIAN_fastpathology_PACKAGE_NAME "fastpathology")

endif()
include(CPack)