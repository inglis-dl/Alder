PROJECT( AlderQtModel )

SET( AlderQtModelLib_SRC
  QAlderAtlasWidget.cxx
  QAlderInterviewWidget.cxx
  QCodeDialog.cxx
  QLoginDialog.cxx
  QChangePasswordDialog.cxx
  QReportDialog.cxx
  QSelectInterviewDialog.cxx
  QSelectWaveDialog.cxx
  QUserListDialog.cxx
  QMailSender.cxx
  QVTKProgressDialog.cxx
)

SET( AlderQtModel_Uis
  QAlderAtlasWidget.ui
  QAlderInterviewWidget.ui
  QCodeDialog.ui
  QChangePasswordDialog.ui
  QLoginDialog.ui
  QReportDialog.ui
  QSelectInterviewDialog.ui
  QSelectWaveDialog.ui
  QUserListDialog.ui
  QVTKProgressDialog.ui
)

SET( AlderQtModel_MocHeaders
  QAlderAtlasWidget.h
  QAlderInterviewWidget.h
  QChangePasswordDialog.h
  QCodeDialog.h
  QLoginDialog.h
  QReportDialog.h
  QSelectInterviewDialog.h
  QSelectWaveDialog.h
  QUserListDialog.h
  QMailSender.h
  QVTKProgressDialog.h
)

SET( AlderQtModel_Resources
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/QAlderModelWidgets.qrc
)

# Wrap QT files
QT4_WRAP_UI(       AlderQtModelLib_UI  ${AlderQtModel_Uis} )
QT4_WRAP_CPP(      AlderQtModelLib_MOC ${AlderQtModel_MocHeaders} )
QT4_ADD_RESOURCES( AlderQtModelLib_QRC ${AlderQtModel_Resources} )

INCLUDE_DIRECTORIES(
  ${ALDER_COMMON_DIR}
  ${ALDER_BUSINESS_DIR}
  ${ALDER_VTK_DIR}
  ${CRYPTO++_INCLUDE_DIR}
)

ADD_LIBRARY( AlderQtModel
  ${AlderQtModelLib_SRC}
  ${AlderQtModelLib_UI}
  ${AlderQtModelLib_MOC}
  ${AlderQtModelLib_QRC}
)

TARGET_LINK_LIBRARIES( AlderQtModel
  AlderVTK
  QVTK
  Qt4::QtNetwork
)

INSTALL( TARGETS AlderQtModel
  DESTINATION lib
)
