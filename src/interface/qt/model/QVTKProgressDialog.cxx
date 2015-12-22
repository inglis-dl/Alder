/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QVTKProgressDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QVTKProgressDialog.h>
#include <ui_QVTKProgressDialog.h>

#include <Application.h>
#include <Interview.h>
#include <ProgressProxy.h>

#include <utility>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QVTKProgressDialog::QVTKProgressDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QVTKProgressDialog;
  this->ui->setupUi( this );

  QObject::connect(
    this->ui->buttonBox, SIGNAL( rejected() ),
    this, SLOT( slotCancel() ) );

  this->observer = vtkSmartPointer< Command >::New();
  this->observer->ui = this->ui;

  Alder::Application *app = Alder::Application::GetInstance();
  app->AddObserver( vtkCommand::ConfigureEvent, this->observer );
  app->AddObserver( vtkCommand::StartEvent, this->observer );
  app->AddObserver( vtkCommand::ProgressEvent, this->observer );
  app->AddObserver( vtkCommand::EndEvent, this->observer );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QVTKProgressDialog::~QVTKProgressDialog()
{
  Alder::Application::GetInstance()->RemoveObserver( this->observer );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QVTKProgressDialog::setMessage( QString message )
{
  this->ui->label->setText( message );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QVTKProgressDialog::slotCancel()
{
  Alder::Application::GetInstance()->SetAbortFlag( true );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QVTKProgressDialog::Command::Execute(
  vtkObject *caller, unsigned long eventId, void *callData )
{
  if( vtkCommand::ConfigureEvent == eventId )
  { // configure the progress bar
    // pair contains int, bool: < progress bar type, show as busy? >
    std::pair<int, bool> progressConfig = *( static_cast< std::pair<int, bool>* >( callData ) );
    QProgressBar* progressBar =
      Alder::ProgressProxy::Global == progressConfig.first ?
      this->ui->globalProgressBar : this->ui->localProgressBar;
    int max = progressConfig.second ? 0 : 100;
    if( max != progressBar->maximum() )
    {
      progressBar->setRange( 0, max );
    }
    progressBar->parentWidget()->repaint();
  }
  else if( vtkCommand::StartEvent == eventId )
  { // set the progress to 0
    // which progress bar?
    int progressType = *( static_cast< int* >( callData ) );
    QProgressBar* progressBar =
      Alder::ProgressProxy::Global == progressType ?
      this->ui->globalProgressBar : this->ui->localProgressBar;
    progressBar->reset();
  }
  else if( vtkCommand::ProgressEvent == eventId )
  { // set the progress to the call data
    // pair contains int and double; < progress bar type, progress value >
    std::pair<int, double> progressConfig = *( static_cast< std::pair<int, double>* >( callData ) );
    QProgressBar* progressBar =
      Alder::ProgressProxy::Global == progressConfig.first ?
      this->ui->globalProgressBar : this->ui->localProgressBar;
    this->ui->localProgressBar->repaint();
    if( 0 < progressBar->maximum() )
    {
      this->ui->localProgressBar->repaint();
      progressBar->setValue(
        static_cast<int>( 100 * progressConfig.second ) );
    }
  }
  else if( vtkCommand::EndEvent == eventId )
  { // set the progress to 100
    // which progress bar?
    int progressType = *( static_cast< int* >( callData ) );
    QProgressBar* progressBar =
      Alder::ProgressProxy::Global == progressType ?
      this->ui->globalProgressBar : this->ui->localProgressBar;
    if( 0 < progressBar->maximum() )
      progressBar->setValue( 100 );
  }
  QApplication::processEvents();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QVTKProgressDialog::Run( const std::string& title, const std::string& message
  /*,
  Alder::BaseInterviewProgressFunc& func*/ )
{
  this->setWindowTitle( tr( title.c_str() ) );
  this->setMessage( tr( message.c_str() ) );
  this->setModal(true);
  this->show();
  //func.progressFunc();
  this->accept();
}
