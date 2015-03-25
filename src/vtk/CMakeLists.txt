PROJECT( AlderVTK )

SET( AlderVTK_SRCS
  vtkAnimationPlayer.cxx
  vtkAlderMySQLDatabase.cxx
  vtkAlderMySQLQuery.cxx
  vtkCustomCornerAnnotation.cxx
  vtkCustomInteractorStyleImage.cxx
  vtkFrameAnimationPlayer.cxx
  vtkImageCoordinateWidget.cxx
  vtkImageDataReader.cxx
  vtkImageDataWriter.cxx
  vtkImageWindowLevel.cxx
  vtkMedicalImageViewer.cxx
  vtkXMLFileReader.cxx
  vtkXMLConfigurationFileReader.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  vtkAnimationPlayer.cxx
  vtkXMLFileReader.cxx
  ABSTRACT
)

INCLUDE( ${VTK_USE_FILE} )

INCLUDE_DIRECTORIES(
  ${ALDER_COMMON_DIR}
  ${CRYPTO++_INCLUDE_DIR}
  ${LIBXML2_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIRECTORIES}
)

ADD_LIBRARY( AlderVTK ${AlderVTK_SRCS} )

TARGET_LINK_LIBRARIES( AlderVTK
  AlderCommon
  vtkCommon
  vtkIO
  vtkRendering
  vtkgdcm
  vtkWidgets
  ${MYSQL_LIBRARY}
  ${CRYPTO++_LIBRARIES}
  ${LIBXML2_LIBRARIES}
)

INSTALL( TARGETS AlderVTK
  DESTINATION lib
)
