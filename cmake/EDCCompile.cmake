# Macro for inclusion of EDJ files into build            
FIND_PROGRAM(EDJE_CC_EXECUTABLE edje_cc DOC "path to edje_cc compiler")
MARK_AS_ADVANCED(EDJE_CC_EXECUTABLE)

IF(EDJE_CC_EXECUTABLE)
    SET(EDJ_TARGET_DEPENDENCIES "")

    MACRO(EDJ_TARGET Name Edc_Input Edj_Output)
        SET(EDJE_TARGET_depends "")
         IF(${ARGC} GREATER 3)
             SET(tmp_arg_counter 0)  
             FOREACH(tmp_arg ${ARGV})
                 math(EXPR tmp_arg_counter "${tmp_arg_counter}+1")
                 IF(${tmp_arg_counter} GREATER 3)
                     SET(EDJE_TARGET_depends ${EDJE_TARGET_depends} "${tmp_arg}")
                 ENDIF()
             ENDFOREACH(tmp_arg)
         ENDIF()

         IF(${PROFILE} MATCHES "mobile")
            SET(IMAGES_URL "images_mob")
            SET(BROWSER_RESOLUTION "720x1280")
            SET(IF_PROFILE_MOBILE 1)
         ELSE(${PROFILE} MATCHES "mobile")
            SET(IMAGES_URL "images")
            SET(BROWSER_RESOLUTION "1920x1080")
            SET(IF_PROFILE_MOBILE 0)
         ENDIF()

         FOREACH(resolution ${BROWSER_RESOLUTION})
             SET(tmp_output ${Edj_Output}/${resolution}_${Name})
             ADD_CUSTOM_COMMAND(OUTPUT ${tmp_output}
                                COMMAND ${EDJE_CC_EXECUTABLE}
                                ARGS -id ${CMAKE_CURRENT_SOURCE_DIR}/${IMAGES_URL}/
                                -DBROWSER_RESOLUTION_${resolution}=1
                                -DPROFILE_MOBILE=${IF_PROFILE_MOBILE}
                                -DDUMMY_BUTTON=${IF_DUMMY_BUTTON}
                                ${Edc_Input} ${tmp_output}
                                MAIN_DEPENDENCY ${Edc_Input}
                                DEPENDS ${EDJE_TARGET_depends}
                                COMMENT "[EDJE_CC][${tmp_output}] Compiling EDC file ${Edc_Input}"
                                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

             ADD_CUSTOM_TARGET(${resolution}_${Name} DEPENDS ${tmp_output})

             ADD_DEPENDENCIES(${PROJECT_NAME} ${resolution}_${Name})

             INSTALL(FILES ${tmp_output} DESTINATION ${EDJE_DIR}/${PROJECT_NAME} RENAME ${Name})
         ENDFOREACH(resolution)
     ENDMACRO(EDJ_TARGET)
 ELSE()
     MESSAGE(FATAL_ERROR "edje_cc tool not found")
 ENDIF(EDJE_CC_EXECUTABLE)
