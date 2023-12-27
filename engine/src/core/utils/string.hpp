#pragma once

#include "defines.hpp"

namespace Engine {

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wformat"
    template<typename ... Args>
    std::string StringFormat( const std::string& format, Args ... args ) {
        int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
        if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
        auto size = static_cast<size_t>( size_s );
        std::unique_ptr<char[]> buf( new char[ size ] );
        std::snprintf( buf.get(), size, format.c_str(), args ... );
        return std::string( buf.get(), buf.get() + size - 1 );
    }
    #pragma clang diagnostic pop

    b8 Parse(std::string& str, glm::vec4* out_vector);
    b8 Parse(std::string& str, glm::vec3* out_vector);
    b8 Parse(std::string& str, glm::vec2* out_vector);
    b8 Parse(std::string& str, f32* out_float);
    b8 Parse(std::string& str, f64* out_float);
    b8 Parse(std::string& str, i64* out_int);
    b8 Parse(std::string& str, i32* out_int);
    b8 Parse(std::string& str, i16* out_int);
    b8 Parse(std::string& str, i8* out_int);
    b8 Parse(std::string& str, u64* out_unsigned_int);
    b8 Parse(std::string& str, u32* out_unsigned_int);
    b8 Parse(std::string& str, u16* out_unsigned_int);
    b8 Parse(std::string& str, u8* out_unsigned_int);
    b8 Parse(std::string& str, b8* out_bool);

    #define DEFAULT_TRIM_VALUE " \t\n\r\f\v"

    std::string& RTrim(std::string& str, const char* t = DEFAULT_TRIM_VALUE);
    std::string& LTrim(std::string& str, const char* t = DEFAULT_TRIM_VALUE);
    std::string& Trim(std::string& str, const char* t = DEFAULT_TRIM_VALUE);

    std::pair<std::string, std::string> MidString(std::string& str, const char& separator);
    std::vector<std::string> SplitString(std::string& str, const char& delimiter);
};

