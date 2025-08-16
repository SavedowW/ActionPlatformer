#ifndef SDL_UTILS_H_
#define SDL_UTILS_H_
#include <SDL3/SDL.h>
#include <string>
#include <sstream>

namespace utils {

    template<bool REMOVE_HASHTAG>
    inline SDL_Color hexToCol(const std::string &s_)
    {
        int offset = (REMOVE_HASHTAG ? 1 : 0);

        SDL_Color col = {255, 255, 255, 255};
        unsigned tmp = 0;

        std::istringstream(s_.substr(0 + offset, 2)) >> std::hex >> tmp;
        col.r = tmp;

        std::istringstream(s_.substr(2 + offset, 2)) >> std::hex >> tmp;
        col.g = tmp;

        std::istringstream(s_.substr(4 + offset, 2)) >> std::hex >> tmp;
        col.b = tmp;

        return col;
    }

}

#endif
