# Warning! This code was automatically generated for projects
# without default 'CMakeLists.txt' file.

set(app_sources
	"Janus_command_handler.cpp"
	"Janus_decoder.cpp"
	"main.cpp"
)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS ".")