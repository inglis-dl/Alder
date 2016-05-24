/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectInterviewDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QSelectInterviewDialog_h
#define __QSelectInterviewDialog_h

// Qt includes
#include <QDialog>

class QSelectInterviewDialogPrivate;

class QSelectInterviewDialog : public QDialog
{
  Q_OBJECT

  public:
    typedef QDialog Superclass;
    // constructor
    explicit QSelectInterviewDialog(QWidget* parent = 0);
    // destructor
    virtual ~QSelectInterviewDialog();

  public slots:
    virtual void accepted();

  signals:
    void interviewSelected(int id);

  protected:
    QScopedPointer<QSelectInterviewDialogPrivate> d_ptr;

  private:
    Q_DECLARE_PRIVATE(QSelectInterviewDialog);
    Q_DISABLE_COPY(QSelectInterviewDialog);
};

#endif
