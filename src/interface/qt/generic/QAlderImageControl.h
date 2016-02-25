/*==============================================================================

  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Module:    QAlderImageControl.cxx
  Language:  C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

==============================================================================*/

/**
 * @class QAlderImageControl
 *
 * @brief Qt widget for controlling images in a QAlderSliceView.
 *
 */
#ifndef __QAlderImageControl_h
#define __QAlderImageControl_h

// Qt includes
#include <QColor>
#include <QIcon>
#include <QPointer>
#include <QWidget>

// Alder includes
class QAlderSliceView;
class QAlderImageControlPrivate;

class QAlderImageControl : public QWidget
{
  Q_OBJECT

  /**
   * This property holds the flipVertical button's icon.
   * @see flipVerticalIcon(), setFlipVerticalIcon()
   */
  Q_PROPERTY(QIcon flipVerticalIcon READ flipVerticalIcon WRITE setFlipVerticalIcon)

  /**
   * This property holds the flipHorizontal button's icon.
   * @see flipHorizontalIcon(), setFlipHorizontalIcon()
   */
  Q_PROPERTY(QIcon flipHorizontalIcon READ flipHorizontalIcon WRITE setFlipHorizontalIcon)

  /**
   * This property holds the rotateClockwise button's icon.
   * @see rotateClockwiseIcon(), setRotateClockwiseIcon()
   */
  Q_PROPERTY(QIcon rotateClockwiseIcon READ rotateClockwiseIcon WRITE setRotateClockwiseIcon)

  /**
   * This property holds the rotateCounterClockwise button's icon.
   * @see rotateCounterClockwiseIcon(), setRotateCounterClockwiseIcon()
   */
  Q_PROPERTY(QIcon rotateCounterClockwiseIcon READ rotateCounterClockwiseIcon WRITE setRotateCounterClockwiseIcon)

  /**
   * This property holds the interpolation button's icon.
   * @see interpolationIcon(), setInterpolationIcon()
   */
  Q_PROPERTY(QIcon interpolationIcon READ interpolationIcon WRITE setInterpolationIcon)

  /**
   * This property holds the invertWindowLevel button's icon.
   * @see invertWindowLevelIcon(), setInvertWindowLevelIcon()
   */
  Q_PROPERTY(QIcon invertWindowLevelIcon READ invertWindowLevelIcon WRITE setInvertWindowLevelIcon)

  /**
   * This property holds the color of the upper portion of the viewing area.
   */
  Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor)

  /**
   * This property holds the color of the lower portion of the viewing area.
   */
  Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)

  /**
   * This property holds the color of the annotations.
   */
  Q_PROPERTY(QColor annotationColor READ annotationColor WRITE setAnnotationColor)

  /**
   * This property holds the interpolation status (on or off).
   */
  Q_PROPERTY(bool interpolation READ interpolation WRITE setInterpolation)

public:
  typedef QWidget Superclass;
  QAlderImageControl(QWidget* parent=0);
  virtual ~QAlderImageControl();

  //@{
  /**  Convenience method to set up signals and slots with a QAlderSliceView. */
  void setSliceView( QAlderSliceView* );
  //@}

  //@{
  /** Set/Get the first frame icon. */
  void setFlipVerticalIcon(const QIcon&);
  QIcon flipVerticalIcon() const;
  //@}

  //@{
  /** Set/Get the previous frame icon. */
  void setFlipHorizontalIcon(const QIcon&);
  QIcon flipHorizontalIcon() const;
  //@}

  /** Set/Get the play icon. */
  void setRotateClockwiseIcon(const QIcon&);
  QIcon rotateClockwiseIcon() const;
  //@}

  //@{
  /** Set/Get the reverse icon. */
  void setRotateCounterClockwiseIcon(const QIcon&);
  QIcon rotateCounterClockwiseIcon() const;
  //@}

  //@{
  /** Set/Get the icon of the next frame button. */
  void setInterpolationIcon(const QIcon&);
  QIcon interpolationIcon() const;
  //@}

  //@{
  /** Set/Get the icon of the last frame button. */
  void setInvertWindowLevelIcon(const QIcon&);
  QIcon invertWindowLevelIcon() const;
  //@}

  //@{
  /** Set/Get the background color. */
  void setBackgroundColor( const QColor& );
  QColor backgroundColor() const;
  //@}

  //@{
  /** Set/Get the foreground color. */
  void setForegroundColor( const QColor& );
  QColor foregroundColor() const;
  //@}

  //@{
  /** Set/Get the annotation color. */
  void setAnnotationColor( const QColor& );
  QColor annotationColor() const;
  //@}

  //@{
  /** Set/Get the interpolation. */
  void setInterpolation( const bool& );
  bool interpolation() const;
  //@}

public Q_SLOTS:
public Q_SLOTS:

  void selectColor();
  void interpolate(bool);
  void update();

protected:
  QScopedPointer<QAlderImageControlPrivate> d_ptr;
  QPointer<QAlderSliceView> sliceViewPointer;

private:
  Q_DECLARE_PRIVATE(QAlderImageControl);
  Q_DISABLE_COPY(QAlderImageControl);
};

#endif
