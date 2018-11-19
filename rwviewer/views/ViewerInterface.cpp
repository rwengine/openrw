#include "ViewerInterface.hpp"

#include "ViewerWindow.hpp"

ViewerWidget* ViewerInterface::createViewer() {
    return static_cast<ViewerWindow*>(window())->createViewer();
}
