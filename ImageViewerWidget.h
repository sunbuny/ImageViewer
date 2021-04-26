#pragma once

#include <memory>
#include <string>

#include <QImage>
#include <QWidget>


// Type definitions which are more concise and thus easier to read and write (no
// underscore). int is used as-is.
typedef size_t usize;
typedef int64_t i64;
typedef uint64_t u64;
typedef int32_t i32;
typedef uint32_t u32;
typedef int16_t i16;
typedef uint16_t u16;
typedef int8_t i8;
typedef uint8_t u8;

// Qt widget for image display.
class ImageViewerWidget : public QWidget {
Q_OBJECT
public:
    explicit ImageViewerWidget(QWidget *parent = nullptr);

    ~ImageViewerWidget() override;

#if 1
    // Sets the image displayed by the widget.
    void SetImage(const QImage& image) {
        bool no_image_before = qimage_.isNull();
        qimage_ = image;
        UpdateViewTransforms();
        if (no_image_before && !qimage_.isNull()) {
            updateGeometry();
        }
        update(rect());
    }

    void SetViewOffset(double x, double y);
    void SetZoomFactor(double zoom_factor);
    void ZoomAt(int x, int y, double target_zoom);
    void FitContent(bool update_display = true);

    void AddSubpixelDotPixelCornerConv(float x, float y, u8 r, u8 g, u8 b);
    void AddSubpixelLinePixelCornerConv(float x0, float y0, float x1, float y1, u8 r, u8 g, u8 b);
    void AddSubpixelTextPixelCornerConv(float x, float y, u8 r, u8 g, u8 b, const std::string& text);
    void Clear();


    virtual QSize sizeHint() const override;

    inline double zoom_factor() const { return view_scale_; }
    inline const QImage& image() const { return qimage_; }

    inline const QTransform& image_to_viewport() const { return image_to_viewport_; }
    inline const QTransform& viewport_to_image() const { return viewport_to_image_; }

    signals:
    // (0, 0) is at the top-left corner of the image here.
    void CursorPositionChanged(QPointF pos, bool pixel_value_valid, std::string pixel_value, QRgb pixel_displayed_value);

    void ZoomChanged(double zoom);

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
//    virtual void keyPressEvent(QKeyEvent* event) override;
//    virtual void keyReleaseEvent(QKeyEvent* event) override;

private:
    struct SubpixelDot {
        QPointF xy;
        QRgb rgb;
    };

    struct SubpixelLine {
        QPointF xy0;
        QPointF xy1;
        QRgb rgb;
    };

    struct SubpixelText {
        QPointF xy;
        QRgb rgb;
        QString text;
    };

    void UpdateViewTransforms();
    QPointF ViewportToImage(const QPointF& pos);
    QPointF ImageToViewport(const QPointF& pos);

    void startDragging(QPoint pos);
    void updateDragging(QPoint pos);
    void finishDragging(QPoint pos);

    // Transformations between viewport coordinates and image coordinates.
    // (0, 0) is at the top-left corner of the image here.
    QTransform image_to_viewport_;
    QTransform viewport_to_image_;

    // Mouse dragging handling.
    bool dragging_;
    QPoint drag_start_pos_;
    QCursor normal_cursor_;

    // View settings.
    double view_scale_;
    double view_offset_x_;
    double view_offset_y_;

    SubpixelLine horizontal_;
    SubpixelLine vertical_;
    std::vector<SubpixelDot> dots_;
    std::vector<SubpixelLine> lines_;
    std::vector<SubpixelText> texts_;

    QImage qimage_;
#endif
};


