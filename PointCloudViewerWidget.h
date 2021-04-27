#pragma once
#include <QOpenGLWidget>

class PointCloudViewerWidget : public QOpenGLWidget {
    Q_OBJECT
public:
    PointCloudViewerWidget(QWidget* parent);
    ~PointCloudViewerWidget();

protected:
    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int width, int height) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;

    bool initialized_ = false;
    shared_ptr<RenderWindowCallbacks> callbacks_;
};
