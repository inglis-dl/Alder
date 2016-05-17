/*=======================================================================

  Module:    QAlderSliceView_p.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QAlderAbstractView_p_h
#define __QAlderAbstractView_p_h

// Qt includes
#include <QObject>

// Alder includes
#include <QAlderAbstractView.h>

// VTK includes
#include <vtkCenteredAxesActor.h>
#include <QVTKWidget.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class QAlderAbstractViewPrivate : public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QAlderAbstractView);

  protected:
    QAlderAbstractView* const q_ptr;

  public:
    explicit QAlderAbstractViewPrivate(QAlderAbstractView& object);

    virtual void init();
    virtual void setupAxesWidget();

    bool axesOverView;

    QList<vtkRenderer*> renderers()const;
    vtkRenderer* firstRenderer()const;

    QVTKWidget*                                 VTKWidget;
    vtkSmartPointer<vtkRenderer>                Renderer;
    vtkSmartPointer<vtkRenderWindow>            RenderWindow;
    vtkSmartPointer<vtkCenteredAxesActor>       AxesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> OrientationMarkerWidget;
};

#endif
