/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QReportDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QReportDialog_h
#define __QReportDialog_h

#include <QDialog>
#include <QMailSender.h>
#include <map>

class QTableWidgetItem;
class Ui_QReportDialog;

class QReportDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QReportDialog( QWidget* parent = 0 );
  //destructor
  ~QReportDialog();

signals:

public slots:
  virtual void slotSend();
  virtual void slotClose();
  virtual void slotHeaderClicked( int );
  virtual void slotServerChanged( int );
  virtual void slotItemChanged( QTableWidgetItem* );

protected:
  bool buildReport();
  void updateInterface();

  int sortColumn;
  Qt::SortOrder sortOrder;
  std::map<std::string, int> columnIndex;
  std::map<int, std::string> columnText;
  QString currentReportFileName;
  QMailSender mailSender;

protected slots:

private:
  // Designer form
  Ui_QReportDialog *ui;

};

#endif
