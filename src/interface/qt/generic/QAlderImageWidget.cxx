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

#include <sstream>
#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderImageWidget::QAlderImageWidget( QWidget* parent )
  : QWidget( parent )
{
  this->ui = new Ui_QAlderImageWidget;
  this->ui->setupUi( this );

/*
  vtkRenderer* renderer = this->ui->sliceView->renderer();
  renderer->GradientBackgroundOn();
  renderer->SetBackground( 0, 0, 0 );
  renderer->SetBackground2( 0, 0, 1 );
*/

  this->ui->imageControl->setSliceView( this->ui->sliceView );
  this->ui->framePlayerWidget->setSliceView( this->ui->sliceView );

  this->ui->sliceView->VTKWidget()->installEventFilter( this );
  this->resetImage();
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
