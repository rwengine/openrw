#include "data/ModelData.hpp"

void SimpleModelInfo::setupBigBuilding(const ModelInfoTable& models) {
    if (loddistances_[0] > 300.f && atomics_[2] == nullptr) {
        isbigbuilding_ = true;
        findRelatedModel(models);
        if (related_) {
            loddistances_[2] = related_->getLargestLodDistance();
        } else {
            loddistances_[2] = 100.f;
        }
    }
}

void SimpleModelInfo::findRelatedModel(const ModelInfoTable& models) {
    for (const auto& model : models) {
        if (model.second.get() == this) continue;
        const auto& othername = model.second->name;
        if (othername.size() <= 3 || name.size() <= 3) continue;
        if (othername.substr(3) == name.substr(3)) {
            related_ = static_cast<SimpleModelInfo*>(model.second.get());
            break;
        }
    }
}
