/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderAtlasWidget.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QAlderAtlasWidget.h>
#include <QAlderAtlasWidget_p.h>

// Alder includes
#include <Application.h>
#include <Common.h>
#include <Exam.h>
#include <Image.h>
#include <ImageNote.h>
#include <Interview.h>
#include <Modality.h>
#include <Rating.h>
#include <ScanType.h>
#include <Site.h>
#include <User.h>
#include <QAlderImageWidget.h>

// VTK includes
#include <vtkEventQtSlotConnect.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderAtlasWidgetPrivate methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidgetPrivate::QAlderAtlasWidgetPrivate(QAlderAtlasWidget& object)
  : QObject(&object), q_ptr(&object)
{
  this->qvtkConnection = vtkSmartPointer<vtkEventQtSlotConnect>::New();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidgetPrivate::~QAlderAtlasWidgetPrivate()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidgetPrivate::setupUi( QWidget* widget )
{
  Q_Q(QAlderAtlasWidget);

  this->Ui_QAlderAtlasWidget::setupUi( widget );

  QObject::connect(
    this->previousPushButton, SIGNAL( clicked() ),
    this, SLOT( previous() ) );
  QObject::connect(
    this->nextPushButton, SIGNAL( clicked() ),
    this, SLOT( next() ) );
  QObject::connect(
    this->ratingComboBox, SIGNAL( currentIndexChanged( int ) ),
    this, SLOT( ratingChanged() ) );

  Alder::Application *app = Alder::Application::GetInstance();

  this->qvtkConnection->Connect( app, Alder::Common::ActiveAtlasImageEvent,
    q, SLOT( update() ) );
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidgetPrivate::ratingChanged()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveImage();
  if( image )
  {
    // See if we have an atlas entry for this kind of image at the requested rating
    vtkSmartPointer< Alder::Exam > exam;
    image->GetRecord( exam );
    vtkSmartPointer< Alder::Image > newAtlasImage =
      image->GetAtlasImage( this->rating() );
    app->SetActiveAtlasImage( newAtlasImage->Get( "Id" ).IsValid() ? newAtlasImage : NULL );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidgetPrivate::previous()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveAtlasImage();

  if( image )
  {
    vtkSmartPointer< Alder::Image > previousImage =
      image->GetPreviousAtlasImage( this->rating() );
    app->SetActiveAtlasImage( previousImage->Get( "Id" ).IsValid() ? previousImage : NULL );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidgetPrivate::next()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveAtlasImage();

  if( image )
  {
    vtkSmartPointer< Alder::Image > nextImage =
      image->GetNextAtlasImage( this->rating() );
    app->SetActiveAtlasImage( nextImage->Get( "Id" ).IsValid() ? nextImage : NULL );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidgetPrivate::updateUi()
{
  Q_Q(QAlderAtlasWidget);
  QString helpString = "";
  QStringList noteString;
  QString interviewerString = tr( "N/A" );
  QString siteString = tr( "N/A" );
  QString dateString = tr( "N/A" );
  QString uidString = tr( "N/A" );
  QString codeString = tr( "N/A" );

  Alder::Image *image = Alder::Application::GetInstance()->GetActiveAtlasImage();
  bool enable = false;
  if( image )
  {
    vtkSmartPointer< Alder::Exam > exam;
    vtkSmartPointer< Alder::Interview > interview;
    vtkSmartPointer< Alder::Site > site;
    if( image->GetRecord( exam ) &&
        exam->GetRecord( interview ) &&
        interview->GetRecord( site ) )
    {
      std::vector< vtkSmartPointer< Alder::ImageNote > > noteList;
      image->GetList( &noteList );
      if( !noteList.empty() )
      {
        for( auto it = noteList.begin(); it != noteList.end(); ++it )
        {
          noteString << (*it)->Get( "Note" ).ToString().c_str();
        }
      }
      interviewerString = exam->Get( "Interviewer" ).ToString().c_str();
      siteString = site->Get( "Name" ).ToString().c_str();
      dateString = exam->Get( "DatetimeAcquired" ).ToString().c_str();
      uidString = interview->Get( "UId" ).ToString().c_str();
      codeString = image->GetCode().c_str();

      vtkSmartPointer< Alder::ScanType > scanType;
      exam->GetRecord( scanType );
      vtkSmartPointer< Alder::Modality > modality;
      scanType->GetRecord( modality );
      helpString = modality->Get( "Help" ).ToString();
    }
    this->imageWidget->load( image->GetFileName().c_str() );
    enable = true;
  }
  else
    this->imageWidget->reset();

  this->helpTextEdit->setPlainText( helpString );
  for( int i = 0; i < noteString.size(); ++i )
  {
    this->noteTextEdit->insertPlainText( noteString.at(i) );
    this->noteTextEdit->moveCursor( QTextCursor::End );
  }
  this->infoInterviewerValueLabel->setText( interviewerString );
  this->infoSiteValueLabel->setText( siteString );
  this->infoDateValueLabel->setText( dateString );
  this->infoCodeValueLabel->setText( codeString );
  this->infoUIdValueLabel->setText( uidString );

  // set all widget enable states
  this->previousPushButton->setEnabled( enable );
  this->nextPushButton->setEnabled( enable );
  this->noteTextEdit->setEnabled( enable );
  this->helpTextEdit->setEnabled( enable );
  this->ratingComboBox->setEnabled( q->isVisible() );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QAlderAtlasWidgetPrivate::rating()
{
  return this->ratingComboBox->currentIndex() + 1;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderAtlasWidget methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidget::QAlderAtlasWidget( QWidget* parent )
  : Superclass( parent )
  , d_ptr(new QAlderAtlasWidgetPrivate(*this))
{
  Q_D(QAlderAtlasWidget);
  d->setupUi(this);
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidget::~QAlderAtlasWidget()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::showEvent( QShowEvent* event )
{
  Q_D(QAlderAtlasWidget);
  QWidget::showEvent( event );

  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveImage();

  // select an appropriate atlas image, if necessary
  if( image )
  {
    bool getNewAtlasImage = false;

    vtkSmartPointer< Alder::Exam > exam;
    image->GetRecord( exam );
    Alder::Image *atlasImage = app->GetActiveAtlasImage();
    if( atlasImage )
    {
      vtkSmartPointer< Alder::Exam > atlasExam;
      atlasImage->GetRecord( atlasExam );
      if( exam->GetScanType() != atlasExam->GetScanType() ||
          exam->Get("Side").ToString() != atlasExam->Get("Side").ToString() )
        getNewAtlasImage = true;
    }
    else
      getNewAtlasImage = true;

    if( getNewAtlasImage )
    {
      vtkSmartPointer< Alder::Image > newAtlasImage =
        image->GetAtlasImage( d->rating() );
      app->SetActiveAtlasImage( newAtlasImage->Get( "Id" ).IsValid() ? newAtlasImage : NULL );
    }
  }
  d->updateUi();
  emit showing( true );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::hideEvent( QHideEvent* event )
{
  QWidget::hideEvent( event );
  Alder::Application *app = Alder::Application::GetInstance();
  app->SetActiveAtlasImage( NULL );
  emit showing( false );
}

// called when the application sets a new atlas image
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::update()
{
  if( !this->isVisible() ) return;
  Q_D(QAlderAtlasWidget);
  d->updateUi();
}
