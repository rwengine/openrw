#include "QOpenGLContextWrapper.hpp"

#include <QApplication>
#include <QMessageBox>
#include <QOpenGLContext>

QOpenGLContextWrapper::QOpenGLContextWrapper(QWidget* parent)
    : m_parent(parent) {
}

void QOpenGLContextWrapper::setup(QSurfaceFormat format) {
    m_context = new QOpenGLContext(m_parent);
    m_context->setShareContext(QOpenGLContext::globalShareContext());
    m_context->setFormat(format);
}

bool QOpenGLContextWrapper::create() {
    if (!m_context->create()) {
        QMessageBox::critical(m_parent, "OpenGL Failure",
                              "Failed to create OpenGL context");
        QApplication::exit(1);
        return false;
    }
    return true;
}

bool QOpenGLContextWrapper::makeCurrent(QSurface* surface) {
    return m_context->makeCurrent(surface);
}

void QOpenGLContextWrapper::doneCurrent() {
    m_context->doneCurrent();
}

void QOpenGLContextWrapper::swapBuffers(QSurface* surface) {
    m_context->swapBuffers(surface);
}
