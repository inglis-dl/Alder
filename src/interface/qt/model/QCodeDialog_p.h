/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QCodeDialog_p.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QCodeDialog_p_h
#define __QCodeDialog_p_h

#include <QCodeDialog.h>
#include <ui_QCodeDialog.h>

#include <QMap>

class QTableWidgetItem;

class QCodeDialogPrivate : public QObject, public Ui_QCodeDialog
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QCodeDialog);
protected:
  QCodeDialog* const q_ptr;

public:
  explicit QCodeDialogPrivate(QCodeDialog& object);
  virtual ~QCodeDialogPrivate();

  void setupUi(QDialog*);
  void updateGroupUi();
  void updateCodeUi();
  void updateUi();

public slots:
  void sort(int);
  void tableDoubleClicked(QTableWidgetItem*);
  void tabChanged();
  void selectedGroupChanged();
  void selectedCodeChanged();
  void addGroup();
  void addCode();
  void applyGroup();
  void applyCode();
  void removeGroup();
  void removeCode();
  void editGroup();
  void editCode();

private:
  QMap<QString, QMap<QString, int>> columnIndex;
  QMap<QString, QMap<int, Qt::SortOrder>> sortOrder;
};

#endif
