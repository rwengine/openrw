#include "data/Clump.hpp"
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

Geometry::Geometry() : flags(0) {
}

Geometry::~Geometry() {
}

ModelFrame::ModelFrame(unsigned int index, glm::mat3 dR, glm::vec3 dT)
    : index(index)
    , defaultRotation(dR)
    , defaultTranslation(dT)
    , parent_(nullptr) {
    reset();
}

void ModelFrame::reset() {
    matrix = glm::translate(glm::mat4(), defaultTranslation) *
             glm::mat4(defaultRotation);
    updateHierarchyTransform();
}

void ModelFrame::updateHierarchyTransform() {
    // Update our own transformation
    if (parent_) {
        worldtransform_ = parent_->getWorldTransform() * matrix;
    } else {
        worldtransform_ = matrix;
    }
    for (const auto& child : children_) {
        child->updateHierarchyTransform();
    }
}

void ModelFrame::addChild(ModelFramePtr& child) {
    // Make sure the child is an orphan
    if (child->getParent()) {
        auto& other_children = child->getParent()->children_;
        other_children.erase(
            std::remove(other_children.begin(), other_children.end(), child),
            other_children.end());
    }
    child->parent_ = this;
    children_.push_back(child);
}

ModelFrame* ModelFrame::findDescendant(const std::string& name) const {
    for (const auto& frame : children_) {
        if (frame->getName() == name) {
            return frame.get();
        }

        auto result = frame->findDescendant(name);
        if (result) {
            return result;
        }
    }

    return nullptr;
}

ModelFrame* Clump::findFrame(const std::string& name) const {
    if (rootframe_->getName() == name) {
        return rootframe_.get();
    }

    return rootframe_->findDescendant(name);
}

Clump::~Clump() {
}

void Clump::recalculateMetrics() {
    boundingRadius = std::numeric_limits<float>::min();
    for (const auto& atomic : atomics_) {
        const auto& geometry = atomic->getGeometry();
        if (!geometry) {
            continue;
        }
        const auto& bounds = geometry->geometryBounds;
        boundingRadius = std::max(boundingRadius,
                                  glm::length(bounds.center) + bounds.radius);
    }
}
