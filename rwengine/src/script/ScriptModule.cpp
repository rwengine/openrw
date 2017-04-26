#include <script/ScriptMachine.hpp>
#include <script/ScriptModule.hpp>

bool ScriptModule::findOpcode(ScriptFunctionID id, ScriptFunctionMeta** out) {
    auto it = functions.find(id);
    if (it == functions.end()) {
        return false;
    }
    *out = &functions[id];
    return true;
}
