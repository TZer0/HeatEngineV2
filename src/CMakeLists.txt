add_executable(HeatEngineV2 material.cpp area.cpp simulation.cpp heatengine.cpp)

target_link_libraries(HeatEngineV2 ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${OPENAL_LIBRARIES})


