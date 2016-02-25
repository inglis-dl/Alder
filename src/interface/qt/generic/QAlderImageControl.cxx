/*==============================================================================

  Module:    QAlderImageControl.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

==============================================================================*/
#include <QAlderImageControl.h>
#include <ui_QAlderImageControl.h>

// Alder includes
#include <Common.h>
#include <QAlderSliceView.h>

// Qt includes
#include <QIcon>
#include <QColor>
#include <QColorDialog>
#include <QPainter>
#include <QPen>

// VTK includes
#include <vtkEventQtSlotConnect.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QAlderImageControlPrivate : public Ui_QAlderImageControl
{
  Q_DECLARE_PUBLIC(QAlderImageControl);
protected:
  QAlderImageControl* const q_ptr;

public:
  QAlderImageControlPrivate(QAlderImageControl& object);
  virtual ~QAlderImageControlPrivate();

  virtual void setupUi(QWidget*);
  virtual void updateUi();

  QColor color( const int& which ) const; // 1 => foreground, 0 => background, 2 => annotation
  void setColor( const int& which, const QColor& color );

private:
  QColor backgroundColor;
  QColor foregroundColor;
  QColor annotationColor;
  bool interpolation;

  void drawButton( const int& which );
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderImageControlPrivate methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderImageControlPrivate::QAlderImageControlPrivate
(QAlderImageControl& object)
  : q_ptr(&object)
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderImageControlPrivate::~QAlderImageControlPrivate()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControlPrivate::setupUi( QWidget* widget )
{
  Q_Q(QAlderImageControl);

  this->Ui_QAlderImageControl::setupUi( widget );

  this->foregroundColorButton->setProperty( "which", QVariant(1) );
  this->backgroundColorButton->setProperty( "which", QVariant(0) );
  this->annotationColorButton->setProperty( "which", QVariant(2) );

  QColor qcolor;
  qcolor.setRgbF(0.,0.,0.);
  this->setColor(0, qcolor); // black background
  qcolor.setRgbF(0.,0.,1.);
  this->setColor(1, qcolor); // blue foreground
  qcolor.setRgbF(1.,1.,1.);
  this->setColor(2, qcolor); // white annotation text
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControlPrivate::updateUi()
{
  Q_Q(QAlderImageControl);
  if(!q->sliceViewPointer.isNull())
  {
    QAlderSliceView* view = q->sliceViewPointer.data();
    this->setColor( 2, view->annotationColor() );
    this->setColor( 1, view->foregroundColor() );
    this->setColor( 0, view->backgroundColor() );
    this->interpolationButton->blockSignals(true);
    q->setInterpolation( view->interpolation() );
    this->interpolationButton->blockSignals(false);
    bool enable = view->hasImageData();
    QList<QToolButton*> buttons = q->findChildren<QToolButton*>();
    QListIterator<QToolButton*> it(buttons);
    while(it.hasNext())
    {
      it.next()->setEnabled(enable);
    }
  }
  else
  {
    q->setEnabled(false);
  }
}

// set the color(s) of the SliceView and this widget's buttons
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControlPrivate::setColor( const int& which, const QColor& color )
{
  Q_Q(QAlderImageControl);

  QAlderSliceView* view = q->sliceViewPointer.isNull() ? 0 : q->sliceViewPointer.data();
  if(2 == which)
  {
    this->annotationColor = color;
    if(view)
    {
      view->setAnnotationColor( color );
    }
    this->drawButton(which);
    return;
  }

  if(1 == which)
    this->foregroundColor = color;
  else
    this->backgroundColor = color;

  bool gradient = true;
  if(view)
  {
    gradient = view->gradientBackground();
    if(gradient)
    {
      if(1 == which)
        view->setForegroundColor( this->foregroundColor );
      else
        view->setBackgroundColor( this->backgroundColor );
    }
    else
    {
      this->foregroundColor = color;
      this->backgroundColor = color;
      view->setForegroundColor( color );
      view->setBackgroundColor( color );
    }
  }
  if(gradient)
  {
    this->drawButton(which);
  }
  else
  {
    this->drawButton(0);
    this->drawButton(1);
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControlPrivate::drawButton( const int& which )
{
  QToolButton* button;
  QColor color;
  if(1 == which)
  {
    color = this->foregroundColor;
    button = this->foregroundColorButton;
  }
  else if(0 == which)
  {
    color = this->backgroundColor;
    button = this->backgroundColorButton;
  }
  else if(2 == which)
  {
    color = this->annotationColor;
    button = this->annotationColorButton;
  }

  int iconSize = button->style()->pixelMetric( QStyle::PM_SmallIconSize );
  QPixmap pix( iconSize , iconSize );
  pix.fill( color.isValid() ? button->palette().button().color() : Qt::transparent);
  QPainter painter( &pix );
  painter.setPen( QPen( Qt::gray ) );
  painter.setBrush( color.isValid() ? color : QBrush( Qt::NoBrush ) );
  painter.drawRect( 0, 0, pix.width(), pix.height() );
  button->setIcon( QIcon( pix ) );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QColor QAlderImageControlPrivate::color( const int& which ) const
{
  QColor qcolor;
  switch(which)
  {
    case 0: qcolor = this->backgroundColor; break;
    case 1: qcolor = this->foregroundColor; break;
    case 2: qcolor = this->annotationColor; break;
  }
  return qcolor;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderImageControl methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderImageControl::QAlderImageControl(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new QAlderImageControlPrivate(*this))
{
  Q_D(QAlderImageControl);
  d->setupUi(this);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderImageControl::~QAlderImageControl()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::update()
{
  Q_D(QAlderImageControl);
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::setSliceView( QAlderSliceView* view )
{
  Q_D(QAlderImageControl);

  if( !this->sliceViewPointer.isNull() )
  {
    disconnect( view, SIGNAL( imageDataChanged() ), this, SLOT( update() ) );
    disconnect( d->flipVerticalButton, SIGNAL(pressed()), view, SLOT(flipCameraVertical()) );
    disconnect( d->flipHorizontalButton, SIGNAL(pressed()), view, SLOT(flipCameraHorizontal()) );
    disconnect( d->rotateClockwiseButton, SIGNAL(pressed()), view, SLOT(rotateCameraClockwise()) );
    disconnect( d->rotateCounterClockwiseButton, SIGNAL(pressed()), view, SLOT(rotateCameraCounterClockwise()) );
    disconnect( d->interpolationButton, SIGNAL(toggled(bool)), this, SLOT(interpolate(bool)) );
    disconnect( d->invertWindowLevelButton, SIGNAL(pressed()), view, SLOT(invertColorWindowLevel()) );
    disconnect( d->backgroundColorButton, SIGNAL(pressed()), this, SLOT(selectColor()) );
    disconnect( d->foregroundColorButton, SIGNAL(pressed()), this, SLOT(selectColor()) );
    disconnect( d->annotationColorButton, SIGNAL(pressed()), this, SLOT(selectColor()) );
  }
  this->sliceViewPointer = view;

  if( !this->sliceViewPointer.isNull() )
  {
    connect( view, SIGNAL( imageDataChanged() ), this, SLOT( update() ) );
    connect( d->flipVerticalButton, SIGNAL(pressed()), view, SLOT(flipCameraVertical()) );
    connect( d->flipHorizontalButton, SIGNAL(pressed()), view, SLOT(flipCameraHorizontal()) );
    connect( d->rotateClockwiseButton, SIGNAL(pressed()), view, SLOT(rotateCameraClockwise()) );
    connect( d->rotateCounterClockwiseButton, SIGNAL(pressed()), view, SLOT(rotateCameraCounterClockwise()) );
    connect( d->interpolationButton, SIGNAL(toggled(bool)), this, SLOT(interpolate(bool)) );
    connect( d->invertWindowLevelButton, SIGNAL(pressed()), view, SLOT(invertColorWindowLevel()) );
    connect( d->backgroundColorButton, SIGNAL(pressed()), this, SLOT(selectColor()) );
    connect( d->foregroundColorButton, SIGNAL(pressed()), this, SLOT(selectColor()) );
    connect( d->annotationColorButton, SIGNAL(pressed()), this, SLOT(selectColor()) );
  }
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::selectColor()
{
  Q_D(QAlderImageControl);

  QToolButton* button = qobject_cast<QToolButton*>( sender() );
  QColorDialog dialog( this );
  if( button && dialog.exec() )
  {
    QColor qcolor = dialog.selectedColor();
    QVariant v = button->property("which");
    if(v.isValid())
      d->setColor( v.toInt(), qcolor );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::interpolate(bool state)
{
  Q_D(QAlderImageControl);
  QAlderSliceView* view = this->sliceViewPointer.isNull() ? 0 : this->sliceViewPointer.data();
  if(state)
  {
    if( view )
    {
      view->setInterpolation( VTK_LINEAR_INTERPOLATION );
    }
    d->interpolationButton->setIcon(QIcon(":/icons/linearicon"));
  }
  else
  {
    if( view )
    {
      view->setInterpolation( VTK_NEAREST_INTERPOLATION );
    }
    d->interpolationButton->setIcon(QIcon(":/icons/nearesticon"));
  }
  d->interpolation = state;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderImageControl::interpolation() const
{
  Q_D(const QAlderImageControl);
  return d->interpolation;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::setInterpolation( const bool& state )
{
  this->interpolate( state );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QColor QAlderImageControl::backgroundColor() const
{
  Q_D(const QAlderImageControl);
  return d->color(0);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QColor QAlderImageControl::foregroundColor() const
{
  Q_D(const QAlderImageControl);
  return d->color(1);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QColor QAlderImageControl::annotationColor() const
{
  Q_D(const QAlderImageControl);
  return d->color(2);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::setForegroundColor( const QColor& color )
{
  Q_D(QAlderImageControl);
  d->setColor( 0, color );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::setBackgroundColor( const QColor& color )
{
  Q_D(QAlderImageControl);
  d->setColor( 1, color );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::setAnnotationColor( const QColor& color )
{
  Q_D(QAlderImageControl);
  d->setColor( 2, color );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::setFlipVerticalIcon(const QIcon& ico)
{
  Q_D(QAlderImageControl);
  d->flipVerticalButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::setFlipHorizontalIcon(const QIcon& ico)
{
  Q_D(QAlderImageControl);
  d->flipHorizontalButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::setRotateClockwiseIcon(const QIcon& ico)
{
  Q_D(QAlderImageControl);
  d->rotateClockwiseButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::setRotateCounterClockwiseIcon(const QIcon& ico)
{
  Q_D(QAlderImageControl);
  d->rotateCounterClockwiseButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::setInterpolationIcon(const QIcon& ico)
{
  Q_D(QAlderImageControl);
  d->interpolationButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageControl::setInvertWindowLevelIcon(const QIcon& ico)
{
  Q_D(QAlderImageControl);
  d->invertWindowLevelButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderImageControl::flipVerticalIcon() const
{
  Q_D(const QAlderImageControl);
  return d->flipVerticalButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderImageControl::flipHorizontalIcon() const
{
  Q_D(const QAlderImageControl);
  return d->flipHorizontalButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderImageControl::rotateClockwiseIcon() const
{
  Q_D(const QAlderImageControl);
  return d->rotateClockwiseButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderImageControl::rotateCounterClockwiseIcon() const
{
  Q_D(const QAlderImageControl);
  return d->rotateCounterClockwiseButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderImageControl::interpolationIcon() const
{
  Q_D(const QAlderImageControl);
  return d->interpolationButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderImageControl::invertWindowLevelIcon() const
{
  Q_D(const QAlderImageControl);
  return d->invertWindowLevelButton->icon();
}
