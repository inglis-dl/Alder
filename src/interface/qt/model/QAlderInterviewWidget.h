/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderInterviewWidget.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QAlderInterviewWidget_h
#define __QAlderInterviewWidget_h

// Qt includes
#include <QWidget>

class QAlderInterviewWidgetPrivate;

class QAlderInterviewWidget : public QWidget
{
  Q_OBJECT

public:
  typedef QWidget Superclass;
  explicit QAlderInterviewWidget( QWidget* parent = 0 );
  virtual ~QAlderInterviewWidget();

  void saveImage( const QString& fileName );

public slots:
  void hideControls( bool );
  void activeInterviewChanged();
  void activeImageChanged();

protected:
  QScopedPointer<QAlderInterviewWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(QAlderInterviewWidget);
  Q_DISABLE_COPY(QAlderInterviewWidget);
};

#endif
