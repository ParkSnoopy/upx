if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
   NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$" OR
   NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    return()
endif()

set(extension_fixture_source "${CMAKE_CURRENT_SOURCE_DIR}/src/extension/analysis_decoy_test.c")
add_executable(upx_extension_test "${extension_fixture_source}")
target_link_libraries(upx_extension_test PRIVATE ${CMAKE_DL_LIBS})
add_library(upx_extension_fixture SHARED "${extension_fixture_source}")
target_compile_definitions(upx_extension_fixture PRIVATE UPX_EXTENSION_SHARED_FIXTURE=1)
set(extension_targets "$<TARGET_FILE:upx_extension_test>")

foreach(method IN ITEMS nrv2b lzma)
    foreach(kind IN ITEMS main library)
        if(kind STREQUAL "main")
            set(input "$<TARGET_FILE:upx_extension_test>")
            set(packed "${CMAKE_CURRENT_BINARY_DIR}/XTesting/$<CONFIG>/extension-${method}")
            set(target "${packed}")
        else()
            set(input "$<TARGET_FILE:upx_extension_fixture>")
            set(packed "${CMAKE_CURRENT_BINARY_DIR}/XTesting/$<CONFIG>/extension-${method}.so")
            set(target "$<TARGET_FILE:upx_extension_test>")
        endif()
        list(APPEND extension_targets "${packed}")
        set(pack_test "upx-extension-pack-${kind}-${method}")
        upx_add_test(${pack_test} upx -1 --${method} "${input}" --force-overwrite -o "${packed}")
        foreach(tool IN ITEMS upx-normal ida ida64 idat idat64 idaq idaq64 ghidra ghidraRun
                              binaryninja java-ghidra java ida-helper ghidra-helper)
            set(process_name "${tool}")
            set(marker "")
            set(expected decoy)
            if(tool STREQUAL "java-ghidra")
                set(process_name java)
                set(marker ghidra.Ghidra)
            elseif(tool MATCHES "^(upx-normal|java|ida-helper|ghidra-helper)$")
                set(expected original)
            endif()
            set(test "upx-extension-${kind}-${method}-${tool}")
            upx_add_test(${test} upx_extension_test "${target}" "${kind}" "${packed}"
                         "${process_name}" "${expected}" ${marker})
            upx_test_depends(${test} ${pack_test})
            # Synthetic tool names are visible to other packed processes too.
            set_tests_properties(${test} PROPERTIES RUN_SERIAL TRUE TIMEOUT 20)
        endforeach()
    endforeach()
endforeach()

set(opaque_packed "${CMAKE_CURRENT_BINARY_DIR}/XTesting/$<CONFIG>/extension-opaque")
list(APPEND extension_targets "${opaque_packed}")
string(REPLACE ";" "\", \"" extension_targets "${extension_targets}")
target_compile_definitions(upx_extension_test PRIVATE "UPX_EXTENSION_TARGETS=\"${extension_targets}\"")
upx_add_test(upx-extension-pack-main-opaque upx -1 --opaque
             "$<TARGET_FILE:upx_extension_test>" --force-overwrite -o "${opaque_packed}")
upx_add_test(upx-extension-opaque-run upx_extension_test "${opaque_packed}" main
             "${opaque_packed}" upx-normal original)
upx_test_depends(upx-extension-opaque-run upx-extension-pack-main-opaque)
upx_add_test(upx-extension-opaque-decoy upx_extension_test "${opaque_packed}" main
             "${opaque_packed}" ida decoy)
upx_test_depends(upx-extension-opaque-decoy upx-extension-pack-main-opaque)
set_tests_properties(upx-extension-opaque-decoy PROPERTIES RUN_SERIAL TRUE TIMEOUT 20)

set(opaque_unpacked "${CMAKE_CURRENT_BINARY_DIR}/XTesting/$<CONFIG>/extension-opaque-unpacked")
upx_add_test(upx-extension-opaque-unpack upx -d "${opaque_packed}" -o "${opaque_unpacked}" -f)
upx_test_depends(upx-extension-opaque-unpack upx-extension-pack-main-opaque)
upx_add_test(upx-extension-opaque-unpacked-run "${opaque_unpacked}" --fixture)
upx_test_depends(upx-extension-opaque-unpacked-run upx-extension-opaque-unpack)

upx_add_test(upx-extension-reject-executable upx_extension_test "${opaque_unpacked}" main
             "${opaque_unpacked}" upx-normal original)
upx_test_depends(upx-extension-reject-executable upx-extension-opaque-unpack)
upx_add_test(upx-extension-reject-library upx_extension_test --load
             "$<TARGET_FILE:upx_extension_fixture>")
set_tests_properties(upx-extension-reject-executable upx-extension-reject-library
                     PROPERTIES WILL_FAIL TRUE)
