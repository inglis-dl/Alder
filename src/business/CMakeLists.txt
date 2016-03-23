PROJECT( AlderBusiness )

SET( AlderBusiness_SRCS
  ActiveRecord.cxx
  Application.cxx
  Code.cxx
  CodeGroup.cxx
  CodeType.cxx
  Configuration.cxx
  Database.cxx
  Exam.cxx
  Image.cxx
  ImageNote.cxx
  Interview.cxx
  Modality.cxx
  ModelObject.cxx
  OpalService.cxx
  QueryModifier.cxx
  Rating.cxx
  ScanType.cxx
  Site.cxx
  User.cxx
  Wave.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  ActiveRecord.cxx
  ModelObject.cxx
  ABSTRACT
)

INCLUDE_DIRECTORIES(
  ${ALDER_COMMON_DIR}
  ${ALDER_VTK_DIR}
  ${LIBXML2_INCLUDE_DIR}
  ${CRYPTO++_INCLUDE_DIR}
  ${CURL_INCLUDE_DIR}
  ${JSONCPP_INCLUDE_DIR}
)

ADD_LIBRARY( AlderBusiness ${AlderBusiness_SRCS} )

TARGET_LINK_LIBRARIES( AlderBusiness
  AlderVTK
  ${CURL_LIBRARY}
  ${JSONCPP_LIBRARIES}
  vtkDICOM
)

INSTALL( TARGETS AlderBusiness
 DESTINATION lib
)
