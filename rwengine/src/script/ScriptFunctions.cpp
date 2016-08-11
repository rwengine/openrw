#include <script/ScriptFunctions.hpp>
#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>

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

ScriptModel script::getModel(const ScriptArguments& args, ScriptModel model)
{
	if (model < 0) {
		/// @todo verify that this is how the game uses negative models
		const auto& m = args.getVM()->getFile()->getModels()[-model];
		return args.getWorld()->data->findModelObject(m);
	}
	return model;
}
