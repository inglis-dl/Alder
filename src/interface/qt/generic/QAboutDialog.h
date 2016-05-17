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

class QAboutDialogPrivate;

class QAboutDialog : public QDialog
{
  Q_OBJECT

  public:
    typedef QDialog Superclass;
    // constructor
    explicit QAboutDialog(QWidget* parent = 0);
    // destructor
    virtual ~QAboutDialog();

  public slots:

  protected:
    QScopedPointer<QAboutDialogPrivate> d_ptr;

  protected slots:

  private:
    Q_DECLARE_PRIVATE(QAboutDialog);
    Q_DISABLE_COPY(QAboutDialog);
};

#endif
