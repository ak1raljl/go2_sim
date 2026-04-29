cmake_minimum_required(VERSION 3.5)
project(go2_description VERSION 3.0.0)

find_package(catkin REQUIRED)
catkin_package()
find_package(roslaunch)
foreach(dir config launch meshes urdf xacro)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
        install(
            DIRECTORY ${dir}/
            DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/${dir}
        )
    endif()
endforeach(dir)
