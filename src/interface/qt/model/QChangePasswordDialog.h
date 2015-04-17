/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QChangePasswordDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QChangePasswordDialog_h
#define __QChangePasswordDialog_h

#include <QDialog>

class Ui_QChangePasswordDialog;

class QChangePasswordDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QChangePasswordDialog( QString const &pwd, QWidget* parent = 0 );
  //destructor
  ~QChangePasswordDialog();
  
public slots:
  virtual void slotAccepted();

signals:
  void passwordChange( QString const &pwd );

protected:
  bool checkNewPassword();
  bool confirmNewPassword();
  virtual bool eventFilter( QObject*, QEvent* );

protected slots:

private:
  // Designer form
  Ui_QChangePasswordDialog *ui;
  QString originalPassword;
  QString newPassword;
};

#endif
