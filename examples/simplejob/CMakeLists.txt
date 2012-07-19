SET( test_qt_job_SOURCES "TestJob.cpp" "main.cpp" )
SET( test_job_HEADERS "TestJob.hpp" "utils.hpp" )
IF(${Tinia_DESKTOP})
  ADD_EXECUTABLE(simple_qt_job
    ${test_qt_job_SOURCES}
    ${test_job_HEADERS}
    )
  TARGET_LINK_LIBRARIES(simple_qt_job
    qtcontroller
    renderlist
    renderlistgl
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    model
    jobcontroller
    ${Boost_LIBRARIES}
    )

    INSTALL( TARGETS simple_qt_job
        RUNTIME DESTINATION bin
        COMPONENT simple_qt_job
    )
                
    IF(WIN32)
        # Boost_LIBRARIS and QT_LIBRARIES contain both debug and optimized (it is not called
        # release) libraries. For our 3rdparty we wish to include both. 

        function( printList theList )         
            foreach( lib ${theList} )
                message( ${lib} )
            endforeach( lib )
        endfunction( printList )

        
        set( tinia_qt_libraries ${QT_LIBRARIES} )
        set( tinia_boost_libraries ${Boost_LIBRARIES} ) 
        list( REMOVE_ITEM tinia_qt_libraries "optimized" )
        list( REMOVE_ITEM tinia_qt_libraries "debug" )

        list( REMOVE_ITEM tinia_boost_libraries "optimized" )
        list( REMOVE_ITEM tinia_boost_libraries "debug" )

        # Fetch the Qt DLLs from the library names
        foreach( Qt_library ${tinia_qt_libraries} )
            get_filename_component( Qt_library_name ${Qt_library} NAME_WE )
            # Note: the following Regex works for me on windows, I am not sure if it 
            # fully portable.
            string( REGEX REPLACE "^lib(.*)" "\\1" Qt_library_name ${Qt_library_name} )

            set( Qt_shared_library ${QT_BINARY_DIR}/${Qt_library_name}.dll )
            if( EXISTS ${Qt_shared_library} )
                # Add it to the list of 'desired' qt-libraries for later installation
                list( APPEND Qt_Install_Libraries ${Qt_shared_library} )
            endif( EXISTS ${Qt_shared_library} )
        endforeach( Qt_library ${QT_LIBRARIES} )
        
        list( REMOVE_DUPLICATES Qt_Install_Libraries )

        # Fetch the Boost DLLs from the libraya names
        foreach( Boost_library ${tinia_boost_libraries} )
            
            string( REGEX REPLACE "lib$" "dll" Boost_library ${Boost_library} )
            #message( ${Boost_library} )
            if ( EXISTS ${Boost_library} )
                #message( "It exists" )
                list( APPEND Boost_Install_Libraries ${Boost_library} )
            endif()            
        endforeach( Boost_library )
            
        #printList( "${Boost_Install_Libraries}" )
                
        INSTALL( FILES ${Qt_install_Libraries} ${Boost_Install_Libraries}
                 DESTINATION 3rdparty/bin
                 COMPONENT tinia_3rdparty
                        )

        INSTALL( FILES  ${GLEW_LIBRARY} ${tinia_qt_libraries} ${tinia_boost_libraries}
                 DESTINATION 3rdparty/lib
                 COMPONENT tinia_3rdparty
                )
    
    ENDIF()

ENDIF()


IF(${Tinia_SERVER})
  SET( test_cloud_job_SOURCES "TestJob.cpp" "cloud_main.cpp" )

  ADD_EXECUTABLE(test_job
    ${test_cloud_job_SOURCES}
    ${test_job_HEADERS}
    )

  TARGET_LINK_LIBRARIES(test_job
    ${RT}
    ${LIB_APR}
    ${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES} 
    ${LIBXML2_LIBRARIES} 
    model 
    modelxml
    renderlist
    renderlistgl
    jobcontroller
    trell
    )

    
  INSTALL(TARGETS
    test_job RUNTIME
    DESTINATION var/trell/apps/test_job/bin/
    COMPONENT mod_trell
    )
ENDIF()
