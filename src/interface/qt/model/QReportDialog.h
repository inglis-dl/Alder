/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QReportDialog.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QReportDialog_h
#define __QReportDialog_h

// Qt includes
#include <QDialog>

class QReportDialogPrivate;
class QTableWidgetItem;

class QReportDialog : public QDialog
{
  Q_OBJECT
  Q_PROPERTY(double percentBuild READ percentBuild CONSTANT)
  Q_PROPERTY(double percentWrite READ percentWrite CONSTANT)

public:
  typedef QDialog Superclass;
  //constructor
  QReportDialog( QWidget* parent = 0 );
  //destructor
  ~QReportDialog();

  double percentBuild() const;
  double percentWrite() const;

public slots:
  virtual void send();
  virtual void close();
  virtual void sortColumn( int );
  virtual void serverSelectionChanged( int );
  virtual void modalitySelectionChanged( QTableWidgetItem* );

protected:
  QScopedPointer<QReportDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(QReportDialog);
  Q_DISABLE_COPY(QReportDialog);
};

#endif
