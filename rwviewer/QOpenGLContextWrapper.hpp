#ifndef _RWVIEWER_QOPENGLWRAPPER_HPP
#define _RWVIEWER_QOPENGLWRAPPER_HPP

#include <QSurfaceFormat>

class QOpenGLContext;
class QSurface;
class QWidget;

class QOpenGLContextWrapper {
public:
    QOpenGLContextWrapper(QWidget *parent);
    void setup(const QSurfaceFormat &format);
    bool create();

    bool makeCurrent(QSurface *surface);
    void doneCurrent();
    void swapBuffers(QSurface *surface);

    QOpenGLContext *get() {
        return m_context;
    }
private:
    QOpenGLContext *m_context = nullptr;
    QWidget *m_parent;
};

#endif
