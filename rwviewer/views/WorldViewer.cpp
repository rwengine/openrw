#include "WorldViewer.hpp"
#include "ViewerWidget.hpp"

#include <QFileDialog>

WorldViewer::WorldViewer(ViewerWidget* viewer, QWidget* parent,
                         Qt::WindowFlags f)
    : ViewerInterface(parent, f) {
    mainLayout = new QVBoxLayout;

    viewerWidget = viewer;
    viewerWidget->setMinimumSize(250, 250);

    this->setLayout(mainLayout);
}

void WorldViewer::setViewerWidget(ViewerWidget* widget) {
    viewerWidget = widget;
    // Clear the active model
    widget->showModel(nullptr);
    mainLayout->addWidget(viewerWidget);
}

void WorldViewer::loadPlacements(const QString& file) {
    world()->placeItems(file.toStdString());
    placementsLoaded(file);
}

void WorldViewer::loadPlacements() {
    QFileDialog dialog(this, "Open Placements", "", "Placement (*.ipl)");
    if (dialog.exec()) {
        loadPlacements(dialog.selectedFiles()[0]);
    }
}
