SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_HANDLERSOCKET")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_HANDLERSOCKET")

IF(HAVE_WVLA)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-vla")
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-vla")
ENDIF()

IF(MSVC)
  # Temporarily disable "conversion from size_t .."
  IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267")
  ENDIF()
ENDIF()

SET(SPIDER_SOURCES
  spd_param.cc spd_sys_table.cc spd_trx.cc spd_db_conn.cc spd_conn.cc
  spd_table.cc spd_direct_sql.cc spd_udf.cc spd_ping_table.cc
  spd_copy_tables.cc spd_i_s.cc spd_malloc.cc ha_spider.cc spd_udf.def
  spd_db_mysql.cc spd_db_handlersocket.cc spd_db_oracle.cc
  spd_group_by_handler.cc spd_db_include.cc
  hs_client/config.cpp hs_client/escape.cpp hs_client/fatal.cpp
  hs_client/hstcpcli.cpp hs_client/socket.cpp hs_client/string_util.cpp
)

IF(DEFINED ENV{ORACLE_HOME})
  SET(ORACLE_HOME $ENV{ORACLE_HOME})
  FIND_PATH(ORACLE_INCLUDE_DIR oci.h PATHS ${ORACLE_HOME}/rdbms/public)
  SET(ORACLE_OCI_LIB_NAME clntsh)
  SET(ORACLE_LIB_DIR ${ORACLE_HOME}/lib)
  FIND_LIBRARY(ORACLE_OCI_LIBRARY NAMES ${ORACLE_OCI_LIB_NAME} PATHS ${ORACLE_LIB_DIR})
ENDIF()

IF(EXISTS ${PROJECT_SOURCE_DIR}/storage/mysql_storage_engine.cmake)
  SET(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} -DSAFEMALLOC -DSAFE_MUTEX -DUSE_SYMDIR /Zi")
  SET(CMAKE_C_FLAGS_DEBUG
    "${CMAKE_C_FLAGS_DEBUG} -DSAFEMALLOC -DSAFE_MUTEX -DUSE_SYMDIR /Zi")
  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /MAP /MAPINFO:EXPORTS")
  INCLUDE("${PROJECT_SOURCE_DIR}/storage/mysql_storage_engine.cmake")

  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/sql
    ${CMAKE_SOURCE_DIR}/regex)

  MYSQL_STORAGE_ENGINE(SPIDER)
ELSEIF(PLUGIN_PARTITION MATCHES "^NO$")
  MESSAGE(STATUS "Spider is skipped because partitioning is disabled")
ELSE()
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/storage/spider/hs_client)

  IF(DEB)
    SET(extra_options COMPONENT spider-engine)
  ELSE()
    SET(extra_options CONFIG spider.cnf)
  ENDIF()

  MYSQL_ADD_PLUGIN(spider ${SPIDER_SOURCES} ${extra_options}
    STORAGE_ENGINE MODULE_ONLY)
  IF(NOT TARGET spider)
    RETURN()
  ENDIF()
ENDIF()

IF(ORACLE_INCLUDE_DIR AND ORACLE_OCI_LIBRARY)
  SET(SPIDER_WITH_ORACLE_OCI OFF CACHE BOOL "Spider is compiled with Oracle OCI library.")
  IF(SPIDER_WITH_ORACLE_OCI)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_ORACLE_OCI -DLINUX -D_GNU_SOURCE -D_REENTRANT")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_ORACLE_OCI -DLINUX -D_GNU_SOURCE -D_REENTRANT")
    INCLUDE_DIRECTORIES(${ORACLE_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES (spider ${ORACLE_OCI_LIBRARY})
  ENDIF()
ENDIF()

IF(MSVC AND (TARGET spider))
  IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_CUSTOM_COMMAND(TARGET spider
                       POST_BUILD
                       COMMAND if not exist ..\\..\\sql\\lib mkdir ..\\..\\sql\\lib\\plugin
                       COMMAND copy Debug\\ha_spider.dll ..\\..\\sql\\lib\\plugin\\ha_spider.dll)
  ENDIF()
ENDIF()
