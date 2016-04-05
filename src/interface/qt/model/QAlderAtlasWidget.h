/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderAtlasWidget.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QAlderAtlasWidget_h
#define __QAlderAtlasWidget_h

// Qt includes
#include <QWidget>

class QAlderAtlasWidgetPrivate;

class QAlderAtlasWidget : public QWidget
{
  Q_OBJECT

public:
  typedef QWidget Superclass;
  explicit QAlderAtlasWidget( QWidget* parent = 0 );
  virtual ~QAlderAtlasWidget();

  virtual void showEvent( QShowEvent* );
  virtual void hideEvent( QHideEvent* );

signals:
  void showing( bool );

public slots:
  virtual void loadImage( int id );

protected:
  QScopedPointer<QAlderAtlasWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(QAlderAtlasWidget);
  Q_DISABLE_COPY(QAlderAtlasWidget);
};

#endif
