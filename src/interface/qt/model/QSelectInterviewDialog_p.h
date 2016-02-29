/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectInterviewDialog_p.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QSelectInterviewDialog_p_h
#define __QSelectInterviewDialog_p_h

#include <QSelectInterviewDialog.h>
#include <ui_QSelectInterviewDialog.h>

namespace Alder { class Interview; };
class QTableWidgetItem;

class QSelectInterviewDialogPrivate : public QObject, public Ui_QSelectInterviewDialog
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QSelectInterviewDialog);
protected:
  QSelectInterviewDialog* const q_ptr;

public:
  explicit QSelectInterviewDialogPrivate(QSelectInterviewDialog& object);
  virtual ~QSelectInterviewDialogPrivate();

  void setupUi(QWidget*);
  void updateUi();

public Q_SLOTS:

  void sort(int);

  void search();

  void selectionChanged();

private:
  bool searchTextInUId( const QString& );
  void updateRow( const int&, Alder::Interview* );
  QStringList searchText;
  int sortColumn;
  Qt::SortOrder sortOrder;
  QMap<QString,int> columnIndex;
};

#endif
