#include "ImageViewerWidget.h"

#include <cmath>
#include <QPainter>
#include <QPaintEvent>
#include <sstream>

#include <QDebug>


ImageViewerWidget::ImageViewerWidget(QWidget *parent)
        : QWidget(parent) {
    dragging_ = false;

    view_scale_ = 1.0;
    view_offset_x_ = 0.0;
    view_offset_y_ = 0.0;

    horizontal_.xy0 = QPointF();
    horizontal_.xy1 = QPointF();
    horizontal_.rgb = qRgb(0,255,0);
    vertical_.xy0 = QPointF();
    vertical_.xy1 = QPointF();
    vertical_.rgb = qRgb(0,255,0);

    qimage_ = QImage();
    UpdateViewTransforms();

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(false);
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
}

ImageViewerWidget::~ImageViewerWidget() {}

#if 1

void ImageViewerWidget::SetViewOffset(double x, double y) {
    view_offset_x_ = x;
    view_offset_y_ = y;
    UpdateViewTransforms();
    update(rect());
}

void ImageViewerWidget::SetZoomFactor(double zoom_factor) {
    view_scale_ = zoom_factor;
    emit ZoomChanged(view_scale_);
    UpdateViewTransforms();
    update(rect());
}

void ImageViewerWidget::ZoomAt(int x, int y, double target_zoom) {
    // viewport_to_image_.m11() * pos.x() + viewport_to_image_.m13() == (pos.x() - (new_view_offset_x_ + (0.5 * width()) - (0.5 * image_.width()) * new_view_scale_)) / new_view_scale_;
    QPointF center_on_image = ViewportToImage(QPoint(x, y));
    view_offset_x_ =
            x - (0.5 * width() - (0.5 * qimage_.width()) * target_zoom) - target_zoom * center_on_image.x();
    view_offset_y_ =
            y - (0.5 * height() - (0.5 * qimage_.height()) * target_zoom) - target_zoom * center_on_image.y();
    SetZoomFactor(target_zoom);
}

void ImageViewerWidget::FitContent(bool update_display) {
    if (qimage_.isNull()) {
        return;
    }

    // Center image
    view_offset_x_ = 0.0;
    view_offset_y_ = 0.0;

    // Scale image such that it exactly fills the widget
    view_scale_ = std::min(width() / (1. * qimage_.width()),
                           height() / (1. * qimage_.height()));
    emit ZoomChanged(view_scale_);

    if (update_display) {
        UpdateViewTransforms();
        update(rect());
    }
}

void ImageViewerWidget::AddSubpixelDotPixelCornerConv(float x, float y, u8 r, u8 g, u8 b) {
    dots_.emplace_back();
    SubpixelDot *new_dot = &dots_.back();
    new_dot->xy = QPointF(x, y);
    new_dot->rgb = qRgb(r, g, b);
}

void ImageViewerWidget::AddSubpixelLinePixelCornerConv(float x0, float y0, float x1, float y1, u8 r, u8 g, u8 b) {
    lines_.emplace_back();
    SubpixelLine *new_line = &lines_.back();
    new_line->xy0 = QPointF(x0, y0);
    new_line->xy1 = QPointF(x1, y1);
    new_line->rgb = qRgb(r, g, b);
}

void
ImageViewerWidget::AddSubpixelTextPixelCornerConv(float x, float y, u8 r, u8 g, u8 b, const std::string &text) {
    texts_.emplace_back();
    SubpixelText *new_text = &texts_.back();
    new_text->xy = QPointF(x, y);
    new_text->rgb = qRgb(r, g, b);
    new_text->text = QString::fromStdString(text);
}

void ImageViewerWidget::Clear() {
    dots_.clear();
    lines_.clear();
    texts_.clear();
}

QSize ImageViewerWidget::sizeHint() const {
    if (qimage_.isNull()) {
        return QSize(150, 150);
    } else {
        return qimage_.size();
    }
}

void ImageViewerWidget::resizeEvent(QResizeEvent *event) {
    UpdateViewTransforms();
    QWidget::resizeEvent(event);
    FitContent(true);
}

void ImageViewerWidget::paintEvent(QPaintEvent *event) {
    // Create painter and set its options.
    QPainter painter(this);
    QRect event_rect = event->rect();
    painter.setClipRect(event_rect);

    painter.fillRect(event_rect, QColor(Qt::gray));

    if (qimage_.isNull()) {
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);

    QTransform image_to_viewport_T = image_to_viewport_.transposed();
    painter.setTransform(image_to_viewport_T);

    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawImage(QPointF(0, 0), qimage_);

    painter.resetTransform();

    if (!dots_.empty()) {
        painter.setBrush(Qt::NoBrush);

        for (const SubpixelDot &dot : dots_) {
            QPointF viewport_position = image_to_viewport_T.map(dot.xy);

            painter.setPen(dot.rgb);
            painter.drawEllipse(viewport_position, 2, 2);
        }
    }

    if (!lines_.empty()) {
        painter.setBrush(Qt::NoBrush);

        for (const SubpixelLine &line : lines_) {
            QPointF viewport_position_0 = image_to_viewport_T.map(line.xy0);
            QPointF viewport_position_1 = image_to_viewport_T.map(line.xy1);

            painter.setPen(line.rgb);
            painter.drawLine(viewport_position_0, viewport_position_1);
        }
    }

    if (!texts_.empty()) {
        painter.setBrush(Qt::NoBrush);

        for (const SubpixelText &text : texts_) {
            QPointF viewport_position = image_to_viewport_T.map(text.xy);

            painter.setPen(text.rgb);
            painter.drawText(viewport_position, text.text);
        }
    }

    painter.setBrush(Qt::NoBrush);
    painter.setPen(horizontal_.rgb);
    painter.drawLine(horizontal_.xy0, horizontal_.xy1);
    painter.setPen(vertical_.rgb);
    painter.drawLine(vertical_.xy0, vertical_.xy1);

    painter.end();
}

void ImageViewerWidget::mousePressEvent(QMouseEvent *event) {
   if (dragging_) {
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton) {

        startDragging(event->pos());
        event->accept();
    }
}


void ImageViewerWidget::mouseMoveEvent(QMouseEvent *event) {
    float x = event->x();
    float y = event->y();
    horizontal_.xy0 = QPointF(0, y);
    horizontal_.xy1 = QPointF(width(), y);
    vertical_.xy0 = QPointF(x, 0);
    vertical_.xy1 = QPointF(x, height());
    QPointF image_pos = ViewportToImage(event->localPos());
    std::string pixel_value;
    QRgb pixel_displayed_value = qRgb(0, 0, 0);
    bool pixel_value_valid =
            !qimage_.isNull() && image_pos.x() >= 0 && image_pos.y() >= 0 &&
            image_pos.x() < qimage_.width() && image_pos.y() < qimage_.height();
    if (pixel_value_valid) {
        int x = image_pos.x();
        int y = image_pos.y();

        std::stringstream o;
        auto rgb= qimage_.pixel(x,y);
        o << "R:" << qRed(rgb) << ",G:" << qGreen(rgb) << ",B:" << qBlue(rgb);
        pixel_value = o.str();
        qDebug() << QString::fromStdString(pixel_value);
        pixel_displayed_value = qimage_.pixel(x, y);
    }
    emit CursorPositionChanged(image_pos, pixel_value_valid, pixel_value, pixel_displayed_value);

    if (dragging_) {
        updateDragging(event->pos());
        return;
    }
    update(rect());
}

void ImageViewerWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (dragging_) {
        finishDragging(event->pos());
        event->accept();
        return;
    }


    if (event->button() == Qt::LeftButton) {

    } else if (event->button() == Qt::MiddleButton) {

    } else if (event->button() == Qt::RightButton) {

    } else {
        return;
    }

    event->accept();
}

void ImageViewerWidget::wheelEvent(QWheelEvent *event) {
    double degrees = event->angleDelta().y() / 8.0;
    double num_steps = degrees / 15.0;

    double scale_factor = pow(sqrt(2.0), num_steps);
    double target_scale = view_scale_ * scale_factor;

    ZoomAt(event->position().x(), event->position().y(), target_scale);

    event->ignore();

}


void ImageViewerWidget::UpdateViewTransforms() {
    image_to_viewport_.setMatrix(
            view_scale_, 0, view_offset_x_ + (0.5 * width()) - (0.5 * qimage_.width()) * view_scale_,
            0, view_scale_, view_offset_y_ + (0.5 * height()) - (0.5 * qimage_.height()) * view_scale_,
            0, 0, 1);
    viewport_to_image_ = image_to_viewport_.inverted();
}

QPointF ImageViewerWidget::ViewportToImage(const QPointF &pos) {
    return QPointF(
            viewport_to_image_.m11() * pos.x() + viewport_to_image_.m12() * pos.y() + viewport_to_image_.m13(),
            viewport_to_image_.m21() * pos.x() + viewport_to_image_.m22() * pos.y() + viewport_to_image_.m23());
}

QPointF ImageViewerWidget::ImageToViewport(const QPointF &pos) {
    return QPointF(
            image_to_viewport_.m11() * pos.x() + image_to_viewport_.m12() * pos.y() + image_to_viewport_.m13(),
            image_to_viewport_.m21() * pos.x() + image_to_viewport_.m22() * pos.y() + image_to_viewport_.m23());
}

void ImageViewerWidget::startDragging(QPoint pos) {
//   Q_ASSERT(!dragging);
    dragging_ = true;
    drag_start_pos_ = pos;
    normal_cursor_ = cursor();
    setCursor(Qt::ClosedHandCursor);
}

void ImageViewerWidget::updateDragging(QPoint pos) {
//   Q_ASSERT(dragging);
    QPoint offset = pos - drag_start_pos_;
    SetViewOffset(view_offset_x_ + offset.x(),
                  view_offset_y_ + offset.y());

    drag_start_pos_ = pos;
}

void ImageViewerWidget::finishDragging(QPoint pos) {
    updateDragging(pos);

    dragging_ = false;
    setCursor(normal_cursor_);
}

#endif
