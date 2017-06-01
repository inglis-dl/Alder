/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QMainAlderWindow.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QMainAlderWindow.h>
#include <QMainAlderWindow_p.h>

// Alder includes
#include <Application.h>
#include <Common.h>
#include <Configuration.h>
#include <Interview.h>
#include <User.h>
#include <Utilities.h>
#include <Wave.h>

#include <QAboutDialog.h>
#include <QAlderDicomTagWidget.h>
#include <QChangePasswordDialog.h>
#include <QCodeDialog.h>
#include <QLoginDialog.h>
#include <QReportDialog.h>
#include <QSelectInterviewDialog.h>
#include <QSelectWaveDialog.h>
#include <QUserListDialog.h>

// VTK includes
#include <vtkEventQtSlotConnect.h>
#include <vtkNew.h>

// Qt includes
#include <QCloseEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QTextStream>

// C++ includes
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QMainAlderWindowPrivate methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindowPrivate::QMainAlderWindowPrivate(QMainAlderWindow& object)
  : QObject(&object), q_ptr(&object)
{
  this->qvtkConnection = vtkSmartPointer<vtkEventQtSlotConnect>::New();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindowPrivate::~QMainAlderWindowPrivate()
{
  this->qvtkConnection->Disconnect();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::setupUi(QMainWindow* window)
{
  Q_Q(QMainAlderWindow);

  this->Ui_QMainAlderWindow::setupUi(window);

  // connect the menu items
  QObject::connect(
    this->actionOpenInterview, SIGNAL(triggered()),
    this, SLOT(openInterview()));
  QObject::connect(
    this->actionShowAtlas, SIGNAL(triggered()),
    this, SLOT(showAtlas()));
  QObject::connect(
    this->actionShowDicomTags, SIGNAL(triggered()),
    this, SLOT(showDicomTags()));
  QObject::connect(
    this->actionLogin, SIGNAL(triggered()),
    this, SLOT(login()));
  QObject::connect(
    this->actionChangePassword, SIGNAL(triggered()),
    this, SLOT(changePassword()));
  QObject::connect(
    this->actionUserManagement, SIGNAL(triggered()),
    this, SLOT(userManagement()));
  QObject::connect(
    this->actionUpdateInterviewDatabase, SIGNAL(triggered()),
    this, SLOT(updateInterviewDatabase()));
  QObject::connect(
    this->actionUpdateWaveDatabase, SIGNAL(triggered()),
    this, SLOT(updateWaveDatabase()));
  QObject::connect(
    this->actionReports, SIGNAL(triggered()),
    this, SLOT(reports()));
  QObject::connect(
    this->actionRatingCodes, SIGNAL(triggered()),
    this, SLOT(ratingCodes()));
  QObject::connect(
    this->actionLoadUIDs, SIGNAL(triggered()),
    this, SLOT(loadUIDs()));
  QObject::connect(
    this->actionExit, SIGNAL(triggered()),
    qApp, SLOT(closeAllWindows()));
  QObject::connect(
    this->actionSaveImage, SIGNAL(triggered()),
    this, SLOT(saveImage()));

  // connect the help menu items
  QObject::connect(
    this->actionAbout, SIGNAL(triggered()),
    this, SLOT(about()));
  QObject::connect(
    this->actionManual, SIGNAL(triggered()),
    this, SLOT(manual()));

  QObject::connect(
    this->interviewWidget, SIGNAL(imageSelected(int)),
    this, SLOT(updateDicom(int)));

  QObject::connect(
    this->atlasWidget, SIGNAL(showing(bool)),
    this->interviewWidget, SLOT(hideControls(bool)));

  QIcon icon(":/icons/alder32x32.png");
  QApplication::setWindowIcon(icon);

  Alder::Application* app = Alder::Application::GetInstance();

  this->atlasWidget->setParent(q);
  this->atlasVisible = false;
  this->atlasWidget->hide();
  this->dicomTagWidget->setParent(q);
  this->dicomTagsVisible = false;
  this->dicomTagWidget->hide();

  QProgressBar* progress = new QProgressBar();
  this->statusbar->addPermanentWidget(progress);
  progress->setVisible(false);
  QPushButton* button = new QPushButton("Abort");
  this->statusbar->addPermanentWidget(button);
  button->setVisible(false);

  this->qvtkConnection->Connect(app, vtkCommand::StartEvent,
    this, SLOT(showProgress(vtkObject*, unsigned long, void*, void*)));
  this->qvtkConnection->Connect(app, vtkCommand::EndEvent,
    this, SLOT(hideProgress()));

  this->qvtkConnection->Connect(app, vtkCommand::ProgressEvent,
    this, SLOT(updateProgress(vtkObject*, unsigned long, void*, void*)));

  QObject::connect(button, SIGNAL(pressed()),
    this, SLOT(abort()), Qt::DirectConnection);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::showProgress(
  vtkObject*, unsigned long, void*, void* call_data)
{
  QProgressBar* progress = this->statusbar->findChild<QProgressBar*>();
  if (progress)
  {
    progress->setVisible(true);
    progress->setValue(0);
  }
  QPushButton* button = this->statusbar->findChild<QPushButton*>();
  if (button)
    button->setVisible(true);
  QString message = reinterpret_cast<const char*>(call_data);
  if (!message.isEmpty())
    this->statusbar->showMessage(message);

  this->statusbar->repaint();
  QApplication::processEvents();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::hideProgress()
{
  QProgressBar* progress = this->statusbar->findChild<QProgressBar*>();
  if (progress)
  {
    progress->setVisible(false);
    this->statusbar->clearMessage();
  }
  QPushButton* button = this->statusbar->findChild<QPushButton*>();
  if (button)
    button->setVisible(false);
  this->statusbar->clearMessage();
  this->statusbar->repaint();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::updateProgress(
  vtkObject*, unsigned long, void*, void* call_data)
{
  QProgressBar* progress = this->statusbar->findChild<QProgressBar*>();
  double value = *(reinterpret_cast<double*>(call_data));
  progress->setValue(static_cast<int>(100*value));
  QApplication::processEvents();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::abort()
{
  Alder::Application::GetInstance()->SetAbortFlag(1);
  QApplication::processEvents();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::openInterview()
{
  Q_Q(QMainAlderWindow);
  Alder::Application* app = Alder::Application::GetInstance();
  Alder::User* user = app->GetActiveUser();
  if (user)
  {
    // the dialog handles updating the application active interview
    QSelectInterviewDialog dialog(q);
    dialog.setModal(true);
    dialog.setWindowTitle(QDialog::tr("Select Interview"));
    QObject::connect(&dialog, SIGNAL(interviewSelected(int)),
      this->interviewWidget, SLOT(loadInterview(int)));
    dialog.exec();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::login()
{
  Q_Q(QMainAlderWindow);
  Alder::Application* app = Alder::Application::GetInstance();
  Alder::User* user = 0;
  if ((user = app->GetActiveUser()))
  {
    int interviewId = this->interviewWidget->activeInterviewId();
    if (0 != interviewId)
    {
      user->Set("InterviewId", interviewId);
      user->Save();
    }
    app->ResetApplication();
  }
  else
  {
    QLoginDialog dialog(q);
    dialog.setModal(true);
    dialog.setWindowTitle(QDialog::tr("Login"));
    dialog.exec();
    // dialog will access the application and set the active user
    // the application will fire the UserChangedEvent
    // the interview widget will then fire userChanged slot
    // set modality permissions and load the last interview
    // the user was reviewing
  }

  // active user may have changed so update the interface
  this->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::showAtlas()
{
  Q_Q(QMainAlderWindow);
  bool lastVisible = this->atlasVisible;

  if (!lastVisible && this->dicomTagsVisible) this->showDicomTags();

  this->atlasVisible = !this->atlasVisible;
  this->actionShowAtlas->setText(
    (this->atlasVisible ? "Hide Atlas" : "Show Atlas"));

  if (this->atlasVisible)
  {
    // add the widget to the splitter
    this->splitter->insertWidget(0, this->atlasWidget);
    this->atlasWidget->show();

    QList<int> sizeList = this->splitter->sizes();
    int total = sizeList[0] + sizeList[1];
    sizeList[0] = floor(total / 2);
    sizeList[1] = sizeList[0];
    this->splitter->setSizes(sizeList);
  }
  else
  {
    // remove the widget from the splitter
    this->atlasWidget->hide();
    this->atlasWidget->setParent(q);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::showDicomTags()
{
  Q_Q(QMainAlderWindow);
  if (this->atlasVisible) return;

  this->dicomTagsVisible = !this->dicomTagsVisible;

  this->actionShowDicomTags->setText(
    (this->dicomTagsVisible ? "Hide Dicom Tags" : "Show Dicom Tags"));

  if (this->dicomTagsVisible)
  {
    this->splitter->insertWidget(0, this->dicomTagWidget);
    this->dicomTagWidget->show();
  }
  else
  {
    this->dicomTagWidget->hide();
    this->dicomTagWidget->setParent(q);
  }
}


// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::userManagement()
{
  Q_Q(QMainAlderWindow);
  q->adminLoginDo(&QMainAlderWindow::adminUserManagement);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::updateInterviewDatabase()
{
  Q_Q(QMainAlderWindow);
  q->adminLoginDo(&QMainAlderWindow::adminUpdateInterviewDatabase);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::updateWaveDatabase()
{
  Q_Q(QMainAlderWindow);
  q->adminLoginDo(&QMainAlderWindow::adminUpdateWaveDatabase);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::reports()
{
  Q_Q(QMainAlderWindow);
  q->adminLoginDo(&QMainAlderWindow::adminReports);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::ratingCodes()
{
  Q_Q(QMainAlderWindow);
  q->adminLoginDo(&QMainAlderWindow::adminRatingCodes);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::loadUIDs()
{
  Q_Q(QMainAlderWindow);
  std::vector<std::pair<std::string, std::string>> list;
  QFileDialog dialog(q, QDialog::tr("Open File"));
  dialog.setNameFilter(QDialog::tr("CSV files (*.csv)"));
  dialog.setFileMode(QFileDialog::ExistingFile);

  QStringList selectedFiles;
  if (dialog.exec())
    selectedFiles = dialog.selectedFiles();
  if (selectedFiles.isEmpty()) return;
  QString fileName = selectedFiles.front();

  bool error = false;
  QString errorMsg;

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
  {
    error = true;
    errorMsg = "Failed to open file";
  }
  else
  {
    // csv file must contain UId and Wave rank
    QTextStream qstream(&file);
    std::string sep = "\",\"";
    while (!qstream.atEnd())
    {
      QString line = qstream.readLine();
      std::string str = line.toStdString();
      str = Alder::Utilities::trim(str);
      str = Alder::Utilities::removeLeadingTrailing(str, '"');
      std::vector<std::string> parts = Alder::Utilities::explode(str, sep);
      if (2 == parts.size())
      {
        list.push_back(
          std::make_pair(
            Alder::Utilities::trim(parts[0]),
            Alder::Utilities::trim(parts[1])));
      }
    }
    file.close();
    if (list.empty())
    {
      error = true;
      errorMsg  = "Failed to parse identifiers from csv file: ";
      errorMsg += "expecting UId / wave rank pairs";
    }
    else
    {
      int numLoaded = Alder::Interview::LoadFromList(list);
      if (0 == numLoaded)
      {
        error = true;
        errorMsg  = "Failed to find any interviews attributed to data in csv file";
      }
    }
  }

  if (error)
  {
    QMessageBox messageBox(q);
    messageBox.setWindowModality(Qt::WindowModal);
    messageBox.setIcon(QMessageBox::Warning);
    messageBox.setText(errorMsg);
    messageBox.exec();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::changePassword()
{
  Q_Q(QMainAlderWindow);
  Alder::Application* app = Alder::Application::GetInstance();
  Alder::User* user;
  if (NULL != (user = app->GetActiveUser()))
  {
    QString password = user->Get("Password").ToString().c_str();
    QChangePasswordDialog dialog(q, password);
    dialog.setModal(true);
    QObject::connect(
      &dialog, SIGNAL(passwordChanged(QString)),
     this, SLOT(changeActiveUserPassword(QString)));
    dialog.exec();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::changeActiveUserPassword(QString password)
{
  if (password.isEmpty()) return;
  Alder::Application* app = Alder::Application::GetInstance();
  Alder::User* user;
  if (NULL != (user = app->GetActiveUser()))
  {
    user->Set("Password", vtkVariant(password.toStdString()));
    user->Save();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::about()
{
  Q_Q(QMainAlderWindow);
  QAboutDialog dialog(q);
  dialog.setModal(true);
  dialog.setWindowTitle(QDialog::tr("About Alder"));
  dialog.exec();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::manual()
{
  // TODO(dean): open link to Alder manual
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::saveImage()
{
  Q_Q(QMainAlderWindow);
  QString fileName = QFileDialog::getSaveFileName(q,
    QDialog::tr("Save Image to File"), this->lastSavePath,
    QDialog::tr("Images (*.png *.pnm *.bmp *.jpg *.jpeg *.tif *.tiff)"));

  if (fileName.isEmpty()) return;
  else
  {
    this->interviewWidget->saveImage(fileName);
    this->lastSavePath = QFileInfo(fileName).path();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::updateUi()
{
  Alder::Application* app = Alder::Application::GetInstance();
  bool loggedIn = NULL != app->GetActiveUser();

  // dynamic menu action names
  this->actionLogin->setText((loggedIn ? "Logout" : "Login"));

  // set all widget enable states
  this->actionOpenInterview->setEnabled(loggedIn);
  this->actionChangePassword->setEnabled(loggedIn);
  this->actionShowAtlas->setEnabled(loggedIn);
  this->actionShowDicomTags->setEnabled(loggedIn);
  this->actionSaveImage->setEnabled(loggedIn);
  this->actionLoadUIDs->setEnabled(loggedIn);

  this->splitter->setEnabled(loggedIn);

  this->interviewWidget->setEnabled(loggedIn);
  this->atlasWidget->setEnabled(loggedIn);
  this->dicomTagWidget->setEnabled(loggedIn);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QMainAlderWindow methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindow::QMainAlderWindow(QWidget* parent)
  : Superclass(parent)
  , d_ptr(new QMainAlderWindowPrivate(*this))
{
  Q_D(QMainAlderWindow);
  d->setupUi(this);
  this->readSettings();
  d->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindow::~QMainAlderWindow()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::closeEvent(QCloseEvent* event)
{
  this->writeSettings();
  event->accept();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::readSettings()
{
  QSettings settings("CLSA", "Alder");
  settings.beginGroup("MainAlderWindow");
  if (settings.contains("size"))
    this->resize(settings.value("size").toSize());
  if (settings.contains("pos"))
    this->move(settings.value("pos").toPoint());
  if (settings.contains("maximized") && settings.value("maximized").toBool())
    this->showMaximized();
  settings.endGroup();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::writeSettings()
{
  QSettings settings("CLSA", "Alder");
  settings.beginGroup("MainAlderWindow");
  settings.setValue("size", this->size());
  settings.setValue("pos", this->pos());
  settings.setValue("maximized", this->isMaximized());
  settings.endGroup();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::updateDicom(int id)
{
  vtkNew<Alder::Image> image;
  if (image->Load("Id", id))
  {
    this->dicomTagWidget->load(image->GetFileName().c_str());
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::updateAtlas(int id)
{
  this->atlasWidget->loadImage(id);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminLoginDo(void (QMainAlderWindow::*fn)())
{
  int attempt = 1;
  while (attempt < 4)
  {
    // check for admin password
    QString text = QInputDialog::getText(
      this,
      QDialog::tr("User Management"),
      QDialog::tr(
        attempt > 1 ?
        "Wrong password, try again:" :
        "Administrator password:"),
      QLineEdit::Password);

    // do nothing if the user hit the cancel button
    if (text.isEmpty()) break;

    vtkNew<Alder::User> user;
    user->Load("Name", "administrator");
    if (user->IsPassword(text.toStdString().c_str()))
    {
      user->Set("LastLogin", Alder::Utilities::getTime("%Y-%m-%d %H:%M:%S"));
      user->Save();
      (this->*fn)();
      break;
    }
    attempt++;
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminUserManagement()
{
  Q_D(QMainAlderWindow);
  QUserListDialog usersDialog(this);
  usersDialog.setModal(true);
  usersDialog.setWindowTitle(QDialog::tr("User Management"));

  Alder::Application* app = Alder::Application::GetInstance();
  if (app->GetActiveUser())
  {
    QObject::connect(
      &usersDialog, SIGNAL(permissionChanged()),
      d->interviewWidget, SLOT(updatePermission()));
  }
  usersDialog.exec();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminUpdateInterviewDatabase()
{
  QSelectWaveDialog waveDialog(this);
  waveDialog.setModal(true);
  waveDialog.exec();
  std::vector<std::pair<int, bool>> waveVec = waveDialog.selection();
  if (!waveVec.empty())
  {
    Alder::Interview::UpdateInterviewData(waveVec);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminUpdateWaveDatabase()
{
  Q_D(QMainAlderWindow);
  Alder::Application* app = Alder::Application::GetInstance();
  std::string waveSource = app->GetConfig()->GetValue("Opal", "WaveSource");
  QString message = "Updating study wave and scan type information ...";
  d->statusbar->showMessage(message);
  d->statusbar->repaint();
  Alder::Wave::UpdateWaveData(waveSource);
  d->statusbar->clearMessage();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminRatingCodes()
{
  QCodeDialog codeDialog(this);
  codeDialog.setModal(true);
  codeDialog.setWindowTitle(QDialog::tr("Rating Codes"));
  codeDialog.exec();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminReports()
{
  QReportDialog reportDialog(this);
  reportDialog.setModal(true);
  reportDialog.setWindowTitle(QDialog::tr("Rating Reports"));
  reportDialog.exec();
}
