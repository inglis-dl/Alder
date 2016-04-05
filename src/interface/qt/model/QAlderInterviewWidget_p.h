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

// Alder includes

// Qt includes
#include <QMap>
#include <QHash>

// VTK includes
#include <vtkSmartPointer.h>

namespace Alder { 
class ActiveRecord; 
class Interview;
class Modality;
class ParticipantData;
class User;
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
  void updateInfo();
  void updateTree();
  void updateRating();
  void updateViewer();
  void updateEnabled();
  void updateCodeList();
  void updatePermission();
  void updateSelected();

public slots:

  void next();
  void previous();
  void ratingChanged(int);
  void resetRating();
  void treeSelectionChanged();
  void noteChanged();
  void codeChanged(QTableWidgetItem*);
  void codeSelected();
  void derivedRatingToggle();
  void buildTree();

private:
  // mapping between tree widget items and Alder::Interview and Alder::Image records
  QHash<QTreeWidgetItem*, vtkSmartPointer<Alder::ActiveRecord>> treeModelMap;

  // connect VTK events to Qt slots
  vtkSmartPointer<vtkEventQtSlotConnect> qvtkConnection;

  // mapping between Alder::Wave records and tree widget items
  // sorted by Wave Name
  QMap<QString, QTreeWidgetItem*> waveLookup;

  // data containter class to facilitate building the tree widget
  // based on participant UID
  vtkSmartPointer<Alder::ParticipantData> participantData;

  // mapping between Alder::Modality names and current Alder::User permissions
  // sorted by Modalit Name
  QMap<QString, bool> modalityPermission;

  // method called by previous() and next() slots to change the interview
  void setActiveInterview( Alder::Interview* );
};

#endif
