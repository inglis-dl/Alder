/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectWaveDialog_p.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QSelectWaveDialog_p_h
#define __QSelectWaveDialog_p_h

#include <QSelectWaveDialog.h>
#include <ui_QSelectWaveDialog.h>

// Qt includes
#include <QMap>

#include <vector>

class QTableWidgetItem;

class QSelectWaveDialogPrivate : public QObject, public Ui_QSelectWaveDialog
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QSelectWaveDialog);
protected:
  QSelectWaveDialog* const q_ptr;

public:
  explicit QSelectWaveDialogPrivate(QSelectWaveDialog& object);
  virtual ~QSelectWaveDialogPrivate();

  void setupUi(QDialog*);
  void buildSelection();

public slots:
  void sort(int);
  void itemPressed(QTableWidgetItem *);
  void itemClicked(QTableWidgetItem *);

private:
  QMap<QString,int> columnIndex;
  Qt::SortOrder sortOrder;
  int sortColumn;
  Qt::CheckState lastItemPressedState;
  std::vector<std::pair<int,bool>> selection;
};

#endif
