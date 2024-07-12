#ifndef DECORATION_BUILDER_H_
#define DECORATION_BUILDER_H_
#include <nlohmann/json.hpp>
#include "Application.h"
#include "DecorLayers.h"

class DecorationBuilder
{
public:
    DecorationBuilder(Application &app_);
    DecorLayers buildDecorLayers(const std::string &mapDescr_, Tileset &usedTileset_);


private:
    Application &m_app;
    const std::string m_root;

};

#endif
