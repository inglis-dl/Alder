PROJECT( AlderQtGeneric )

SET( AlderQtGenericLib_SRC
  QAboutDialog.cxx
  QAlderDicomTagWidget.cxx
  QAlderDoubleSlider.cxx
  QAlderFramePlayerWidget.cxx
  QAlderSliderWidget.cxx
  QMedicalImageWidget.cxx
  QAlderImageWidget.cxx
  QAlderAbstractView.cxx
  QAlderSliceView.cxx
  QAlderImageControl.cxx
  QMailSender.cxx
)

SET( AlderQtGeneric_Uis
  QAboutDialog.ui
  QAlderDicomTagWidget.ui
  QAlderFramePlayerWidget.ui
  QAlderSliderWidget.ui
  QMedicalImageWidget.ui
  QAlderImageWidget.ui
  QAlderImageControl.ui
)

SET( AlderQtGeneric_MocHeaders
  QAboutDialog.h
  QAlderDicomTagWidget.h
  QAlderDoubleSlider.h
  QAlderFramePlayerWidget.h
  QAlderSliderWidget.h
  QMedicalImageWidget.h
  QAlderImageWidget.h
  QAlderAbstractView.h
  QAlderAbstractView_p.h
  QAlderSliceView.h
  QAlderSliceView_p.h
  QAlderImageControl.h
  QMailSender.h
)

SET( AlderQtGeneric_Resources
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/QAlderGenericWidgets.qrc
)

INCLUDE_DIRECTORIES(
  ${ALDER_COMMON_DIR}
  ${ALDER_VTK_DIR}
)

# Wrap QT files
QT4_WRAP_UI(       AlderQtGenericLib_UI  ${AlderQtGeneric_Uis} )
QT4_WRAP_CPP(      AlderQtGenericLib_MOC ${AlderQtGeneric_MocHeaders} )
QT4_ADD_RESOURCES( AlderQtGenericLib_QRC ${AlderQtGeneric_Resources} )

ADD_LIBRARY( AlderQtGeneric
  ${AlderQtGenericLib_SRC}
  ${AlderQtGenericLib_UI}
  ${AlderQtGenericLib_MOC}
  ${AlderQtGenericLib_QRC}
)

TARGET_LINK_LIBRARIES( AlderQtGeneric
 Qt4::QtNetwork
) 

INSTALL( TARGETS AlderQtGeneric 
  DESTINATION lib
)
