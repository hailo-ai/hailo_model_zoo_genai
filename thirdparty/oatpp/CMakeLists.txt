fetchcontent_declare(oatpp
    GIT_REPOSITORY https://github.com/oatpp/oatpp
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DOATPP_INSTALL=OFF -DOATPP_BUILD_TESTS=OFF
    GIT_SHALLOW 1
)
fetchcontent_makeavailable(oatpp)

fetchcontent_getproperties(
  oatpp
  SOURCE_DIR OATPP_DIR_SRC
  BINARY_DIR OATPP_DIR_LIB
)

set(OATPP_MODULES_LOCATION
  "CUSTOM"
  CACHE STRING
  "Location where to find oatpp modules. can be [INSTALLED|EXTERNAL|CUSTOM]")

fetchcontent_declare(oatpp-openssl
    GIT_REPOSITORY https://github.com/oatpp/oatpp-openssl
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DOATPP_INSTALL=OFF -DOATPP_BUILD_TESTS=OFF
    GIT_SHALLOW 1
)
fetchcontent_makeavailable(oatpp-openssl)
