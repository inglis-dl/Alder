/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderAtlasWidget_p.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QAlderAtlasWidget_p_h
#define __QAlderAtlasWidget_p_h

#include <QAlderAtlasWidget.h>
#include <ui_QAlderAtlasWidget.h>

// VTK includes
#include <vtkSmartPointer.h>

class vtkEventQtSlotConnect;

class QAlderAtlasWidgetPrivate : public QObject, public Ui_QAlderAtlasWidget
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QAlderAtlasWidget);
protected:
  QAlderAtlasWidget* const q_ptr;

public:
  explicit QAlderAtlasWidgetPrivate(QAlderAtlasWidget& object);
  virtual ~QAlderAtlasWidgetPrivate();

  void setupUi(QWidget*);
  void updateUi();
  int  rating();

public Q_SLOTS:

  void next();
  void previous();
  void ratingChanged();

private:
  vtkSmartPointer<vtkEventQtSlotConnect> qvtkConnection;
};

#endif
