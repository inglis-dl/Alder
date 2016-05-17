/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QCodeDialog.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QCodeDialog_h
#define __QCodeDialog_h

// Qt includes
#include <QDialog>

class QCodeDialogPrivate;

class QCodeDialog : public QDialog
{
  Q_OBJECT

  public:
    typedef QDialog Superclass;
    // constructor
    explicit QCodeDialog(QWidget* parent = 0);
    // destructor
    virtual ~QCodeDialog();

  public slots:
    void close();

  protected:
    QScopedPointer<QCodeDialogPrivate> d_ptr;

  private:
    Q_DECLARE_PRIVATE(QCodeDialog);
    Q_DISABLE_COPY(QCodeDialog);
};

#endif
