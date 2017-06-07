PROJECT( AlderVTK )

SET( AlderVTK_SRCS
  vtkAnimationPlayer.cxx
  vtkCenteredAxesActor.cxx
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

SET(VTK_LIBS
  vtkCommonCore
  vtkCommonMath
  vtkInteractionStyle
  vtkInteractionWidgets
  vtkIOCore
  vtkIOMINC
  vtkIOXML
  vtkRenderingFreeTypeOpenGL
  vtkRenderingImage
  vtkRenderingVolumeOpenGL
)

IF(${VTK_MAJOR_VERSION} GREATER 6)
  SET(VTK_LIBS
    vtkCommonCore
    vtkCommonMath
    vtkInteractionStyle
    vtkInteractionWidgets
    vtkIOCore
    vtkIOMINC
    vtkIOXML
    vtkRenderingFreeType
    vtkRenderingGL2PSOpenGL2
    vtkRenderingImage
    vtklibxml2
  )
ENDIF()

TARGET_LINK_LIBRARIES( AlderVTK
  AlderCommon
  ${VTK_LIBS}
  vtkgdcm
  ${CRYPTO++_LIBRARIES}
  ${LIBXML2_LIBRARIES}
)

INSTALL( TARGETS AlderVTK DESTINATION lib )
