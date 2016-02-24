/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderImageWidget.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QAlderImageWidget_h
#define __QAlderImageWidget_h

// Qt includes
#include <QWidget>

class QAlderImageWidgetPrivate;

class QAlderImageWidget : public QWidget
{
  Q_OBJECT

public:
  typedef QWidget Superclass;
  //constructor
  QAlderImageWidget( QWidget* parent = 0 );
  //destructor
  virtual ~QAlderImageWidget();

  void resetImage();
  void loadImage( const QString& filename );
  void saveImage( const QString& fileName );

protected:
  QScopedPointer<QAlderImageWidgetPrivate> d_ptr;

  bool eventFilter( QObject *, QEvent * );

private:
  Q_DECLARE_PRIVATE(QAlderImageWidget);
  Q_DISABLE_COPY(QAlderImageWidget);
};

#endif
