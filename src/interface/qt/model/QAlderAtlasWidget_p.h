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

namespace Alder {
class Image;
};

class QAlderAtlasWidgetPrivate : public QObject, public Ui_QAlderAtlasWidget
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QAlderAtlasWidget);
  protected:
    QAlderAtlasWidget* const q_ptr;

  public:
    explicit QAlderAtlasWidgetPrivate(QAlderAtlasWidget& object);
    virtual ~QAlderAtlasWidgetPrivate();

    void setupUi(QWidget* widget);
    void updateUi();
    int  rating();
    void updateRoot(const int& id);

  public slots:
    void next();
    void previous();
    void ratingChanged();

  private:
    vtkSmartPointer<vtkEventQtSlotConnect> qvtkConnection;

    vtkSmartPointer<Alder::Image> rootImage;
    vtkSmartPointer<Alder::Image> atlasImage;
};

#endif
