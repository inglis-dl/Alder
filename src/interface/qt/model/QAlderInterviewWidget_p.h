/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderInterviewWidget_p.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QAlderInterviewWidget_p_h
#define __QAlderInterviewWidget_p_h

#include <QAlderInterviewWidget.h>
#include <ui_QAlderInterviewWidget.h>

// Qt includes
#include <QMap>

// VTK includes
#include <vtkSmartPointer.h>

namespace Alder { 
class ActiveRecord; 
class Interview;
};

class QTableWidgetItem;
class QTreeWidgetItem;
class vtkEventQtSlotConnect;

class QAlderInterviewWidgetPrivate : public QObject, public Ui_QAlderInterviewWidget
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QAlderInterviewWidget);
protected:
  QAlderInterviewWidget* const q_ptr;

public:
  explicit QAlderInterviewWidgetPrivate(QAlderInterviewWidget& object);
  virtual ~QAlderInterviewWidgetPrivate();

  void setupUi(QWidget*);
  void setActiveInterview( Alder::Interview* );
  void updateInfo();
  void updateExamTreeWidget();
  void updateRating();
  void updateViewer();
  void updateEnabled();
  void updateCodeList();

public Q_SLOTS:

  void next();
  void previous();
  void ratingChanged(int);
  void resetRating();
  void treeSelectionChanged();
  void noteChanged();
  void codeChanged(QTableWidgetItem*);
  void codeSelected();
  void derivedRatingToggle();

private:
  QMap<QTreeWidgetItem*, vtkSmartPointer<Alder::ActiveRecord>> treeModelMap;
  vtkSmartPointer<vtkEventQtSlotConnect> qvtkConnection;
  QMap<QString, QTreeWidgetItem*> modalityLookup;
};

#endif
