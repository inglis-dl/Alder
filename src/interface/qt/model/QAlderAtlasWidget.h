/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderAtlasWidget.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QAlderAtlasWidget_h
#define __QAlderAtlasWidget_h

#include <QWidget>

#include <vtkSmartPointer.h>

class vtkEventQtSlotConnect;
class Ui_QAlderAtlasWidget;

class QAlderAtlasWidget : public QWidget
{
  Q_OBJECT

public:
  QAlderAtlasWidget( QWidget* parent = 0 );
  ~QAlderAtlasWidget();

  virtual void showEvent( QShowEvent* );
  virtual void hideEvent( QHideEvent* );

signals:
  void showing( bool );
  
public slots:
  virtual void slotPrevious();
  virtual void slotNext();
  virtual void slotRatingChanged( int );
  virtual void updateInfo();
  virtual void updateViewer();
  virtual void updateAtlasImage();
  virtual void updateEnabled();

protected slots:

private:
  // Designer form
  Ui_QAlderAtlasWidget *ui;

  vtkSmartPointer<vtkEventQtSlotConnect> Connections;
};

#endif
