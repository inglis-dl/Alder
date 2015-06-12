/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderAtlasWidget.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QAlderAtlasWidget.h>
#include <ui_QAlderAtlasWidget.h>

#include <Application.h>
#include <Exam.h>
#include <Image.h>
#include <Interview.h>
#include <Modality.h>
#include <Rating.h>
#include <ScanType.h>
#include <User.h>

#include <vtkEventQtSlotConnect.h>
#include <vtkMedicalImageViewer.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidget::QAlderAtlasWidget( QWidget* parent )
  : QWidget( parent )
{
  Alder::Application *app = Alder::Application::GetInstance();

  this->ui = new Ui_QAlderAtlasWidget;
  this->ui->setupUi( this );

  QObject::connect(
    this->ui->previousPushButton, SIGNAL( clicked() ),
    this, SLOT( slotPrevious() ) );
  QObject::connect(
    this->ui->nextPushButton, SIGNAL( clicked() ),
    this, SLOT( slotNext() ) );
  QObject::connect(
    this->ui->ratingComboBox, SIGNAL( currentIndexChanged( int ) ),
    this, SLOT( slotRatingChanged( int ) ) );

  this->Connections = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->Connections->Connect( app,
    Alder::Application::ActiveAtlasImageEvent,
    this, SLOT( updateInfo() ) );
  this->Connections->Connect( app,
    Alder::Application::ActiveAtlasImageEvent,
    this, SLOT( updateViewer() ) );
  this->Connections->Connect( app,
    Alder::Application::ActiveAtlasImageEvent,
    this, SLOT( updateEnabled() ) );

  this->Viewer = vtkSmartPointer<vtkMedicalImageViewer>::New();
  vtkRenderWindow* renwin = this->ui->imageWidget->GetRenderWindow();
  vtkRenderer* renderer = this->Viewer->GetRenderer();
  renderer->GradientBackgroundOn();
  renderer->SetBackground( 0, 0, 0 );
  renderer->SetBackground2( 0, 0, 1 );
  this->Viewer->SetRenderWindow( renwin );
  this->Viewer->InterpolateOff();
  this->Viewer->SetImageToSinusoid();

  this->updateEnabled();
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidget::~QAlderAtlasWidget()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::showEvent( QShowEvent* event )
{
  QWidget::showEvent( event );

  this->updateAtlasImage();

  emit showing( true );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::hideEvent( QHideEvent* event )
{
  QWidget::hideEvent( event );
  emit showing( false );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkMedicalImageViewer* QAlderAtlasWidget::GetViewer()
{
  return static_cast<vtkMedicalImageViewer*>(this->Viewer.GetPointer());
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::updateAtlasImage()
{
  if( ! this->isVisible() ) return;

  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *atlasImage = app->GetActiveAtlasImage();
  Alder::Image *image = app->GetActiveImage();

  // select an appropriate atlas image, if necessary
  if( image )
  {
    bool getNewAtlasImage = false;

    vtkSmartPointer< Alder::Exam > exam;
    image->GetRecord( exam );

    if( NULL == atlasImage ) getNewAtlasImage = true;
    else
    {
      vtkSmartPointer< Alder::Exam > atlasExam;
      atlasImage->GetRecord( atlasExam );
      if( exam->GetScanType() != atlasExam->GetScanType() ) getNewAtlasImage = true;
    }

    if( getNewAtlasImage )
    {
      vtkSmartPointer< Alder::Image > newAtlasImage =
        image->GetAtlasImage( this->ui->ratingComboBox->currentIndex() + 1 );
      app->SetActiveAtlasImage( newAtlasImage->Get( "Id" ).IsValid() ? newAtlasImage : NULL );
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::slotPrevious()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveAtlasImage();

  if( NULL != image )
  {
    vtkSmartPointer< Alder::Image > previousImage =
      image->GetPreviousAtlasImage( this->ui->ratingComboBox->currentIndex() + 1 );
    app->SetActiveAtlasImage( previousImage->Get( "Id" ).IsValid() ? previousImage : NULL );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::slotNext()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveAtlasImage();

  if( NULL != image )
  {
    vtkSmartPointer< Alder::Image > nextImage =
      image->GetNextAtlasImage( this->ui->ratingComboBox->currentIndex() + 1 );
    app->SetActiveAtlasImage( nextImage->Get( "Id" ).IsValid() ? nextImage : NULL );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::slotRatingChanged( int value )
{
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active user and image
  Alder::User *user = app->GetActiveUser();
  if( !user ) throw std::runtime_error( "Rating slider modified without an active user" );
  Alder::Image *image = app->GetActiveImage();
  if( !image ) throw std::runtime_error( "Rating slider modified without an active image" );

  // See if we have an atlas entry for this kind of image at the requested rating
  vtkSmartPointer< Alder::Exam > exam;
  image->GetRecord( exam );
  vtkSmartPointer< Alder::Image > newAtlasImage =
    image->GetAtlasImage( this->ui->ratingComboBox->currentIndex() + 1 );
  app->SetActiveAtlasImage( newAtlasImage->Get( "Id" ).IsValid() ? newAtlasImage : NULL );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::updateInfo()
{
  QString helpString = "";
  QString noteString = "";
  QString interviewerString = tr( "N/A" );
  QString siteString = tr( "N/A" );
  QString dateString = tr( "N/A" );
  QString uidString = tr( "N/A" );
  QString codeString = tr( "N/A" );

  Alder::Image *image = Alder::Application::GetInstance()->GetActiveAtlasImage();
  if( image )
  {
    vtkSmartPointer< Alder::Exam > exam;
    vtkSmartPointer< Alder::Interview > interview;
    if( image->GetRecord( exam ) && exam->GetRecord( interview ) )
    {
      noteString = image->Get( "Note" ).ToString().c_str();
      interviewerString = exam->Get( "Interviewer" ).ToString().c_str();
      siteString = interview->Get( "Site" ).ToString().c_str();
      dateString = exam->Get( "DatetimeAcquired" ).ToString().c_str();
      uidString = interview->Get( "UId" ).ToString().c_str();
      codeString = image->GetCode().c_str();

      vtkSmartPointer< Alder::ScanType > scanType;
      exam->GetRecord( scanType );
      vtkSmartPointer< Alder::Modality > modality;
      scanType->GetRecord( modality );
      helpString = modality->Get( "Help" ).ToString();
    }
  }

  this->ui->helpTextEdit->setPlainText( helpString );
  this->ui->noteTextEdit->setPlainText( noteString );
  this->ui->infoInterviewerValueLabel->setText( interviewerString );
  this->ui->infoSiteValueLabel->setText( siteString );
  this->ui->infoDateValueLabel->setText( dateString );
  this->ui->infoCodeValueLabel->setText( codeString );
  this->ui->infoUIdValueLabel->setText( uidString );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::updateViewer()
{
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveAtlasImage();
  if( image ) this->Viewer->Load( image->GetFileName().c_str() );
  else this->Viewer->SetImageToSinusoid();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::updateEnabled()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveAtlasImage();

  // set all widget enable states
  this->ui->previousPushButton->setEnabled( image );
  this->ui->nextPushButton->setEnabled( image );
  this->ui->noteTextEdit->setEnabled( image );
  this->ui->helpTextEdit->setEnabled( image );
  this->ui->ratingComboBox->setEnabled( this->isVisible() );
}
