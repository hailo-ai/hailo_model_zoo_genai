fetchcontent_declare(oatpp
    GIT_REPOSITORY https://github.com/oatpp/oatpp
    CMAKE_ARGS -DOATPP_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
    GIT_SHALLOW 1
)
fetchcontent_makeavailable(oatpp)
