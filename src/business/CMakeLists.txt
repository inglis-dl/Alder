PROJECT( AlderBusiness )

SET( AlderBusiness_SRCS
  ActiveRecord.cxx
  Application.cxx
  Configuration.cxx
  Database.cxx
  Exam.cxx
  Image.cxx
  Interview.cxx
  Modality.cxx
  ModelObject.cxx
  OpalService.cxx
  ProgressProxy.cxx
  QueryModifier.cxx
  Rating.cxx
  User.cxx
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
)  
  
INSTALL( TARGETS AlderBusiness 
 DESTINATION lib
)
