function(CompileShader filename outputname)
    message(STATUS "Compiling Shader [ ${filename} ]")

    set(shader_input_filepath "${GAMEZERO_SHADERS_DIR}/${filename}")
    set(shader_output_filepath "${GAMEZERO_BUILD_DIR}/shaders/${outputname}")
    set(shader_compile_command "glslc -c ${shader_input_filepath} -o ${shader_output_filepath}")
    
    message("Shader Compile Command : ${shader_compile_command}")
    execute_process(COMMAND ${shader_compile_command})
    
    message(STATUS "Compiling Shader [ ${filename} ] - done")
endfunction(CompileShader filename)
