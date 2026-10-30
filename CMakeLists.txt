cmake_minimum_required(VERSION 3.16)

project(finalproduct VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick QuickControls2 Location Positioning)
qt_standard_project_setup(REQUIRES 6.8)

set(V2V_CORE_SOURCES
    roadgraph.h roadgraph.cpp
    node.h node.cpp
    edge.h edge.cpp
    vehicle.h vehicle.cpp
    trafficsimulation.h trafficsimulation.cpp
    roadgraphmodel.h roadgraphmodel.cpp
    vehiclemodel.h vehiclemodel.cpp
    interferencegraph.h interferencegraph.cpp
)

qt_add_executable(appfinalproduct main.cpp ${V2V_CORE_SOURCES})
qt_add_qml_module(appfinalproduct
    URI finalproduct
    VERSION 1.0
    QML_FILES Main.qml
)
qt_add_resources(appfinalproduct demo_map
    PREFIX "/"
    FILES data/demo.osm
)
target_link_libraries(appfinalproduct PRIVATE
    Qt6::Quick Qt6::QuickControls2 Qt6::Location Qt6::Positioning
)
set_target_properties(appfinalproduct PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    WIN32_EXECUTABLE TRUE
)

include(CTest)
if(BUILD_TESTING)
    add_executable(simulation_core_tests tests/core_tests.cpp ${V2V_CORE_SOURCES})
    target_include_directories(simulation_core_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(simulation_core_tests PRIVATE Qt6::Core)
    add_test(NAME simulation_core COMMAND simulation_core_tests ${CMAKE_CURRENT_SOURCE_DIR}/data/demo.osm)
    add_test(NAME application_smoke COMMAND appfinalproduct --smoke-test)
    set_tests_properties(application_smoke PROPERTIES
        TIMEOUT 20
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QML_DISABLE_DISK_CACHE=1"
    )
endif()

include(GNUInstallDirs)
install(TARGETS appfinalproduct
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
option(V2V_BUNDLE_QT "Bundle the shared Qt runtime when installing" OFF)
if(V2V_BUNDLE_QT)
    qt_generate_deploy_qml_app_script(
        TARGET appfinalproduct
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
        NO_TRANSLATIONS
    )
    install(SCRIPT ${deploy_script})
    if(TARGET Qt6::QOffscreenIntegrationPlugin)
        install(FILES $<TARGET_FILE:Qt6::QOffscreenIntegrationPlugin> DESTINATION plugins/platforms)
    endif()
    get_filename_component(qt_sdk_prefix "${Qt6_DIR}/../../.." ABSOLUTE)
    if(EXISTS "${qt_sdk_prefix}/sbom")
        install(DIRECTORY "${qt_sdk_prefix}/sbom/" DESTINATION ${CMAKE_INSTALL_DOCDIR}/qt-sbom
            FILES_MATCHING PATTERN "*.spdx.json")
    endif()
    foreach(license_name LGPL-3 GPL-3 GPL-2 LGPL-2.1 GFDL-1.3 Apache-2.0 BSD)
        if(EXISTS "/usr/share/common-licenses/${license_name}")
            install(FILES "/usr/share/common-licenses/${license_name}" DESTINATION ${CMAKE_INSTALL_DOCDIR}/qt-license-texts)
        endif()
    endforeach()
endif()
install(FILES README.md THIRD_PARTY_NOTICES.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES data/README.md DESTINATION ${CMAKE_INSTALL_DOCDIR} RENAME DATA.md)

set(CPACK_PACKAGE_NAME "v2v-traffic-simulation")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "TGZ")
include(CPack)
