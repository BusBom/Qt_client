include("D:/workspace/project/Busbom_Qt/build/Desktop_Qt_6_9_1_MinGW_64_bit-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/Busbomproject-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE D:/workspace/project/Busbom_Qt/build/Desktop_Qt_6_9_1_MinGW_64_bit-Debug/Busbomproject.exe
    GENERATE_QT_CONF
)
