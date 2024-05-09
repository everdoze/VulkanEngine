#include "string.hpp"

#include "platform/platform.hpp"

namespace Engine {
    
    b8 Parse(std::string& str, glm::vec4* out_vector) {
        if (!str.length()) {
            return false;
        }

        Platform::ZMemory(out_vector, sizeof(glm::vec4));
        i32 result = sscanf(str.c_str(), "%f %f %f %f", &out_vector->x, &out_vector->y, &out_vector->z, &out_vector->w);
        return result != -1;
    };

    b8 Parse(std::string& str, glm::vec3* out_vector) {
        if (!str.length()) {
            return false;
        }

        Platform::ZMemory(out_vector, sizeof(glm::vec3));
        i32 result = sscanf(str.c_str(), "%f %f %f", &out_vector->x, &out_vector->y, &out_vector->z);
        return result != -1;
    };

    b8 Parse(std::string& str, glm::vec2* out_vector) {
        if (!str.length()) {
            return false;
        }

        Platform::ZMemory(out_vector, sizeof(glm::vec2));
        i32 result = sscanf(str.c_str(), "%f %f", &out_vector->x, &out_vector->y);
        return result != -1;
    };

    b8 Parse(std::string& str, f32* out_float) {
        if (!str.length()) {
            return false;
        }

        *out_float = 0;
        i32 result = sscanf(str.c_str(), "%f", out_float);
        return result != -1;
    };

    b8 Parse(std::string& str, f64* out_float) {
        if (!str.length()) {
            return false;
        }

        *out_float = 0;
        i32 result = sscanf(str.c_str(), "%lf", out_float);
        return result != -1;
    };

    b8 Parse(std::string& str, i64* out_int) {
        if (!str.length()) {
            return false;
        }

        *out_int = 0;
        i32 result = sscanf(str.c_str(), "%lli", out_int);
        return result != -1;
    };

    b8 Parse(std::string& str, i32* out_int) {
        if (!str.length()) {
            return false;
        }

        *out_int = 0;
        i32 result = sscanf(str.c_str(), "%i", out_int);
        return result != -1;
    };

    b8 Parse(std::string& str, i16* out_int) {
        if (!str.length()) {
            return false;
        }

        *out_int = 0;
        i32 result = sscanf(str.c_str(), "%hi", out_int);
        return result != -1;
    };

    b8 Parse(std::string& str, i8* out_int) {
        if (!str.length()) {
            return false;
        }

        *out_int = 0;
        i32 result = sscanf(str.c_str(), "%hhi", out_int);
        return result != -1;
    };

    b8 Parse(std::string& str, u64* out_unsigned_int) {
        if (!str.length()) {
            return false;
        }

        *out_unsigned_int = 0;
        i32 result = sscanf(str.c_str(), "%llu", out_unsigned_int);
        return result != -1;
    };

    b8 Parse(std::string& str, u32* out_unsigned_int) {
        if (!str.length()) {
            return false;
        }

        *out_unsigned_int = 0;
        i32 result = sscanf(str.c_str(), "%u", out_unsigned_int);
        return result != -1;
    };

    b8 Parse(std::string& str, u16* out_unsigned_int) {
        if (!str.length()) {
            return false;
        }

        *out_unsigned_int = 0;
        i32 result = sscanf(str.c_str(), "%hu", out_unsigned_int);
        return result != -1;
    };

    b8 Parse(std::string& str, u8* out_unsigned_int) {
        if (!str.length()) {
            return false;
        }

        *out_unsigned_int = 0;
        i32 result = sscanf(str.c_str(), "%hhu", out_unsigned_int);
        return result != -1;
    };

    b8 Parse(std::string& str, b8* out_bool) {
        if (!str.length()) {
            return false;
        }
        std::string buffer = str;
        std::transform(buffer.begin(), buffer.end(), buffer.begin(), [](u8 c){ return std::tolower(c); });
        if ((str == "1" || buffer == "true")) {
            *out_bool = true;
            return true;
        }
        *out_bool = false;
        return true;
    };

    std::string& RTrim(std::string& str, const char* t) {
        str.erase(str.find_last_not_of(t) + 1);
        return str;
    };

    std::string& LTrim(std::string& str, const char* t) {
        str.erase(0, str.find_first_not_of(t));
        return str;
    };

    std::string& Trim(std::string& str, const char* t) {
        return LTrim(RTrim(str, t), t);
    };

    std::pair<std::string, std::string> MidString(std::string& str, const char& separator) {
        std::pair<std::string, std::string> split_string;
        u32 pos = 0; pos = str.find(separator);
        if (pos) {
            split_string.first = str.substr(0, pos);
            split_string.second = str.substr(pos+1, str.length() - (pos+1));
        }
        return split_string;
    };

    std::vector<std::string> SplitString(std::string& str, const char& delimiter) {
        std::vector<std::string> strings;
        u32 new_string_offset = 0;
        for (u32 i = 0; i < str.size(); ++i) {
            char c = str[i];

            if (c == delimiter) {
                std::string substring = str.substr(new_string_offset, i-1);
                if (substring.size()) {
                    strings.push_back(substring);
                    new_string_offset = i + 1;
                }
                continue;
            }

            if (i+1 == str.size()) {
                std::string substring = str.substr(new_string_offset, i);
                strings.push_back(substring);
                continue;
            }
            
        } 
        return strings;
    };

    b8 ICharEquals(char a, char b) {
        return std::tolower(static_cast<u8>(a)) == std::tolower(static_cast<u8>(b));
    };

    b8 StringIEquals(const std::string& a, const std::string& b) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end(), ICharEquals);
    };

    std::string GetFullDirectoryFromPath(const std::string& path) {
        for (u32 i = path.size(); i >= 0; --i) {
            if (path[i] == '/' || path[i] == '\\') {
                return path.substr(0, i+1);
            }
        }
        return "";
    };
    
    std::string GetFilenameFromPath(const std::string& path) {
        for (u32 i = path.size(); i >= 0; --i) {
            if (path[i] == '/' || path[i] == '\\') {
                return path.substr(i+1, path.size() - i);
            }
        }
        return "";
    };

    std::string GetStripFilenameFromPath(const std::string& path) {
        u32 filename_start = 0;
        u32 size = 0;
        b8 found = false;
        for (u32 i = path.size(); i >= 0; --i) {
            if (path[i] == '.') {
                found = true;
            }
            if (path[i] == '/' || path[i] == '\\') {
                filename_start = i;
                break;
            }
            if (found) {
                size++;
            }
        }
        if (found) {
            return path.substr(filename_start, size);
        }
        return "";
    };

};
