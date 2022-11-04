# Install pathology application
if(APPLE)
	install(
		TARGETS fastpathology
		DESTINATION ../MacOS
	)
else()
	install(
		TARGETS fastpathology
		DESTINATION bin
	)
endif()

# Create dummy bin/ folder relevant for MacOS
if(APPLE)
	install(
		DIRECTORY
		DESTINATION bin
	)
	install(
		DIRECTORY
		DESTINATION ../MacOS/bin
	)
endif()

# License file
if(APPLE)
	install(
		FILES LICENSE.md
		DESTINATION licenses/fastpathology
	)
else()
	install(
		FILES LICENSE.md
		DESTINATION licenses/fastpathology
	)
endif()

# Install FAST dependency
if(WIN32)
	install(DIRECTORY ${FAST_BINARY_DIR}/
			DESTINATION bin
			FILES_MATCHING PATTERN "*.dll")
elseif(APPLE)
	install(DIRECTORY ${FAST_BINARY_DIR}/../lib/
			DESTINATION ../MacOS/lib
			FILES_MATCHING PATTERN "*.dylib*")
	install(DIRECTORY ${FAST_BINARY_DIR}/../lib/
			DESTINATION ../MacOS/lib
			FILES_MATCHING PATTERN "*.so*")
else()
	install(DIRECTORY ${FAST_BINARY_DIR}/../lib/
			DESTINATION lib
			FILES_MATCHING PATTERN "*.so*")
endif()

# Additional OpenVINO files
if(WIN32)
	install(FILES ${FAST_BINARY_DIR}/plugins.xml ${FAST_BINARY_DIR}/cache.json
			DESTINATION bin
			)
elseif(APPLE)
	install(FILES ${FAST_BINARY_DIR}/../lib/plugins.xml ${FAST_BINARY_DIR}/../lib/cache.json
			DESTINATION ../MacOS/lib
			OPTIONAL
			)
else()
	install(FILES ${FAST_BINARY_DIR}/../lib/plugins.xml ${FAST_BINARY_DIR}/../lib/cache.json
			DESTINATION lib
			OPTIONAL
			)
endif()

if(APPLE)
	install(
		DIRECTORY ${FAST_BINARY_DIR}/../kernels/
		DESTINATION ../MacOS/kernels
	)
	install(
		DIRECTORY ${FAST_BINARY_DIR}/../plugins/
		DESTINATION ../MacOS/plugins
	)
	install(
		FILES ${FAST_BINARY_DIR}/../doc/images/fast_icon.ico ${FAST_BINARY_DIR}/../doc/images/fast_icon.png
		DESTINATION ../MacOS/doc/images
	)
	install(
		DIRECTORY ${FAST_BINARY_DIR}/../doc/fonts/
		DESTINATION ../MacOS/doc/fonts
	)
	install(
		DIRECTORY ${FAST_BINARY_DIR}/../licenses/
		DESTINATION ../MacOS/licenses
	)

	# add Data folder for storing saved models, icons, pipelines and other stuff, and move necessary folders
	install(
		DIRECTORY ${PROJECT_BINARY_DIR}/../data/Icons
		DESTINATION ../MacOS/data
	)
	install(
		DIRECTORY ${PROJECT_BINARY_DIR}/../data/pipelines
		DESTINATION ../MacOS/data
	)
else()
	install(
		DIRECTORY ${FAST_BINARY_DIR}/../kernels/
		DESTINATION kernels
	)
	install(
		DIRECTORY ${FAST_BINARY_DIR}/../plugins/
		DESTINATION plugins
	)
	install(
		FILES ${FAST_BINARY_DIR}/../doc/images/fast_icon.ico ${FAST_BINARY_DIR}/../doc/images/fast_icon.png
		DESTINATION doc/images
	)
	install(
		DIRECTORY ${FAST_BINARY_DIR}/../doc/fonts/
		DESTINATION doc/fonts
	)
	install(
		DIRECTORY ${FAST_BINARY_DIR}/../licenses/
		DESTINATION licenses
	)

	# add Data folder for storing saved models, icons, pipelines and other stuff, and move necessary folders
	install(
		DIRECTORY ${PROJECT_BINARY_DIR}/../data/Icons
		DESTINATION data
	)
	install(
		DIRECTORY ${PROJECT_BINARY_DIR}/../data/pipelines
		DESTINATION data
	)
endif()

# Setup fast_configuration.txt file
if(WIN32)
	# windows
	set(FILE_CONTENT "KernelSourcePath = @ROOT@/kernels/
	DocumentationPath = @ROOT@/doc/
	LibraryPath = @ROOT@/bin/
	QtPluginsPath = @ROOT@/plugins/")

	# move data folder to specific location
	#file(MAKE_DIRECTORY $ENV{HOME}/fastpathology/data/Icons)

elseif(APPLE)
	# macOS
	set(FILE_CONTENT "KernelSourcePath = @ROOT@/kernels/
	DocumentationPath = @ROOT@/doc/
	LibraryPath = @ROOT@/lib/
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
if(APPLE)
	install(
		FILES ${PROJECT_BINARY_DIR}/fast_configuration_install.txt
		DESTINATION ../MacOS/bin
		RENAME fast_configuration.txt
	)
else()
	install(
		FILES ${PROJECT_BINARY_DIR}/fast_configuration_install.txt
		DESTINATION bin
		RENAME fast_configuration.txt
	)
endif()

if(WIN32)
elseif(APPLE)
else()
	# setup .desktop file
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
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FastPathology is an open-source platform for deep learning-based digital pathology created by SINTEF Medical Technology and the Norwegian University of Science and Technology (NTNU).")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.md) # @TODO somehow concatenate all licences to this file..

set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME "fastpathology")
set(CPACK_COMPONENT_FAST_REQUIRED ON)

SET(CPACK_PACKAGE_EXECUTABLES "fastpathology" "fastpathology")

if(WIN32)
    ## Windows
    # Create windows installer (Requires NSIS from http://nsis.sourceforge.net)
    set(CPACK_GENERATOR NSIS)

	set(CPACK_PACKAGE_INSTALL_DIRECTORY "FastPathology")
	set(CPACK_PACKAGE_FILE_NAME "fastpathology_windows_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
	set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
	set(CPACK_NSIS_MENU_LINKS "bin\\\\fastpathology.exe" "FastPathology")
	set(CPACK_CREATE_DESKTOP_LINKS "fastpathology")

	# Icon stuff
	#set(CPACK_NSIS_MUI_ICON ${PROJECT_SOURCE_DIR}/data/Icons/fastpathology_icon_large.ico)  # @TODO: find a way to add icon to installer
	#set(CPACK_NSIS_MUI_UNICON ${PROJECT_SOURCE_DIR}/data/Icons/fastpathology_icon_large.ico)
	#set(CPACK_CREATE_DESKTOP_LINKS ON)
	set(CPACK_NSIS_INSTALLED_ICON_NAME bin\\\\fastpathology.exe)
	set(CPACK_NSIS_INSTALL_DIRECTORY ${CPACK_NSIS_INSTALL_ROOT}/FastPathology) #${CPACK_PACKAGE_INSTALL_DIRECTORY})

elseif(APPLE)
	## macOS
	# Create APP Bundle
	set(CPACK_GENERATOR "Bundle")
	#"fastpathology_macos${CMAKE_OSX_DEPLOYMENT_TARGET}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
	set(CPACK_BUNDLE_NAME "FastPathology")
	set(CPACK_BUNDLE_ICON "${PROJECT_SOURCE_DIR}/data/Icons/fastpathology_logo_large.icns")
	set(CPACK_BUNDLE_PLIST "${PROJECT_SOURCE_DIR}/misc/Info.plist")

else()
    ## Linux
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
	set(CPACK_PACKAGE_FILE_NAME "fastpathology_${DISTRO_NAME}${DISTRO_VERSION}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
	set(CPACK_DEBIAN_FAST_FILE_NAME "fastpathology_${DISTRO_NAME}${DISTRO_VERSION}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.deb")
	set(CPACK_DEBIAN_fastpathology_PACKAGE_NAME "fastpathology")

endif()
include(CPack)
