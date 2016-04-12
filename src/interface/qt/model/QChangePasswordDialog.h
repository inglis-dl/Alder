/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QChangePasswordDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QChangePasswordDialog_h
#define __QChangePasswordDialog_h

// Qt includes
#include <QDialog>

class QChangePasswordDialogPrivate;

class QChangePasswordDialog : public QDialog
{
  Q_OBJECT

public:
  typedef QDialog Superclass;
  //constructor
  explicit QChangePasswordDialog( QWidget* parent = 0, const QString& pwd = "" );
  //destructor
  virtual ~QChangePasswordDialog();

public slots:
  virtual void accepted();

signals:
  void passwordChanged( const QString& pwd );

protected:
  QScopedPointer<QChangePasswordDialogPrivate> d_ptr;

  virtual bool eventFilter( QObject*, QEvent* );

private:
  Q_DECLARE_PRIVATE(QChangePasswordDialog);
  Q_DISABLE_COPY(QChangePasswordDialog);
};

#endif
