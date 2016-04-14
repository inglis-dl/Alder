/*=======================================================================

  Module:    QAlderSliceView_p.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QAlderSliceView_p_h
#define __QAlderSliceView_p_h

// Qt includes
#include <QObject>

// Alder includes
#include "QAlderSliceView.h"
#include "QAlderAbstractView_p.h"
#include <vtkCustomCornerAnnotation.h>
#include <vtkCustomInteractorStyleImage.h>
#include <vtkImageCoordinateWidget.h>
#include <vtkImageWindowLevel.h>

// VTK includes
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

#include <map>

class QAlderSliceViewPrivate : public QAlderAbstractViewPrivate
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QAlderSliceView);

public:
  QAlderSliceViewPrivate(QAlderSliceView& object);

  void setColorWindowLevel( const double&, const double& );
  void flipCameraVertical();
  void flipCameraHorizontal();
  void rotateCamera( const double& );
  void setSlice( const int& );
  void setImageData( vtkImageData* );
  void setOrientation( const QAlderSliceView::Orientation& );
  void setInterpolation( const int& );
  int sliceMin();
  int sliceMax();

  void doResetWindowLevelEvent();
  void doStartWindowLevelEvent();
  void doWindowLevelEvent();

  vtkSmartPointer<vtkCustomCornerAnnotation>     CornerAnnotation;
  vtkSmartPointer<vtkImageSlice>                 ImageSlice;
  vtkSmartPointer<vtkImageSliceMapper>           ImageSliceMapper;
  vtkSmartPointer<vtkImageCoordinateWidget>      CoordinateWidget;
  vtkSmartPointer<vtkCustomInteractorStyleImage> InteractorStyle;
  vtkSmartPointer<vtkImageWindowLevel>           WindowLevel;

  void setupRendering( const bool& display, const bool& initCamera = false );
  void setupCoordinateWidget();
  void setupCornerAnnotation();

  bool annotateOverView;
  bool cursorOverView;
  int slice;
  QAlderSliceView::Orientation orientation;
  int dimensionality;
  int interpolation;
  int frameRate;

private:
  int lastSlice[3];
  double cameraPosition[3][3];      /**< Current camera position */
  double cameraFocalPoint[3][3];    /**< Current camera focal point */
  double cameraViewUp[3][3];        /**< Current camera view up */
  double cameraParallelScale[3];    /**< Current camera parallel scale (zoom) */
  double cameraDistance[3];         /**< Current camera distance */
  double cameraClippingRange[3][2]; /**< Current clipping range */

  double originalColorWindow;
  double originalColorLevel;
  double initialColorWindow;
  double initialColorLevel;

  int* sliceRange();

  void computeCameraFromCurrentSlice( const bool& useCamera = true );
  void updateCameraView();
  void initializeWindowLevel();
  void initializeCameraViews();
  void recordCameraView( const int& specified = -1 );

  std::map< std::string, unsigned long > callbackTags;
};

#endif
