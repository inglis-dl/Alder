/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QLoginDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QLoginDialog_h
#define __QLoginDialog_h

// Qt includes
#include <QDialog>

class QLoginDialogPrivate;

class QLoginDialog : public QDialog
{
  Q_OBJECT

public:
  typedef QDialog Superclass;
  //constructor
  explicit QLoginDialog( QWidget* parent = 0 );
  //destructor
  virtual ~QLoginDialog();
  
public slots:
  virtual void accepted();

protected:
  QScopedPointer<QLoginDialogPrivate> d_ptr;

protected slots:

private:
  Q_DECLARE_PRIVATE(QLoginDialog);
  Q_DISABLE_COPY(QLoginDialog);
};

#endif
