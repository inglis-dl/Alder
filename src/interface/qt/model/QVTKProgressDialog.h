/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QVTKProgressDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QVTKProgressDialog_h
#define __QVTKProgressDialog_h

#include <QDialog>

#include <vtkCommand.h>
#include <vtkSmartPointer.h>

class Ui_QVTKProgressDialog;

namespace Alder
{
  class BaseInterviewProgressFunc;
}  

class QVTKProgressDialog : public QDialog
{
  Q_OBJECT
private:
  class Command : public vtkCommand
  {
  public:
    static Command *New() { return new Command; }
    void Execute( vtkObject *caller, unsigned long eventId, void *callData );
    Ui_QVTKProgressDialog *ui;
    QWidget *parent;

  protected:
    Command() { this->ui = NULL; }
  };  

public:
  //constructor
  QVTKProgressDialog( QWidget* parent = 0 );
  //destructor
  ~QVTKProgressDialog();

  void setMessage( QString message );

  void Run( const std::string& title, const std::string& message
    /* , 
    Alder::BaseInterviewProgressFunc& func */ );
  
public slots:
  virtual void slotCancel();

protected:
  vtkSmartPointer< Command > observer;

protected slots:

private:
  // Designer form
  Ui_QVTKProgressDialog *ui;
};

#endif
