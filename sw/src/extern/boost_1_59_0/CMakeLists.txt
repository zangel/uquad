project(boost)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -DNO_BZIP2" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_BZIP2")
add_definitions(-DBOOST_CHRONO_VERSION=2)

include_directories(${PROJECT_SOURCE_DIR})

# Build each of the Boost libraries that have compiled components

file(GLOB lib_srcs ${PROJECT_SOURCE_DIR}/libs/system/src/*.cpp)
add_library(boost_system STATIC ${lib_srcs})

file(GLOB lib_srcs ${PROJECT_SOURCE_DIR}/libs/chrono/src/*.cpp)
add_library(boost_chrono STATIC ${lib_srcs})

add_library(boost_date_time STATIC
	${PROJECT_SOURCE_DIR}/libs/date_time/src/gregorian/date_generators.cpp
    ${PROJECT_SOURCE_DIR}/libs/date_time/src/gregorian/greg_month.cpp
    ${PROJECT_SOURCE_DIR}/libs/date_time/src/gregorian/greg_weekday.cpp
)

add_library(boost_signals STATIC
    ${PROJECT_SOURCE_DIR}/libs/signals/src/connection.cpp
    ${PROJECT_SOURCE_DIR}/libs/signals/src/named_slot_map.cpp
    ${PROJECT_SOURCE_DIR}/libs/signals/src/signal_base.cpp
    ${PROJECT_SOURCE_DIR}/libs/signals/src/slot.cpp
    ${PROJECT_SOURCE_DIR}/libs/signals/src/trackable.cpp
)

add_library(boost_random STATIC
    ${PROJECT_SOURCE_DIR}/libs/random/src/random_device.cpp
)

file(GLOB lib_srcs ${PROJECT_SOURCE_DIR}/libs/regex/src/*.cpp)
add_library(boost_regex STATIC ${lib_srcs})

add_library(boost_iostreams STATIC
    ${PROJECT_SOURCE_DIR}/libs/iostreams/src/file_descriptor.cpp
    ${PROJECT_SOURCE_DIR}/libs/iostreams/src/mapped_file.cpp
)

add_library(boost_filesystem STATIC
    ${PROJECT_SOURCE_DIR}/libs/filesystem/src/codecvt_error_category.cpp
    ${PROJECT_SOURCE_DIR}/libs/filesystem/src/operations.cpp
    ${PROJECT_SOURCE_DIR}/libs/filesystem/src/path_traits.cpp
    ${PROJECT_SOURCE_DIR}/libs/filesystem/src/path.cpp
    ${PROJECT_SOURCE_DIR}/libs/filesystem/src/portability.cpp
    ${PROJECT_SOURCE_DIR}/libs/filesystem/src/unique_path.cpp
    ${PROJECT_SOURCE_DIR}/libs/filesystem/src/utf8_codecvt_facet.cpp
    ${PROJECT_SOURCE_DIR}/libs/filesystem/src/windows_file_codecvt.cpp
)

file(GLOB lib_srcs ${PROJECT_SOURCE_DIR}/libs/program_options/src/*.cpp)
add_library(boost_program_options STATIC ${lib_srcs})

file(GLOB lib_srcs ${PROJECT_SOURCE_DIR}/libs/thread/src/pthread/*.cpp)
add_library( boost_thread STATIC
	${lib_srcs}
    ${PROJECT_SOURCE_DIR}/libs/thread/src/tss_null.cpp
    ${PROJECT_SOURCE_DIR}/libs/thread/src/future.cpp
)

if(UQUAD_PLATFORM_MAC)
	enable_language(ASM)
	
	if(UQUAD_CPU_ARCHITECTURE STREQUAL "i386")
		add_library(boost_context STATIC
			 ${PROJECT_SOURCE_DIR}/libs/context/src/asm/make_i386_sysv_macho_gas.S
			 ${PROJECT_SOURCE_DIR}/libs/context/src/asm/jump_i386_sysv_macho_gas.S
			 ${PROJECT_SOURCE_DIR}/libs/context/src/posix/stack_traits.cpp
			 ${PROJECT_SOURCE_DIR}/libs/context/src/execution_context.cpp
		)
	else()
		add_library(boost_context STATIC
			 ${PROJECT_SOURCE_DIR}/libs/context/src/asm/make_x86_64_sysv_macho_gas.S
			 ${PROJECT_SOURCE_DIR}/libs/context/src/asm/jump_x86_64_sysv_macho_gas.S
			 ${PROJECT_SOURCE_DIR}/libs/context/src/posix/stack_traits.cpp
			 ${PROJECT_SOURCE_DIR}/libs/context/src/execution_context.cpp
		)
	endif()
elseif(UQUAD_PLATFORM_IOS)
	enable_language(ASM)
	add_library(boost_context STATIC
		 ${PROJECT_SOURCE_DIR}/libs/context/src/asm/make_arm64_aapcs_macho_gas.S
		 ${PROJECT_SOURCE_DIR}/libs/context/src/asm/jump_arm64_aapcs_macho_gas.S
		 ${PROJECT_SOURCE_DIR}/libs/context/src/posix/stack_traits.cpp
		 ${PROJECT_SOURCE_DIR}/libs/context/src/execution_context.cpp
	)
elseif(UQUAD_PLATFORM_ANDROID)
	enable_language(ASM)
	add_library(boost_context STATIC
		 ${PROJECT_SOURCE_DIR}/libs/context/src/asm/make_arm_aapcs_elf_gas.S
		 ${PROJECT_SOURCE_DIR}/libs/context/src/asm/jump_arm_aapcs_elf_gas.S
		 ${PROJECT_SOURCE_DIR}/libs/context/src/posix/stack_traits.cpp
		 ${PROJECT_SOURCE_DIR}/libs/context/src/execution_context.cpp
	)
endif()

add_library(boost_coroutine STATIC
	${PROJECT_SOURCE_DIR}/libs/coroutine/src/detail/coroutine_context.cpp
	${PROJECT_SOURCE_DIR}/libs/coroutine/src/exceptions.cpp
	${PROJECT_SOURCE_DIR}/libs/coroutine/src/posix/stack_traits.cpp
)

#set_property(TARGET boost_coroutine APPEND PROPERTY
#    COMPILE_DEFINITIONS "BOOST_USE_SEGMENTED_STACKS"
#)

#set_property(TARGET boost_coroutine APPEND PROPERTY
#    COMPILE_FLAGS "-fsplit-stack"
#)

target_link_libraries(boost_coroutine boost_context)

add_library(boost_serialization STATIC
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_archive.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_iarchive.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_iserializer.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_oarchive.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_oserializer.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_pointer_iserializer.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_pointer_oserializer.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_serializer_map.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_text_iprimitive.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_text_oprimitive.cpp
	${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_xml_archive.cpp
	${PROJECT_SOURCE_DIR}/libs/serialization/src/binary_iarchive.cpp
	${PROJECT_SOURCE_DIR}/libs/serialization/src/binary_oarchive.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/extended_type_info_no_rtti.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/extended_type_info_typeid.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/extended_type_info.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/polymorphic_iarchive.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/polymorphic_oarchive.cpp
	${PROJECT_SOURCE_DIR}/libs/serialization/src/stl_port.cpp
	${PROJECT_SOURCE_DIR}/libs/serialization/src/text_iarchive.cpp
	${PROJECT_SOURCE_DIR}/libs/serialization/src/text_oarchive.cpp
	${PROJECT_SOURCE_DIR}/libs/serialization/src/void_cast.cpp
	${PROJECT_SOURCE_DIR}/libs/serialization/src/archive_exception.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/xml_grammar.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/xml_iarchive.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/xml_oarchive.cpp
	${PROJECT_SOURCE_DIR}/libs/serialization/src/xml_archive_exception.cpp
	
	
	
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_text_wiprimitive.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_text_woprimitive.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/text_wiarchive.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/text_woarchive.cpp
	${PROJECT_SOURCE_DIR}/libs/serialization/src/utf8_codecvt_facet.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/xml_wgrammar.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/xml_wiarchive.cpp
    ${PROJECT_SOURCE_DIR}/libs/serialization/src/xml_woarchive.cpp
	${PROJECT_SOURCE_DIR}/libs/serialization/src/codecvt_null.cpp
	
    #${PROJECT_SOURCE_DIR}/libs/serialization/src/basic_xml_grammar.ipp
    #${PROJECT_SOURCE_DIR}/libs/serialization/src/binary_wiarchive.cpp
    #${PROJECT_SOURCE_DIR}/libs/serialization/src/binary_woarchive.cpp
    #${PROJECT_SOURCE_DIR}/libs/serialization/src/shared_ptr_helper.cpp
    
)

add_library(boost_unit_test_framework STATIC
	${PROJECT_SOURCE_DIR}/libs/test/src/compiler_log_formatter.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/debug.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/decorator.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/execution_monitor.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/framework.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/plain_report_formatter.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/progress_monitor.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/results_collector.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/results_reporter.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/test_tools.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/test_tree.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/unit_test_log.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/unit_test_main.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/unit_test_monitor.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/unit_test_parameters.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/xml_log_formatter.cpp
	${PROJECT_SOURCE_DIR}/libs/test/src/xml_report_formatter.cpp
)

if(UQUAD_PLATFORM_ANDROID)
	add_library(boost_log STATIC
		${PROJECT_SOURCE_DIR}/libs/log/src/attribute_name.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/attribute_set.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/attribute_value_set.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/code_conversion.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/core.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/record_ostream.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/severity_level.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/global_logger_storage.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/named_scope.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/process_name.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/process_id.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/thread_id.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/timer.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/exceptions.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/default_attribute_names.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/default_sink.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/text_ostream_backend.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/text_file_backend.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/text_multifile_backend.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/syslog_backend.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/thread_specific.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/once_block.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/timestamp.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/threadsafe_queue.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/event.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/trivial.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/spirit_encoding.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/format_parser.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/date_time_format_parser.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/named_scope_format_parser.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/unhandled_exception_count.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/dump.cpp
	)
else()
	add_library(boost_log STATIC
		${PROJECT_SOURCE_DIR}/libs/log/src/attribute_name.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/attribute_set.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/attribute_value_set.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/code_conversion.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/core.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/record_ostream.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/severity_level.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/global_logger_storage.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/named_scope.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/process_name.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/process_id.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/thread_id.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/timer.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/exceptions.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/default_attribute_names.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/default_sink.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/text_ostream_backend.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/text_file_backend.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/text_multifile_backend.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/syslog_backend.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/thread_specific.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/once_block.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/timestamp.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/threadsafe_queue.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/event.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/trivial.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/spirit_encoding.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/format_parser.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/date_time_format_parser.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/named_scope_format_parser.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/unhandled_exception_count.cpp
		${PROJECT_SOURCE_DIR}/libs/log/src/dump.cpp
	)
endif()

set_property(TARGET boost_log APPEND PROPERTY
    COMPILE_DEFINITIONS "BOOST_LOG_BUILDING_THE_LIB=1"
)

# enable boost.thread tss_cleanup implementation 
set_property(TARGET boost_thread APPEND PROPERTY
    COMPILE_DEFINITIONS "BOOST_THREAD_BUILD_LIB=1"
)
