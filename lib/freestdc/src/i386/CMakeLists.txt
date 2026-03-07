file(GLOB SRC *.c *.h *.asm *.s *.S)

target_sources(freestdc PRIVATE ${SRC})

target_compile_definitions(freestdc PRIVATE _FREESTDC_HAVE_NATIVE_memset)
target_compile_definitions(freestdc PRIVATE _FREESTDC_HAVE_NATIVE_memcmp)
