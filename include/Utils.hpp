#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <random>
#include <string>
#include <chrono>
#include <format>

namespace Utils {

    static std::string generate_alphanumeric(size_t length) {
        const std::string alphabet = 
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        
        static std::random_device rd; 
        static std::mt19937 gen(rd());
        
        std::uniform_int_distribution<> dist(0, static_cast<int>(alphabet.size() - 1));
        
        std::string result;
        result.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            result += alphabet[dist(gen)];
        }

        return result;
    }
    
    static std::chrono::system_clock::time_point get_current_time() {
        return std::chrono::system_clock::now();
    }

    static std::string get_formatted_date_time_from_utc(std::chrono::system_clock::time_point time) {
        return std::format("{:%Y-%m-%d %H:%M:%S}", time);
    }

    static std::string get_db_iso_string() {
        return std::format("{:%FT%TZ}", std::chrono::system_clock::now());
    }
}

#endif 