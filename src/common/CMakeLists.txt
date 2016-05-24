PROJECT( AlderCommon )

SET( AlderCommon_SRCS
  Common.cxx
  Utilities.cxx
)

INCLUDE_DIRECTORIES( 
  ${CRYPTO++_INCLUDE_DIR}
)

ADD_LIBRARY( AlderCommon ${AlderCommon_SRCS} )

INSTALL( TARGETS AlderCommon
  DESTINATION lib 
)
