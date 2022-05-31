//
// Created by dbouget on 06.10.2021.
//

#ifndef FASTPATHOLOGY_UTILITIES_H
#define FASTPATHOLOGY_UTILITIES_H

#include <iostream>
#include <vector>
#include <string>
#include <time.h>

namespace fast {
    /**
     * Creates a string of random numbers of length n.
     * @param n
     * @return
     */
    static std::string createRandomNumbers_(int n) {
        srand(time(NULL));
        std::string out;
        for (int i = 0; i < n; i++) {
            out.append(std::to_string(rand() % 10));
        }
        return out;
    }

    // for string delimiter
    static std::vector<std::string> splitCustom(const std::string& s, const std::string& delimiter) {
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        std::string token;
        std::vector<std::string> res;

        while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
            token = s.substr (pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back (token);
        }

        res.push_back (s.substr (pos_start));
        return res;
    }
}
#endif //FASTPATHOLOGY_UTILITIES_H
