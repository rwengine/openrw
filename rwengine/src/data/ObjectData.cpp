#include "data/ObjectData.hpp"

const ObjectInformation::ObjectClass ObjectData::class_id =
    ObjectInformation::_class("OBJS");
const ObjectInformation::ObjectClass VehicleData::class_id =
    ObjectInformation::_class("CARS");
const ObjectInformation::ObjectClass CharacterData::class_id =
    ObjectInformation::_class("PEDS");
const ObjectInformation::ObjectClass CutsceneObjectData::class_id =
    ObjectInformation::_class("HIER");
