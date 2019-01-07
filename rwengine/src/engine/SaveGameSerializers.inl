#ifndef _RWENGINE_SAVEGAMESERIALIZERS_HPP_
#define _RWENGINE_SAVEGAMESERIALIZERS_HPP_
#include <engine/GameState.hpp>

namespace SaveGameSerialization {

template <class Tserializer, class T, typename std::enable_if<
          std::is_trivial_v<T>,int>::type = 0>
bool serialize(Tserializer& serializer, T& value) {
    return serializer.serialize(value);
}

template <class Tserializer, class Ttype, size_t N>
bool serialize(Tserializer& serializer, Ttype (&value)[N]) {
    return serializer.serialize(value);
}

template <class Tserializer>
bool serialize(Tserializer& serializer, BasicState& value) {
    return serializer.serialize(value);
}

template <class Tserializer, class T>
bool serialize(Tserializer& serializer, std::vector<T>& value) {
    bool result = true;
    for (auto i = 0u; i < value.size(); ++i) {
       result &= serialize(serializer, value[i]);
    }
    return result;
}

} // namespace SaveGameSerialization

#endif //_RWENGINE_SAVEGAMESERIALIZERS_HPP_
