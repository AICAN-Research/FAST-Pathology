
# Install pathology application
install(TARGETS fastPathology
    DESTINATION bin
)

# Install FAST dependency
install(
    DIRECTORY ${FAST_BINARY_DIR}
    DESTINATION bin
)
install(
    DIRECTORY ${FAST_BINARY_DIR}/../kernels/
    DESTINATION bin
)


if(WIN32 AND NOT UNIX)
    ## Windows
    # Create windows installer
else()
    ## UNIX
    # Create debian package
endif()

include(CPack)
