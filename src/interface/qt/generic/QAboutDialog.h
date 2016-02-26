/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAboutDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QAboutDialog_h
#define __QAboutDialog_h

// Qt includes
#include <QDialog>

class Ui_QAboutDialog;

class QAboutDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QAboutDialog( QWidget* parent = 0 );
  //destructor
  ~QAboutDialog();
  
public slots:

protected:

protected slots:

private:
  // Designer form
  Ui_QAboutDialog *ui;
};

#endif
