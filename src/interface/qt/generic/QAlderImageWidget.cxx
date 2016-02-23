/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderImageWidget.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QAlderImageWidget.h>
#include <ui_QAlderImageWidget.h>

// Alder includes
#include <QAlderSliceView.h>

// VTK includes
#include <vtkEventQtSlotConnect.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

// Qt includes
#include <QColorDialog>
#include <QPainter>
#include <QPen>
#include <QScrollBar>

#include <sstream>
#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderImageWidget::QAlderImageWidget( QWidget* parent )
  : QWidget( parent )
{
  this->ui = new Ui_QAlderImageWidget;
  this->ui->setupUi( this );

  vtkRenderer* renderer = this->ui->sliceView->renderer();
  renderer->GradientBackgroundOn();
  renderer->SetBackground( 0, 0, 0 );
  renderer->SetBackground2( 0, 0, 1 );

  this->ui->interpolationToggleButton->setIcon(QIcon(":/icons/nearesticon"));

  this->resetImage();

  connect( this->ui->invertButton, SIGNAL( clicked() ),
           this->ui->sliceView, SLOT( invertColorWindowLevel() ) );
  connect( this->ui->flipVerticalButton, SIGNAL( clicked() ),
           this->ui->sliceView, SLOT( flipCameraVertical() ) );
  connect( this->ui->flipHorizontalButton, SIGNAL( clicked() ),
           this->ui->sliceView, SLOT( flipCameraHorizontal() ) );
  connect( this->ui->rotateCWButton, SIGNAL( clicked() ),
           this->ui->sliceView, SLOT( rotateCameraClockwise() ) );
  connect( this->ui->rotateCCWButton, SIGNAL( clicked() ),
           this->ui->sliceView, SLOT( rotateCameraCounterClockwise() ) );
  connect( this->ui->interpolationToggleButton, SIGNAL( clicked() ),
           this, SLOT( slotInterpolationToggle() ) );
  connect( this->ui->foregroundButton, SIGNAL( clicked() ), this, SLOT( slotSelectColor() ) );
  connect( this->ui->backgroundButton, SIGNAL( clicked() ), this, SLOT( slotSelectColor() ) );

  this->ui->framePlayerWidget->setSliceView( this->ui->sliceView );

  QColor qcolor;
  double* color = renderer->GetBackground();
  qcolor.setRgbF( color[0], color[1], color[2] );
  int iconSize = this->ui->backgroundButton->style()->pixelMetric( QStyle::PM_SmallIconSize );
  QPixmap pix( iconSize , iconSize );
  pix.fill( qcolor.isValid() ? this->ui->backgroundButton->palette().button().color() : Qt::transparent);
  QPainter painter( &pix );
  painter.setPen( QPen( Qt::gray ) );
  painter.setBrush( qcolor.isValid() ? qcolor : QBrush( Qt::NoBrush ) );
  painter.drawRect( 2, 2, pix.width() - 2, pix.height() - 2 );

  this->ui->backgroundButton->setIcon( QIcon( pix ) );

  color = renderer->GetBackground2();
  qcolor.setRgbF( color[0], color[1], color[2] );
  pix.fill( qcolor.isValid() ? this->ui->foregroundButton->palette().button().color() : Qt::transparent);
  painter.setBrush( qcolor.isValid() ? qcolor : QBrush( Qt::NoBrush ) );
  painter.drawRect( 2, 2, pix.width() - 2, pix.height() - 2 );

  this->ui->foregroundButton->setIcon( QIcon( pix ) );

  this->ui->sliceView->VTKWidget()->installEventFilter( this );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderImageWidget::~QAlderImageWidget()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderImageWidget::eventFilter( QObject *obj, QEvent *event )
{
  if( obj == this->ui->sliceView->VTKWidget() )
  {
    if( QEvent::Enter == event->type() )
    {
      this->ui->frame->setStyleSheet("border : 3px solid red");
    }
    if( QEvent::Leave == event->type() )
    {
      this->ui->frame->setStyleSheet("border : 3px solid green");
    }
  }
  return false;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageWidget::resetImage()
{
  this->ui->sliceView->setImageToSinusoid();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageWidget::loadImage( const QString& fileName )
{
  if( !this->ui->sliceView->load( fileName ) )
  {
    std::stringstream stream;
    stream << "Unable to load image file \"" << fileName.toStdString() << "\"";
    throw std::runtime_error( stream.str() );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageWidget::saveImage( const QString& fileName )
{
  this->ui->sliceView->writeSlice( fileName );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageWidget::slotInterpolationToggle()
{
  int interpolation = this->ui->sliceView->interpolation();
  if( VTK_LINEAR_INTERPOLATION == interpolation )
  {
    this->ui->interpolationToggleButton->setIcon(QIcon(":/icons/nearesticon"));
    this->ui->sliceView->setInterpolation( VTK_NEAREST_INTERPOLATION );
  }
  else
  {
    this->ui->interpolationToggleButton->setIcon(QIcon(":/icons/linearicon"));
    this->ui->sliceView->setInterpolation( VTK_LINEAR_INTERPOLATION );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderImageWidget::slotSelectColor()
{
  QToolButton* button = qobject_cast<QToolButton*>( sender() );
  QColorDialog dialog( this );
  if( button && dialog.exec() )
  {
    QColor qcolor = dialog.selectedColor();
    int iconSize = button->style()->pixelMetric( QStyle::PM_SmallIconSize );
    QPixmap pix( iconSize , iconSize );
    pix.fill( qcolor.isValid() ? button->palette().button().color() : Qt::transparent);
    QPainter painter( &pix );
    painter.setPen( QPen( Qt::gray ) );
    painter.setBrush( qcolor.isValid() ? qcolor : QBrush( Qt::NoBrush ) );
    painter.drawRect( 2, 2, pix.width() - 2, pix.height() - 2 );

    button->setIcon( QIcon( pix ) );

    if( QString("foregroundButton") == button->objectName() )
      this->ui->sliceView->setBackgroundColor2( qcolor );
    else
      this->ui->sliceView->setBackgroundColor( qcolor );
  }
}
