#include <script/ScriptFunctions.hpp>

static const char* sprite_names[] = {
	"", // 0
	"radar_asuka",
	"radar_bomb",
	"radar_cat",
	"radar_centre",
	"radar_copcar",
	"radar_don",
	"radar_eight",
	"radar_el",
	"radar_ice",
	"radar_joey",
	"radar_kenji",
	"radar_liz",
	"radar_luigi",
	"radar_north",
	"radar_ray",
	"radar_sal",
	"radar_save",
	"radar_spray",
	"radar_tony",
	"radar_weapon",
};

const char*script::getBlipSprite(ScriptRadarSprite sprite)
{
	return sprite_names[sprite];
}
