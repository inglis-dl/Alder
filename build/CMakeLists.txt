CMAKE_MINIMUM_REQUIRED( VERSION 2.8.7 )
PROJECT( Alder )

# The major version number is incremented after major milestones are met
SET( ALDER_VERSION_MAJOR 1 )
# The minor version number is incremented after a development branch is merged to qa
SET( ALDER_VERSION_MINOR 7 )
# The patch version number is incremented when changes are made to the release branch
SET( ALDER_VERSION_PATCH 0 )

# Icons sizes to build
SET( ALDER_ICON_SIZES 16x16 22x22 24x24 32x32 48x48 64x64 128x128 )

# Look in the build directory for cmake modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/" )

# We're using cbegin and cend so we need c++11
SET( CMAKE_CXX_FLAGS "-std=c++0x -Wno-deprecated" )

# We need VTK
FIND_PACKAGE( VTK REQUIRED )
INCLUDE( ${VTK_USE_FILE} )
IF( "${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}" VERSION_LESS 6.2 )
  MESSAGE( FATAL_ERROR "VTK 6.2 or later is required." )
ENDIF()

# We need GDCM
FIND_PACKAGE( GDCM REQUIRED )
INCLUDE( ${GDCM_USE_FILE} )

# We need vtk-dicom
FIND_PACKAGE( DICOM REQUIRED )
INCLUDE( ${DICOM_USE_FILE} )
SET( VTK_DICOM_LBRARIES vtkDICOM )

# We need Qt5
SET( QT_MIN_VERSION 5.7.0 )
FIND_PACKAGE( Qt5 COMPONENTS Core Widgets Network REQUIRED )
INCLUDE_DIRECTORIES(
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
)
ADD_DEFINITIONS(
  ${Qt5Widgets_DEFINITIONS}
  ${Qt5Network_DEFINITIONS}
  ${Qt5Core_DEFINITIONS}
)

# We need libxml2
FIND_PACKAGE( LibXml2 REQUIRED )

# We need Crypto++
FIND_PACKAGE( CRYPTO++ REQUIRED )

# We need JSONCpp
FIND_PACKAGE( JSONCpp REQUIRED )

# We need MySQL
FIND_PACKAGE( MySQL REQUIRED )

# We need CURL
FIND_PACKAGE( CURL REQUIRED )

# We need convert
IF( UNIX AND NOT APPLE )
  FIND_PACKAGE( ImageMagick COMPONENTS convert REQUIRED )
ENDIF( UNIX AND NOT APPLE )

SET( ALDER_ROOT_DIR       ${PROJECT_SOURCE_DIR}/.. )
SET( ALDER_AUX_DIR        ${ALDER_ROOT_DIR}/aux )
SET( ALDER_SRC_DIR        ${ALDER_ROOT_DIR}/src )
SET( ALDER_COMMON_DIR     ${ALDER_SRC_DIR}/common )
SET( ALDER_BUSINESS_DIR   ${ALDER_SRC_DIR}/business )
SET( ALDER_QT_DIR         ${ALDER_SRC_DIR}/interface/qt )
SET( ALDER_QT_GENERIC_DIR ${ALDER_QT_DIR}/generic )
SET( ALDER_QT_MODEL_DIR   ${ALDER_QT_DIR}/model )
SET( ALDER_VTK_DIR        ${ALDER_SRC_DIR}/vtk )
SET( ALDER_APP_DIR        ${ALDER_QT_DIR}/application )
SET( ALDER_DOC_DIR        ${ALDER_ROOT_DIR}/doc )
SET( ALDER_DOXY_DIR       ${PROJECT_BINARY_DIR}/doxygen )
SET( ALDER_TESTING_DIR    ${ALDER_ROOT_DIR}/testing )
#SET( ALDER_PROGRAMS_DIR   ${ALDER_SRC_DIR}/programs )

# Define the config file location, then test to see if it is in the install path when installing
SET( ALDER_CONFIG_FILE ${PROJECT_BINARY_DIR}/config.xml CACHE PATH "The location of Alder's runtime config file" )

IF( NOT ${ALDER_CONFIG_FILE} MATCHES ${CMAKE_INSTALL_PREFIX} )
  INSTALL( CODE "MESSAGE( \"
### WARNING ###
The location of the config is not within the install prefix!
In order to use the installed version of the config file (${CMAKE_INSTALL_PREFIX}/etc/config.xml)
you must explicitly set ALDER_CONFIG_FILE to that path
\" )" )
ENDIF( NOT ${ALDER_CONFIG_FILE} MATCHES ${CMAKE_INSTALL_PREFIX} )

# Create a salt string which can be set at build time
SET( ALDER_SALT_STRING "this is salt" CACHE STRING "This is salt for encrypting passwords" )

# Define stack depth to display on thrown exceptions
SET( ALDER_STACK_DEPTH "10" CACHE STRING "The stack depth to display on thrown exceptions" )

# Configure a header file to pass CMake settings to
CONFIGURE_FILE( ${ALDER_SRC_DIR}/AlderConfig.h.in
                ${PROJECT_BINARY_DIR}/AlderConfig.h @ONLY IMMEDIATE )

# Configure the desktop file
CONFIGURE_FILE( ${ALDER_AUX_DIR}/alder.desktop.in
                ${PROJECT_BINARY_DIR}/alder.desktop @ONLY IMMEDIATE )

# Copy the readme file to build
CONFIGURE_FILE( ${ALDER_ROOT_DIR}/readme
                ${PROJECT_BINARY_DIR}/readme COPYONLY )

# Run the configuration initialization script if the config xml file is not found
IF( NOT EXISTS ${ALDER_CONFIG_FILE} )
  MESSAGE( "No config.xml file was found, building one" )
  EXECUTE_PROCESS( COMMAND ${ALDER_AUX_DIR}/init_config ${ALDER_CONFIG_FILE} )
ENDIF( NOT EXISTS ${ALDER_CONFIG_FILE} )

# Set default locations for exe and lib files
SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin )
SET( LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib )

# Make sure to include RPATH in the installed binary to support linking to libraries
SET( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" )
SET( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )

# Set up include directories
SET( CMAKE_INCLUDE_CURRENT_DIR ON )

SET( BUILD_SHARED_LIBS ON )

# The libraries need AlderConfig.h
INCLUDE_DIRECTORIES( BEFORE
  ${PROJECT_BINARY_DIR}
)

SET( ALDER_LIBRARIES
  AlderQtGeneric
  AlderQtModel
  AlderVTK
  AlderBusiness
  AlderCommon
)

# Alder libraries for use in other applications
ADD_SUBDIRECTORY(
  ${ALDER_COMMON_DIR} ${PROJECT_BINARY_DIR}/common
)
ADD_SUBDIRECTORY(
  ${ALDER_VTK_DIR} ${PROJECT_BINARY_DIR}/vtk
)
ADD_SUBDIRECTORY(
  ${ALDER_BUSINESS_DIR} ${PROJECT_BINARY_DIR}/business
)
ADD_SUBDIRECTORY(
  ${ALDER_QT_GENERIC_DIR} ${PROJECT_BINARY_DIR}/generic
)
ADD_SUBDIRECTORY(
  ${ALDER_QT_MODEL_DIR} ${PROJECT_BINARY_DIR}/model
)
ADD_SUBDIRECTORY(
  ${ALDER_APP_DIR} ${PROJECT_BINARY_DIR}/application
)

INSTALL( FILES ${CMAKE_BINARY_DIR}/config.xml DESTINATION etc )

ADD_CUSTOM_TARGET( dist
  COMMAND git archive --prefix=${ALDER_ARCHIVE_NAME}/ HEAD
    | bzip2 > ${CMAKE_BINARY_DIR}/${ALDER_ARCHIVE_NAME}.tar.bz2
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Desktop entries and icons
IF( UNIX AND NOT APPLE )
  INSTALL( FILES ${PROJECT_BINARY_DIR}/alder.desktop DESTINATION share/applications )

  FOREACH( SIZE ${ALDER_ICON_SIZES} )
    ADD_CUSTOM_COMMAND( OUTPUT logo_clsa_${SIZE}.png
      COMMAND convert -background none -resize ${SIZE} ${ALDER_AUX_DIR}/logo_clsa.png logo_clsa_${SIZE}.png
      COMMENT "Generating ${SIZE} icon"
    )
    SET( ALDER_ICON_FILES ${ALDER_ICON_FILES} logo_clsa_${SIZE}.png )
    INSTALL( FILES ${CMAKE_BINARY_DIR}/logo_clsa_${SIZE}.png
      DESTINATION share/icons/hicolor/${SIZE}/apps
      RENAME logo_clsa.png
    )
  ENDFOREACH( SIZE )
  ADD_CUSTOM_TARGET( icons ALL DEPENDS ${ALDER_ICON_FILES} )
ENDIF( UNIX AND NOT APPLE )

# Build doxygen documentation ?
INCLUDE( ${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL )
IF( BUILD_DOCUMENTATION )

  SET( HAVE_DOT_YESNO NO )
  IF( DOT )
    SET( HAVE_DOT_YESNO YES )
    IF( NOT DOT_PATH )
      GET_FILENAME_COMPONENT( DOT_PATH ${DOT} PATH )
    ENDIF( NOT DOT_PATH )
  ENDIF( DOT )

  CONFIGURE_FILE(
    ${ALDER_DOC_DIR}/doc_mainpage.dox.in
    ${ALDER_DOXY_DIR}/doc_mainpage.dox )

  CONFIGURE_FILE(
    ${ALDER_DOC_DIR}/doc_makeall.cmake.in
    ${ALDER_DOXY_DIR}/doc_makeall.cmake
    @ONLY )

  CONFIGURE_FILE(
    ${ALDER_DOC_DIR}/doc_mainpage.dox.in
    ${ALDER_DOXY_DIR}/doc_mainpage.dox )

  SET( DOXY_INPUT_SOURCE
    ${ALDER_SRC_DIR}
    ${ALDER_BUSINESS_DIR}
    ${ALDER_COMMON_DIR}
    ${ALDER_VTK_DIR}
    ${ALDER_QT_GENERIC_DIR}
    ${ALDER_QT_MODEL_DIR}
    ${ALDER_DOXY_DIR}/doc_mainpage.dox )

  STRING( REPLACE ";" "\" \"" ALDER_DOXY_LIST "${DOXY_INPUT_SOURCE}" )

  CONFIGURE_FILE(
    ${ALDER_DOC_DIR}/config.dox.in
    ${ALDER_DOXY_DIR}/config.dox )

# Uncommenting this block will force a build of the documentation
# every time cmake is run in the build directory
#
#  execute_process(COMMAND
#    ${CMAKE_COMMAND} -P ${ALDER_DOXY_DIR}/doc_makeall.cmake
#    DEPENDS ${ALDER_DOXY_DIR}/doc_makeall.cmake
#    WORKING DIR ${ALDER_DOXY_DIR}/doxygen
#    RESULT_VARIABLE rv )

ENDIF( BUILD_DOCUMENTATION )
