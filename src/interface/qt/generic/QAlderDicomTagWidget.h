/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderDicomTagWidget.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QAlderDicomTagWidget_h
#define __QAlderDicomTagWidget_h

// Qt includes
#include <QWidget>

class QAlderDicomTagWidgetPrivate;

class QAlderDicomTagWidget : public QWidget
{
  Q_OBJECT

  public:
    typedef QWidget Superclass;
    explicit QAlderDicomTagWidget(QWidget* parent = 0);
    virtual ~QAlderDicomTagWidget();

  public Q_SLOTS:
    virtual void load(const QString& aFileName);

  protected:
    QScopedPointer<QAlderDicomTagWidgetPrivate> d_ptr;

  private:
    Q_DECLARE_PRIVATE(QAlderDicomTagWidget);
    Q_DISABLE_COPY(QAlderDicomTagWidget);
};

#endif
