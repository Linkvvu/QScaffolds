#pragma once
#include <QtWidgets/QWidget>

class FramelessWin : public QWidget {
  Q_OBJECT

  enum class ResizeDirection { None, TopLeft, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left };
  enum class OperationMode { None, Resize, Move };
  static constexpr int kResizeScaleLimit = 5;

public:
  explicit FramelessWin(QWidget* parent = nullptr);

  void setResized(bool on)
  {
    setMouseTracking(on);
    this->isResized_ = on;
  }

  void setDragging(bool on) { this->isDragging_ = on; }

protected:
  void paintEvent(QPaintEvent*) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  ResizeDirection getResizeDirection(const QPoint& globalPos) const;
  void updateCursor(ResizeDirection direction);
  void applyResize(const QPoint& globalPos);
  void initializeOperation(const QPoint& globalPos);

private:
  bool isDragging_   = false;
  bool isResized_    = false;
  bool isMosPressed_ = false;
  QPoint prevMosPos_;
  OperationMode curOpMode_            = OperationMode::None;
  ResizeDirection curResizeDirection_ = ResizeDirection::None;
};