/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderApplication.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QAlderApplication_h
#define __QAlderApplication_h

// Qt includes
#include <QApplication>

class Ui_QAlderApplication;

class QAlderApplication : public QApplication
{
  public:
    QAlderApplication(int& argc, char** argv) : QApplication(argc, argv) {}
    bool notify(QObject* pObject, QEvent* pEvent);
};

#endif
