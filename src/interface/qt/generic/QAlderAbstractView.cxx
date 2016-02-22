/*=======================================================================

I  Module:    QAlderAbstractView.cxx
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include "QAlderAbstractView.h"
#include "QAlderAbstractView_p.h"

// Qt includes
#include <QVBoxLayout>
#include <QDebug>

// VTK includes
#include <vtkOpenGLRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindowInteractor.h>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// QAlderAbstractViewPrivate methods

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAbstractViewPrivate::QAlderAbstractViewPrivate(QAlderAbstractView& object)
  : q_ptr(&object)
{
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  this->AxesActor = vtkSmartPointer<vtkAxesActor>::New();
  this->OrientationMarkerWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
  this->OrientationMarkerWidget->SetOrientationMarker( this->AxesActor );
  this->OrientationMarkerWidget->KeyPressActivationOff();
  this->OrientationMarkerWidget->SetViewport( 0.8, 0.0, 1.0, 0.2 );
  this->OrientationMarkerWidget->InteractiveOff();
  this->axesOverView = true;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAbstractViewPrivate::init()
{
  Q_Q(QAlderAbstractView);

  this->setParent(q);

  this->VTKWidget = new QVTKWidget;
  q->setLayout(new QVBoxLayout);
  q->layout()->setMargin(0);
  q->layout()->setSpacing(0);
  q->layout()->addWidget(this->VTKWidget);

  this->setupRendering();

  q->setInteractor(this->RenderWindow->GetInteractor());
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+--
void QAlderAbstractViewPrivate::setupRendering()
{
  Q_ASSERT(this->RenderWindow);
  Q_ASSERT(this->Renderer);
  this->RenderWindow->AddRenderer(this->Renderer);
  this->VTKWidget->SetRenderWindow(this->RenderWindow);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+--
void QAlderAbstractViewPrivate::setupAxesWidget()
{
  Q_ASSERT(this->RenderWindow);
  Q_ASSERT(this->Renderer);
  if( this->axesOverView )
  {
    this->OrientationMarkerWidget->SetDefaultRenderer( this->Renderer );
    this->OrientationMarkerWidget->SetInteractor( this->RenderWindow->GetInteractor() );
    this->OrientationMarkerWidget->On();
  }
  else
  {
    this->OrientationMarkerWidget->Off();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+--
QList<vtkRenderer*> QAlderAbstractViewPrivate::renderers()const
{
  QList<vtkRenderer*> rendererList;

  vtkRendererCollection* rendererCollection = this->RenderWindow->GetRenderers();
  vtkCollectionSimpleIterator rendererIterator;
  rendererCollection->InitTraversal(rendererIterator);
  vtkRenderer *renderer;
  while ( (renderer= rendererCollection->GetNextRenderer(rendererIterator)) )
  {
    rendererList << renderer;
  }
  return rendererList;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+--
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkRenderer* QAlderAbstractViewPrivate::firstRenderer()const
{
  return static_cast<vtkRenderer*>(this->RenderWindow->GetRenderers()
    ->GetItemAsObject(0));
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// QAlderAbstractView methods

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAbstractView::QAlderAbstractView(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new QAlderAbstractViewPrivate(*this))
{
  Q_D(QAlderAbstractView);
  d->init();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAbstractView::QAlderAbstractView(QAlderAbstractViewPrivate* pimpl, QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(pimpl)
{
  // derived classes must call init manually. Calling init() here may results in
  // actions on a derived public class not yet finished to be created
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAbstractView::~QAlderAbstractView()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAbstractView::forceRender()
{
  Q_D(QAlderAbstractView);

  if (!this->isVisible())
  {
    return;
  }
  d->RenderWindow->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkRenderWindow* QAlderAbstractView::renderWindow()const
{
  Q_D(const QAlderAbstractView);
  return d->RenderWindow;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QAlderAbstractView::setInteractor(vtkRenderWindowInteractor* newInteractor)
{
  Q_D(QAlderAbstractView);
  if( newInteractor != d->RenderWindow->GetInteractor() )
    d->RenderWindow->SetInteractor(newInteractor);

  if( newInteractor != d->OrientationMarkerWidget->GetInteractor() )
    d->OrientationMarkerWidget->SetInteractor(newInteractor);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
vtkRenderWindowInteractor* QAlderAbstractView::interactor()const
{
  Q_D(const QAlderAbstractView);
  return d->RenderWindow->GetInteractor();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QAlderAbstractView::setOrientationDisplay(bool display)
{
  Q_D(QAlderAbstractView);
  d->OrientationMarkerWidget->SetEnabled(display);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
bool QAlderAbstractView::orientationDisplay() const
{
  Q_D(const QAlderAbstractView);
  return d->OrientationMarkerWidget->GetEnabled();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
vtkInteractorObserver* QAlderAbstractView::interactorStyle()const
{
  return this->interactor() ?
    this->interactor()->GetInteractorStyle() : 0;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QVTKWidget * QAlderAbstractView::VTKWidget() const
{
  Q_D(const QAlderAbstractView);
  return d->VTKWidget;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QSize QAlderAbstractView::minimumSizeHint()const
{
  // Arbitrary size. 50x50 because smaller seems too small.
  return QSize(50, 50);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QSize QAlderAbstractView::sizeHint()const
{
  // Arbitrary size. 300x300 is the default vtkRenderWindow size.
  return QSize(300, 300);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
bool QAlderAbstractView::hasHeightForWidth()const
{
  return true;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
int QAlderAbstractView::heightForWidth(int width)const
{
  // typically VTK render window tend to be square...
  return width;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QAlderAbstractView::setBackgroundColor(const QColor& newBackgroundColor)
{
  Q_D(QAlderAbstractView);
  double color[3];
  color[0] = newBackgroundColor.redF();
  color[1] = newBackgroundColor.greenF();
  color[2] = newBackgroundColor.blueF();
  foreach(vtkRenderer* renderer, d->renderers())
  {
    renderer->SetBackground(color);
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
vtkRenderer* QAlderAbstractView::renderer()
{
  Q_D(const QAlderAbstractView);
  return d->firstRenderer();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QColor QAlderAbstractView::backgroundColor()const
{
  Q_D(const QAlderAbstractView);
  vtkRenderer* firstRenderer = d->firstRenderer();
  return firstRenderer ? QColor::fromRgbF(firstRenderer->GetBackground()[0],
                                          firstRenderer->GetBackground()[1],
                                          firstRenderer->GetBackground()[2])
                       : QColor();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QAlderAbstractView::setBackgroundColor2(const QColor& newBackgroundColor)
{
  Q_D(QAlderAbstractView);
  double color[3];
  color[0] = newBackgroundColor.redF();
  color[1] = newBackgroundColor.greenF();
  color[2] = newBackgroundColor.blueF();
  foreach(vtkRenderer* renderer, d->renderers())
  {
    renderer->SetBackground2(color);
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
QColor QAlderAbstractView::backgroundColor2()const
{
  Q_D(const QAlderAbstractView);
  vtkRenderer* firstRenderer = d->firstRenderer();
  return firstRenderer ? QColor::fromRgbF(firstRenderer->GetBackground2()[0],
                                          firstRenderer->GetBackground2()[1],
                                          firstRenderer->GetBackground2()[2])
                       : QColor();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
void QAlderAbstractView::setGradientBackground(bool enable)
{
  Q_D(QAlderAbstractView);
  foreach(vtkRenderer* renderer, d->renderers())
  {
    renderer->SetGradientBackground(enable);
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+---
bool QAlderAbstractView::gradientBackground()const
{
  Q_D(const QAlderAbstractView);
  vtkRenderer* firstRenderer = d->firstRenderer();
  return firstRenderer ? firstRenderer->GetGradientBackground() : false;
}
