/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QMedicalImageWidget.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QMedicalImageWidget_h
#define __QMedicalImageWidget_h

// Qt includes
#include <QWidget>

// VTK includes
#include <vtkSmartPointer.h>

class vtkMedicalImageViewer;
class Ui_QMedicalImageWidget;

class QMedicalImageWidget : public QWidget
{
  Q_OBJECT

public:
  //constructor
  QMedicalImageWidget( QWidget* parent = 0 );
  //destructor
  ~QMedicalImageWidget();

  void resetImage();
  void loadImage( QString filename );
  void saveImage( const QString& fileName );
  vtkMedicalImageViewer* GetViewer();

public Q_SLOTS:

  void slotSelectColor();
  void slotFlipVertical();
  void slotFlipHorizontal();
  void slotRotateClockwise();
  void slotRotateCounterClockwise();
  void slotInvertWindowLevel();
  void slotInterpolationToggle();

protected:
  void updateInterface();

  bool eventFilter( QObject *, QEvent * );

  vtkSmartPointer<vtkMedicalImageViewer> viewer;

private:
  // Designer form
  Ui_QMedicalImageWidget *ui;
};

#endif
