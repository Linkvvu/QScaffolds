#include "FramelessWin.h"

#include <QtCore/QDebug>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QStyleOption>

FramelessWin::FramelessWin(QWidget* parent)
    : QWidget(parent)
{
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
}

void FramelessWin::paintEvent(QPaintEvent*)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void FramelessWin::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    isMosPressed_ = true;
    prevMosPos_   = event->globalPos();
    initializeOperation(event->globalPos());
  }
  return QWidget::mousePressEvent(event);
}

void FramelessWin::initializeOperation(const QPoint& globalPos)
{
  if (!isResized_ && !isDragging_) {
    curOpMode_ = OperationMode::None;
    return;
  }

  if (isResized_) {
    curResizeDirection_ = getResizeDirection(globalPos);
    if (curResizeDirection_ != ResizeDirection::None) {
      curOpMode_ = OperationMode::Resize;
      return;
    }
  }

  if (isDragging_) {
    curOpMode_ = OperationMode::Move;
  }
}

void FramelessWin::mouseMoveEvent(QMouseEvent* event)
{
  const QPoint curMosPos = event->globalPos();

  if (!isMosPressed_) {
    if (isResized_) {
      updateCursor(getResizeDirection(curMosPos));
    }
  } else {
    switch (curOpMode_) {
    case OperationMode::Resize:
      if (curResizeDirection_ != ResizeDirection::None) {
        applyResize(curMosPos);
      }
      break;
    case OperationMode::Move:
      move(pos() + (curMosPos - prevMosPos_));
      break;
    }
  }

  prevMosPos_ = curMosPos;
  return QWidget::mouseMoveEvent(event);
}

void FramelessWin::mouseReleaseEvent(QMouseEvent* event)
{
  isMosPressed_       = false;
  curOpMode_          = OperationMode::None;
  curResizeDirection_ = ResizeDirection::None;
  return QWidget::mouseReleaseEvent(event);
}

void FramelessWin::updateCursor(ResizeDirection direction)
{
  switch (direction) {
  case ResizeDirection::TopLeft:
  case ResizeDirection::BottomRight:
    setCursor(Qt::SizeFDiagCursor);
    break;
  case ResizeDirection::TopRight:
  case ResizeDirection::BottomLeft:
    setCursor(Qt::SizeBDiagCursor);
    break;
  case ResizeDirection::Left:
  case ResizeDirection::Right:
    setCursor(Qt::SizeHorCursor);
    break;
  case ResizeDirection::Top:
  case ResizeDirection::Bottom:
    setCursor(Qt::SizeVerCursor);
    break;
  default:
    setCursor(Qt::ArrowCursor);
    break;
  }
}

FramelessWin::ResizeDirection FramelessWin::getResizeDirection(const QPoint& globalPos) const
{
  const auto& geom     = geometry();
  const auto xLeftOff  = std::abs(globalPos.x() - geom.left());
  const auto yTopOff   = std::abs(globalPos.y() - geom.top());
  const auto xRightOff = std::abs(globalPos.x() - geom.right());
  const auto yBottmOff = std::abs(globalPos.y() - geom.bottom());

  if (xLeftOff <= kResizeScaleLimit && yTopOff <= kResizeScaleLimit)
    return ResizeDirection::TopLeft;
  if (xRightOff <= kResizeScaleLimit && yTopOff <= kResizeScaleLimit)
    return ResizeDirection::TopRight;
  if (xRightOff <= kResizeScaleLimit && yBottmOff <= kResizeScaleLimit)
    return ResizeDirection::BottomRight;
  if (xLeftOff <= kResizeScaleLimit && yBottmOff <= kResizeScaleLimit)
    return ResizeDirection::BottomLeft;
  if (xLeftOff <= kResizeScaleLimit)
    return ResizeDirection::Left;
  if (xRightOff <= kResizeScaleLimit)
    return ResizeDirection::Right;
  if (yTopOff <= kResizeScaleLimit)
    return ResizeDirection::Top;
  if (yBottmOff <= kResizeScaleLimit)
    return ResizeDirection::Bottom;

  return ResizeDirection::None;
}

/// TODO: avoid using QRect::right|QRect::bottom|QRect::setRight|QRect::setBottom
///   use QRect::setWidth|QRect::setHeight instead
void FramelessWin::applyResize(const QPoint& globalPos)
{
  const auto off = globalPos - prevMosPos_;
  auto newGeom   = geometry();

  switch (curResizeDirection_) {
  case ResizeDirection::TopLeft: {
    auto newTopLeft = newGeom.topLeft() + off;
    auto newWidth   = (newGeom.x() + newGeom.width()) - newTopLeft.x();
    auto newHeight  = (newGeom.y() + newGeom.height()) - newTopLeft.y();

    if (newWidth < minimumWidth()) {
      newTopLeft.setX((newGeom.x() + newGeom.width()) - minimumWidth());
    } else if (newWidth > maximumWidth()) {
      newTopLeft.setX((newGeom.x() + newGeom.width()) - maximumWidth());
    }

    if (newHeight < minimumHeight()) {
      newTopLeft.setY((newGeom.y() + newGeom.height()) - minimumHeight());
    } else if (newHeight > maximumHeight()) {
      newTopLeft.setY((newGeom.y() + newGeom.height()) - maximumHeight());
    }

    newGeom.setTopLeft(newTopLeft);
  } break;

  case ResizeDirection::TopRight: {
    auto newTopRight = newGeom.topRight() + off;
    auto newWidth    = newTopRight.x() - newGeom.left();
    auto newHeight   = (newGeom.y() + newGeom.height()) - newTopRight.y();

    if (newWidth < minimumWidth()) {
      newTopRight.setX(newGeom.left() + minimumWidth());
    } else if (newWidth > maximumWidth()) {
      newTopRight.setX(newGeom.left() + maximumWidth());
    }

    if (newHeight < minimumHeight()) {
      newTopRight.setY((newGeom.y() + newGeom.height()) - minimumHeight());
    } else if (newHeight > maximumHeight()) {
      newTopRight.setY((newGeom.y() + newGeom.height()) - maximumHeight());
    }

    newGeom.setTopRight(newTopRight);
  } break;

  case ResizeDirection::BottomRight: {
    auto newBottomRight = newGeom.bottomRight() + off;
    auto newWidth       = newBottomRight.x() - newGeom.left();
    auto newHeight      = newBottomRight.y() - newGeom.top();

    if (newWidth < minimumWidth()) {
      newBottomRight.setX(newGeom.left() + minimumWidth());
    } else if (newWidth > maximumWidth()) {
      newBottomRight.setX(newGeom.left() + maximumWidth());
    }

    if (newHeight < minimumHeight()) {
      newBottomRight.setY(newGeom.top() + minimumHeight());
    } else if (newHeight > maximumHeight()) {
      newBottomRight.setY(newGeom.top() + maximumHeight());
    }

    newGeom.setBottomRight(newBottomRight);
  } break;

  case ResizeDirection::BottomLeft: {
    auto newBottomLeft = newGeom.bottomLeft() + off;
    auto newWidth      = (newGeom.x() + newGeom.width()) - newBottomLeft.x();
    auto newHeight     = newBottomLeft.y() - newGeom.top();

    if (newWidth < minimumWidth()) {
      newBottomLeft.setX((newGeom.x() + newGeom.width()) - minimumWidth());
    } else if (newWidth > maximumWidth()) {
      newBottomLeft.setX((newGeom.x() + newGeom.width()) - maximumWidth());
    }

    if (newHeight < minimumHeight()) {
      newBottomLeft.setY(newGeom.top() + minimumHeight());
    } else if (newHeight > maximumHeight()) {
      newBottomLeft.setY(newGeom.top() + maximumHeight());
    }

    newGeom.setBottomLeft(newBottomLeft);
  } break;

  case ResizeDirection::Left: {
    auto newX     = newGeom.left() + off.x();
    auto newWidth = (newGeom.x() + newGeom.width()) - newX;

    if (newWidth < minimumWidth()) {
      newX = (newGeom.x() + newGeom.width()) - minimumWidth();
    } else if (newWidth > maximumWidth()) {
      newX = (newGeom.x() + newGeom.width()) - maximumWidth();
    }

    newGeom.setLeft(newX);
  } break;

  case ResizeDirection::Right: {
    // auto newX     = (newGeom.x() + newGeom.width()) + off.x();
    auto newX     = newGeom.right() + off.x();
    auto newWidth = newX - newGeom.left();

    if (newWidth < minimumWidth()) {
      newX = newGeom.left() + minimumWidth();
    } else if (newWidth > maximumWidth()) {
      newX = newGeom.left() + maximumWidth();
    }

    newGeom.setRight(newX);
  } break;

  case ResizeDirection::Top: {
    auto newY      = newGeom.top() + off.y();
    auto newHeight = (newGeom.y() + newGeom.height()) - newY;

    if (newHeight < minimumHeight()) {
      newY = (newGeom.y() + newGeom.height()) - minimumHeight();
    } else if (newHeight > maximumHeight()) {
      newY = (newGeom.y() + newGeom.height()) - maximumHeight();
    }

    newGeom.setTop(newY);
  } break;

  case ResizeDirection::Bottom: {
    // auto newY      = (newGeom.y() + newGeom.height()) + off.y();
    auto newY      = newGeom.bottom() + off.y();
    auto newHeight = newY - newGeom.top();

    if (newHeight < minimumHeight()) {
      newY = newGeom.top() + minimumHeight();
    } else if (newHeight > maximumHeight()) {
      newY = newGeom.top() + maximumHeight();
    }

    newGeom.setBottom(newY);
  } break;

  default:
    return;
  }

  setGeometry(newGeom);
}
