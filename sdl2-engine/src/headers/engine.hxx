/*
 * engine.hxx
 *
 *  Created on: 24 нояб. 2017 г.
 *      Author: Shaft
 */

#ifndef HEADERS_ENGINE_HXX_
#define HEADERS_ENGINE_HXX_

#include <string>

#include "SDL2/SDL.h"

std::ostream& operator<<(std::ostream& out, const SDL_version& v);

namespace CHL    // chlorine-5
{
class engine {
   public:
    engine();
    virtual ~engine();

    int CHL_Init(std::string) = 0;

    void read_input() = 0;
    void CHL_Exit() = 0;

   private:
    SDL_Window* const window = nullptr;
};
}    // namespace CHL

#endif /* HEADERS_ENGINE_HXX_ */
