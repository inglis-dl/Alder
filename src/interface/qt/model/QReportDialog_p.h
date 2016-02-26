/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QReportDialog_p.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QReportDialog_p_h
#define __QReportDialog_p_h

// Alder includes
#include <QReportDialog.h>
#include <ui_QReportDialog.h>
#include <QMailSender.h>

class QTableWidgetItem;

class QReportDialogPrivate : public QObject, public Ui_QReportDialog
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QReportDialog);
protected:
  QReportDialog* const q_ptr;

public:
  explicit QReportDialogPrivate(QReportDialog& object);
  virtual ~QReportDialogPrivate();

  void setupUi(QWidget*);
  void updateUi();
  void setProgress(int);
  bool buildReportFile();

public Q_SLOTS:

  void sort(int);

  void send();

  void serverSelectionChanged(int);

  void modalitySelectionChanged(QTableWidgetItem*);

private:
  QMailSender mailSender;

  int sortColumn;
  Qt::SortOrder sortOrder;
  QMap<QString, int> columnIndex;
  QMap<int, QString> columnText;
  QString currentReportFileName;
  double percentBuild;
  double percentWrite;
};

#endif
