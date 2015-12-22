/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectInterviewDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QSelectInterviewDialog_h
#define __QSelectInterviewDialog_h

#include <QDialog>

#include <map>
#include <string>

namespace Alder { class Interview; };
class Ui_QSelectInterviewDialog;

class QSelectInterviewDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QSelectInterviewDialog( QWidget* parent = 0 );
  //destructor
  ~QSelectInterviewDialog();
  
public slots:
  virtual void slotSearch();
  virtual void slotAccepted();
  virtual void slotSelectionChanged();
  virtual void slotHeaderClicked( int index );

protected:
  void updateRow( const int&, Alder::Interview* );
  void updateInterface();
  bool searchTextInUId( const QString& );
  QStringList searchText;
  int sortColumn;
  Qt::SortOrder sortOrder;
  std::map< std::string, int > columnIndex;

protected slots:

private:
  // Designer form
  Ui_QSelectInterviewDialog *ui;
};

#endif
