/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QMainAlderWindow.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#ifndef __QMainAlderWindow_h
#define __QMainAlderWindow_h

// Qt includes
#include <QMainWindow>

class QMainAlderWindowPrivate;

class QMainAlderWindow : public QMainWindow
{
  Q_OBJECT

  public:
    typedef QMainWindow Superclass;
    explicit QMainAlderWindow(QWidget* parent = 0);
    virtual ~QMainAlderWindow();

  protected:
    QScopedPointer<QMainAlderWindowPrivate> d_ptr;

    // called whenever the main window is closed
    virtual void closeEvent(QCloseEvent* event);

    // read/write application GUI settings
    virtual void readSettings();
    virtual void writeSettings();

  private:
    Q_DECLARE_PRIVATE(QMainAlderWindow);
    Q_DISABLE_COPY(QMainAlderWindow);

    void adminUserManagement();
    void adminUpdateDatabase();
    void adminReports();
    void adminRatingCodes();
    void adminLoginDo(void (QMainAlderWindow::*fn)());
};

#endif
