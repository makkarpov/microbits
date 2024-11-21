file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)

add_library(ub_crypto INTERFACE)
add_library(UB::Crypto ALIAS ub_crypto)

target_sources(ub_crypto INTERFACE ${SOURCES})
target_include_directories(ub_crypto INTERFACE include)

ub_install_library(UB::Crypto)

if ("${ENABLE_TESTING}")
    set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/test-generated")

    file(MAKE_DIRECTORY "${GENERATED_DIR}")
    set(GENERATED_FILES "")

    function(run_test_generator FILE SCRIPT)
        file(REAL_PATH "${FILE}" ABS_FILE BASE_DIRECTORY "${GENERATED_DIR}")
        file(REAL_PATH "${SCRIPT}" ABS_SCRIPT BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")

        add_custom_command(
                OUTPUT "${ABS_FILE}"
                COMMAND "${CMAKE_COMMAND}" -E env
                    "PYTHONPATH=${TEST_PYTHON_LIB}"
                    python3 -B "${ABS_SCRIPT}" ${ARGN} "${ABS_FILE}"
                WORKING_DIRECTORY "${GENERATED_DIR}"
                DEPENDS "${ABS_SCRIPT}" ${TEST_PYTHON_FILES}
                COMMENT "Generating (src/crypto/test) ${FILE}"
        )

        set(NEXT_GENERATED_FILES "${GENERATED_FILES}")
        list(APPEND NEXT_GENERATED_FILES "${ABS_FILE}")
        set(GENERATED_FILES "${NEXT_GENERATED_FILES}" PARENT_SCOPE)
    endfunction()

    function(add_crypto_test FILE)
        get_filename_component(NAME "${FILE}" NAME_WLE)
        string(REGEX REPLACE "[^a-z_0-9]+" "_" NAME "${NAME}")
        string(PREPEND NAME "test_crypto_")

        add_executable("${NAME}" EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/test/${FILE}")

        foreach (F IN LISTS ARGN)
            file(REAL_PATH "${F}" AF BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")
            target_sources("${NAME}" PRIVATE "${AF}")
        endforeach ()

        target_link_libraries("${NAME}" PRIVATE ub_crypto_tests)
        target_compile_options("${NAME}" PRIVATE -Wall -Wextra)

        add_test(NAME "${NAME}" COMMAND "${NAME}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    endfunction()

    run_test_generator(sha256_test_data.cpp hash/hash_test_gen.py sha256)
    run_test_generator(sha512_test_data.cpp hash/hash_test_gen.py sha512)
    run_test_generator(sha3_256_test_data.cpp hash/hash_test_gen.py sha3_256)
    run_test_generator(sha3_512_test_data.cpp hash/hash_test_gen.py sha3_512)
    run_test_generator(shake128_test_data.cpp hash/hash_test_gen.py shake_128)
    run_test_generator(shake256_test_data.cpp hash/hash_test_gen.py shake_256)
    run_test_generator(f25519_test_data.cpp edwards/f25519_test_gen.py)
    run_test_generator(f448_test_data.cpp edwards/f448_test_gen.py)
    run_test_generator(ed25519_test_data.cpp edwards/ed25519_test_gen.py)
    run_test_generator(ed448_test_data.cpp edwards/ed448_test_gen.py)
    run_test_generator(fprime_test_data.cpp edwards/fprime_test_gen.py)
    run_test_generator(eddsa_test_data.cpp edwards/eddsa_test_gen.py)
    run_test_generator(eddh_test_data.cpp edwards/eddh_test_gen.py)
    run_test_generator(aes_test_data.cpp cipher/block_test_gen.py aes)
    run_test_generator(chacha20_test_data.cpp cipher/stream_test_gen.py chacha20)
    run_test_generator(hmac_sha256_test_data.cpp mac/mac_test_gen.py sha256)
    run_test_generator(hmac_sha512_test_data.cpp mac/mac_test_gen.py sha512)
    run_test_generator(kmac128_test_data.cpp mac/mac_test_gen.py kmac128)
    run_test_generator(kmac256_test_data.cpp mac/mac_test_gen.py kmac256)

    # Intermediate static library to avoid repeated compilations when building a lot of tests
    add_library(ub_crypto_tests STATIC EXCLUDE_FROM_ALL "${GENERATED_FILES}" test/test_utils.cpp)
    target_include_directories(ub_crypto_tests PUBLIC src test include)
    target_link_libraries(ub_crypto_tests PRIVATE ub_crypto)
    target_compile_options(ub_crypto_tests PUBLIC -O3)

    add_crypto_test(hash/sha2_sha256.cpp)
    add_crypto_test(hash/sha2_sha512.cpp)
    add_crypto_test(hash/sha3_sha3.cpp)
    add_crypto_test(hash/sha3_shake.cpp)
    add_crypto_test(edwards/f25519.cpp)
    add_crypto_test(edwards/f448.cpp)
    add_crypto_test(edwards/ed25519.cpp)
    add_crypto_test(edwards/ed448.cpp)
    add_crypto_test(edwards/fprime.cpp)
    add_crypto_test(edwards/eddsa.cpp)
    add_crypto_test(edwards/eddh.cpp)
    add_crypto_test(cipher/aes.cpp)
    add_crypto_test(cipher/chacha20.cpp)
    add_crypto_test(mac/hmac.cpp)
    add_crypto_test(mac/kmac.cpp)
endif ()

if ("${ENABLE_DEVICE_TESTING}")
    set(COMMON_ARGS "LIBRARIES;ub_crypto;CC_FLAGS;-Os;-Wall;-Wextra")

    add_device_test(
            NAME        crypto_eddsa_verify_25519_Os
            SOURCES     test/device/eddsa_verify.cpp
            RUNNER      test/device/eddsa_runner.py verify 25519
            ${COMMON_ARGS}
    )

    add_device_test(
            NAME        crypto_eddsa_verify_448_Os
            SOURCES     test/device/eddsa_verify.cpp
            RUNNER      test/device/eddsa_runner.py verify 448
            DEFINES     TEST_ED448=1
            ${COMMON_ARGS}
    )

    add_device_test(
            NAME        crypto_eddsa_sign_25519_Os
            SOURCES     test/device/eddsa_sign.cpp
            RUNNER      test/device/eddsa_runner.py sign 25519
            ${COMMON_ARGS}
    )

    add_device_test(
            NAME        crypto_eddsa_sign_448_Os
            SOURCES     test/device/eddsa_sign.cpp
            RUNNER      test/device/eddsa_runner.py sign 448
            DEFINES     TEST_ED448=1
            ${COMMON_ARGS}
    )

    add_device_test(
            NAME        crypto_eddh_25519_Os
            SOURCES     test/device/eddh.cpp
            RUNNER      test/device/eddh_runner.py 25519
            ${COMMON_ARGS}
    )

    add_device_test(
            NAME        crypto_eddh_448_Os
            SOURCES     test/device/eddh.cpp
            RUNNER      test/device/eddh_runner.py 448
            DEFINES     TEST_ED448=1
            ${COMMON_ARGS}
    )

    add_device_test(
            NAME        crypto_sha2_sha256_Os
            SOURCES     test/device/sha2_sha256.cpp
            RUNNER      test/device/hash_runner.py sha256
            ${COMMON_ARGS}
    )

    add_device_test(
            NAME        crypto_sha2_sha512_Os
            SOURCES     test/device/sha2_sha512.cpp
            RUNNER      test/device/hash_runner.py sha512
            ${COMMON_ARGS}
    )

    add_device_test(
            NAME        crypto_sha3_shake256_Os
            SOURCES     test/device/sha3_shake256.cpp
            RUNNER      test/device/hash_runner.py shake_256
            ${COMMON_ARGS}
    )

    add_device_test(
            NAME        crypto_sha3_sha256_Os
            SOURCES     test/device/sha3_sha256.cpp
            RUNNER      test/device/hash_runner.py sha3_256
            ${COMMON_ARGS}
    )

    add_device_test(
            NAME        crypto_aes_Os
            SOURCES     test/device/cipher.cpp
            RUNNER      test/device/cipher_runner.py aes
            DEFINES     TEST_AES=1
            ${COMMON_ARGS}
    )

    add_device_test(
            NAME        crypto_chacha20_Os
            SOURCES     test/device/cipher.cpp
            RUNNER      test/device/cipher_runner.py chacha20
            DEFINES     TEST_CHACHA20=1
            ${COMMON_ARGS}
    )
endif ()
