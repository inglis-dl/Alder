/*=======================================================================

  Module:    QAlderAbstractView.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QAlderAbstractView_h
#define __QAlderAbstractView_h

// Qt includes
#include <QWidget>

// VTK includes
#include <QVTKWidget.h>

class QAlderAbstractViewPrivate;

class vtkInteractorObserver;
class vtkRenderWindowInteractor;
class vtkRenderWindow;

class QAlderAbstractView : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool orientationDisplay READ orientationDisplay
    WRITE setOrientationDisplay)
  Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
  Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor)
  Q_PROPERTY(bool gradientBackground READ gradientBackground WRITE setGradientBackground)

  public:
    typedef QWidget Superclass;
    explicit QAlderAbstractView(QWidget* parent = 0);
    virtual ~QAlderAbstractView();

    vtkRenderer* renderer();

  public Q_SLOTS:
    virtual void forceRender();
    virtual void setForegroundColor(const QColor& qcolor);
    virtual void setBackgroundColor(const QColor& qcolor);
    virtual void setGradientBackground(bool enable);
    void setOrientationDisplay(bool display);

  public:
    bool orientationDisplay() const;
    vtkRenderWindow* renderWindow() const;
    vtkRenderWindowInteractor* interactor() const;
    virtual void setInteractor(vtkRenderWindowInteractor* interactor);
    vtkInteractorObserver* interactorStyle() const;
    QVTKWidget* VTKWidget() const;
    virtual QColor backgroundColor() const;
    virtual QColor foregroundColor() const;
    virtual bool gradientBackground() const;
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;
    virtual bool hasHeightForWidth() const;
    virtual int heightForWidth(int width) const;

  protected:
    QScopedPointer<QAlderAbstractViewPrivate> d_ptr;
    QAlderAbstractView(QAlderAbstractViewPrivate* pimpl, QWidget* parent);

  private:
    Q_DECLARE_PRIVATE(QAlderAbstractView);
    Q_DISABLE_COPY(QAlderAbstractView);
};

#endif
