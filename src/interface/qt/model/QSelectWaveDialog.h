/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectWaveDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QSelectWaveDialog_h
#define __QSelectWaveDialog_h

#include <QDialog>
#include <QTableWidgetItem>
#include <map>
#include <vector>

class Ui_QSelectWaveDialog;

class QSelectWaveDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QSelectWaveDialog( QWidget* parent = 0 );
  //destructor
  ~QSelectWaveDialog();

  std::vector< std::pair< int, bool > > getSelection() { return this->selection; };

public slots:
  virtual void slotAccepted();
  virtual void slotHeaderClicked( int index );
  virtual void slotItemPressed( QTableWidgetItem * );
  virtual void slotItemClicked( QTableWidgetItem * );

protected:
  int sortColumn;
  Qt::SortOrder sortOrder;
  std::map< std::string, int > columnIndex;

protected slots:

private:
  // Designer form
  Ui_QSelectWaveDialog *ui;

  std::vector< std::pair< int, bool > > selection;
  Qt::CheckState lastItemPressedState;
};

#endif
