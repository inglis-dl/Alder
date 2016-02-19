/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderImageWidget.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QAlderImageWidget_h
#define __QAlderImageWidget_h

#include <QWidget>

class Ui_QAlderImageWidget;

class QAlderImageWidget : public QWidget
{
  Q_OBJECT

public:
  //constructor
  QAlderImageWidget( QWidget* parent = 0 );
  //destructor
  ~QAlderImageWidget();

  void resetImage();
  void loadImage( const QString& filename );
  void saveImage( const QString& fileName );

public slots:

  void slotSelectColor();
  void slotInterpolationToggle();

protected:
  void updateInterface();

  bool eventFilter( QObject *, QEvent * );

protected slots:

private:
  // Designer form
  Ui_QAlderImageWidget *ui;
};

#endif
