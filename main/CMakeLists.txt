idf_component_register(SRCS alles_esp32.c
							buttons.c
							power.c
)

set_source_files_properties(alles_esp32.c
    PROPERTIES COMPILE_FLAGS
    -Wno-strict-aliasing
)

