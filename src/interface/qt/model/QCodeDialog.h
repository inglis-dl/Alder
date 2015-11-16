/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QCodeDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QCodeDialog_h
#define __QCodeDialog_h

#include <QDialog>
#include <map>

class QTableWidgetItem;
class Ui_QCodeDialog;

class QCodeDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QCodeDialog( QWidget* parent = 0 );
  //destructor
  ~QCodeDialog();

signals:

public slots:

  void slotClose();

  void slotHeaderClicked(int index);
  void slotTableDoubleClicked();
  void slotTabChanged();

  void slotGroupSelectionChanged();
  void slotGroupAdd();
  void slotGroupEdit();
  void slotGroupRemove();
  void slotGroupApply();

  void slotCodeSelectionChanged();
  void slotCodeAdd();
  void slotCodeEdit();
  void slotCodeRemove();
  void slotCodeApply();

protected:
  void updateInterface();
  void updateGroupTable();
  void updateCodeTable();

protected slots:

private:
  // Designer form
  Ui_QCodeDialog *ui;

  std::map< std::string, std::map< std::string, int > > columnIndex;
  std::map< std::string, std::map< int, Qt::SortOrder > > sortOrder;
};

#endif
