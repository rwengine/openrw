#include "WorldViewer.hpp"
#include "ViewerWidget.hpp"

#include <QFileDialog>

WorldViewer::WorldViewer(QWidget* parent, Qt::WindowFlags f)
    : ViewerInterface(parent, f) {
    mainLayout = new QVBoxLayout;

    mainLayout->addWidget(QWidget::createWindowContainer(createViewer()));

    setLayout(mainLayout);
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
