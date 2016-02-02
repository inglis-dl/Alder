/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QMedicalImageWidget.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QMedicalImageWidget.h>

#include <ui_QMedicalImageWidget.h>

#include <vtkMedicalImageViewer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkEventQtSlotConnect.h>

#include <QColorDialog>
#include <QPainter>
#include <QPen>
#include <QScrollBar>

#include <sstream>
#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMedicalImageWidget::QMedicalImageWidget( QWidget* parent )
  : QWidget( parent )
{
  this->ui = new Ui_QMedicalImageWidget;
  this->ui->setupUi( this );
  this->viewer = vtkSmartPointer<vtkMedicalImageViewer>::New();
  vtkRenderWindow* renwin = this->ui->vtkWidget->GetRenderWindow();
  this->viewer->SetRenderWindow( renwin );
  vtkRenderer* renderer = this->viewer->GetRenderer();
  renderer->GradientBackgroundOn();
  renderer->SetBackground( 0, 0, 0 );
  renderer->SetBackground2( 0, 0, 1 );
  this->viewer->SetRenderWindow( renwin );
  this->viewer->InterpolateOff();
  this->ui->interpolationToggleButton->setIcon(QIcon(":/icons/nearesticon"));
  this->resetImage();

  connect( this->ui->invertButton, SIGNAL( clicked() ), this, SLOT( slotInvertWindowLevel() ) );
  connect( this->ui->flipVerticalButton, SIGNAL( clicked() ), this, SLOT( slotFlipVertical() ) );
  connect( this->ui->flipHorizontalButton, SIGNAL( clicked() ), this, SLOT( slotFlipHorizontal() ) );
  connect( this->ui->rotateCWButton, SIGNAL( clicked() ), this, SLOT( slotRotateClockwise() ) );
  connect( this->ui->rotateCCWButton, SIGNAL( clicked() ), this, SLOT( slotRotateCounterClockwise() ) );
  connect( this->ui->interpolationToggleButton, SIGNAL( clicked() ), this, SLOT( slotInterpolationToggle() ) );
  connect( this->ui->foregroundButton, SIGNAL( clicked() ), this, SLOT( slotSelectColor() ) );
  connect( this->ui->backgroundButton, SIGNAL( clicked() ), this, SLOT( slotSelectColor() ) );

  this->ui->framePlayerWidget->setViewer( this->viewer );

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

  this->ui->vtkWidget->installEventFilter( this );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMedicalImageWidget::~QMedicalImageWidget()
{
  this->ui->framePlayerWidget->setViewer( 0 );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QMedicalImageWidget::eventFilter( QObject *obj, QEvent *event )
{
  if( obj == this->ui->vtkWidget )
  {
    if( QEvent::Enter == event->type() )
    {
      qobject_cast<QFrame*>(this->ui->vtkWidget->parent())->setStyleSheet("border : 3px solid red");
    }
    if( QEvent::Leave == event->type() )
    {
      qobject_cast<QFrame*>(this->ui->vtkWidget->parent())->setStyleSheet("border : 3px solid green");
    }
  }
  return false;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkMedicalImageViewer* QMedicalImageWidget::GetViewer()
{
  return static_cast<vtkMedicalImageViewer*>( this->viewer.GetPointer() );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::resetImage()
{
  this->viewer->SetImageToSinusoid();
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::loadImage( QString filename )
{
  if( !this->viewer->Load( filename.toStdString() ) )
  {
    std::stringstream stream;
    stream << "Unable to load image file \"" << filename.toStdString() << "\"";
    throw std::runtime_error( stream.str() );
  }

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::updateInterface()
{
  this->ui->framePlayerWidget->updateFromViewer();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::saveImage( const QString& fileName )
{
  this->viewer->WriteSlice( fileName.toStdString().c_str() );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::slotFlipVertical()
{
  this->viewer->FlipCameraVertical();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::slotFlipHorizontal()
{
  this->viewer->FlipCameraHorizontal();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::slotRotateClockwise()
{
  this->viewer->RotateCamera( -90.0 );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::slotRotateCounterClockwise()
{
  this->viewer->RotateCamera( 90.0 );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::slotInterpolationToggle()
{
  int interpolate = this->viewer->GetInterpolate();
  if( interpolate )
  {
    this->ui->interpolationToggleButton->setIcon(QIcon(":/icons/nearesticon"));
    this->viewer->InterpolateOff();
  }
  else
  {
    this->ui->interpolationToggleButton->setIcon(QIcon(":/icons/linearicon"));
    this->viewer->InterpolateOn();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::slotInvertWindowLevel()
{
  this->viewer->InvertWindowLevel();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::slotSelectColor()
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

    double color[3];
    color[0] = qcolor.redF();
    color[1] = qcolor.greenF();
    color[2] = qcolor.blueF();

    vtkRenderer* renderer = this->viewer->GetRenderer();
    if( QString("foregroundButton") == button->objectName() )
      renderer->SetBackground2( color );
    else
      renderer->SetBackground( color );
  }
}

