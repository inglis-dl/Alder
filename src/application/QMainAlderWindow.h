/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QMainAlderWindow.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QMainAlderWindow_h
#define __QMainAlderWindow_h

#include <QMainWindow>

#include <vtkSmartPointer.h>

class vtkEventQtSlotConnect;
class vtkObject;
class Ui_QMainAlderWindow;
class QAlderDicomTagWidget;

class QMainAlderWindow : public QMainWindow
{
  Q_OBJECT

public:
  QMainAlderWindow( QWidget* parent = 0 );
  ~QMainAlderWindow();

public slots:
  // action event functions
  virtual void slotOpenInterview();
  virtual void slotShowAtlas();
  virtual void slotShowDicomTags();
  virtual void slotLogin();
  virtual void slotChangePassword();
  virtual void slotLoadUIDs();
  virtual void slotUserManagement();
  virtual void slotUpdateDatabase();
  virtual void slotSaveImage();

  // help event functions
  virtual void slotAbout();
  virtual void slotManual();

  virtual void updateDicomTagWidget();
  void changeActiveUserPassword( QString );

protected:
  // called whenever the main window is closed
  virtual void closeEvent( QCloseEvent *event );

  // read/write application GUI settings
  virtual void readSettings();
  virtual void writeSettings();
  
  virtual void updateInterface();

private:
  // Designer form
  Ui_QMainAlderWindow *ui;

  bool atlasVisible;
  bool dicomTagsVisible;
  QAlderDicomTagWidget* DicomTagWidget;
  QString lastSavePath;

  void adminLoadUIDs();
  void adminUserManagement();
  void adminUpdateDatabase();
  void adminLoginDo( void (QMainAlderWindow::*fn)() );

  vtkSmartPointer<vtkEventQtSlotConnect> Connections;
};

#endif
