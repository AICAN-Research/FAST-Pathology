
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

# Install FAST dependency
if(WIN32)
    install(
        DIRECTORY ${FAST_BINARY_DIR}
        DESTINATION bin
        FILES_MATCHING PATTERN "*.dll"
    )
else()
    install(
        DIRECTORY ${FAST_BINARY_DIR}
        DESTINATION bin
        FILES_MATCHING PATTERN "*.so"
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

if(WIN32 AND NOT UNIX)
    ## Windows
    # Create windows installer
else()
    ## UNIX
    # Create debian package
endif()

include(CPack)
