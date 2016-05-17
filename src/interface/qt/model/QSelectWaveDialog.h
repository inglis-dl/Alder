/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectWaveDialog.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QSelectWaveDialog_h
#define __QSelectWaveDialog_h

// Qt includes
#include <QDialog>

// c++ includes
#include <utility>
#include <vector>

class QSelectWaveDialogPrivate;

class QSelectWaveDialog : public QDialog
{
  Q_OBJECT

  public:
    typedef QDialog Superclass;
    // constructor
    explicit QSelectWaveDialog(QWidget* parent = 0);
    // destructor
    virtual ~QSelectWaveDialog();

    std::vector<std::pair<int, bool>> selection();

  public slots:
    void close();

  protected:
    QScopedPointer<QSelectWaveDialogPrivate> d_ptr;

  private:
    Q_DECLARE_PRIVATE(QSelectWaveDialog);
    Q_DISABLE_COPY(QSelectWaveDialog);
};

#endif
