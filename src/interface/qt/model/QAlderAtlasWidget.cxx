/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderAtlasWidget.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QAlderAtlasWidget.h>
#include <QAlderAtlasWidget_p.h>

// Alder includes
#include <Application.h>
#include <Common.h>
#include <Exam.h>
#include <Image.h>
#include <ImageNote.h>
#include <Interview.h>
#include <Modality.h>
#include <Rating.h>
#include <ScanType.h>
#include <Site.h>
#include <User.h>
#include <QAlderImageWidget.h>

// VTK includes
#include <vtkEventQtSlotConnect.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

// C++ includes
#include <stdexcept>
#include <vector>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderAtlasWidgetPrivate methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidgetPrivate::QAlderAtlasWidgetPrivate(QAlderAtlasWidget& object)
  : QObject(&object), q_ptr(&object)
{
  this->qvtkConnection = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->rootImage = 0;
  this->atlasImage = 0;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidgetPrivate::~QAlderAtlasWidgetPrivate()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidgetPrivate::setupUi(QWidget* widget)
{
  Q_Q(QAlderAtlasWidget);

  this->Ui_QAlderAtlasWidget::setupUi(widget);

  QObject::connect(
    this->previousPushButton, SIGNAL(clicked()),
    this, SLOT(previous()));
  QObject::connect(
    this->nextPushButton, SIGNAL(clicked()),
    this, SLOT(next()));
  QObject::connect(
    this->ratingComboBox, SIGNAL(currentIndexChanged(int)),
    this, SLOT(ratingChanged()));
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidgetPrivate::ratingChanged()
{
  if (this->rootImage)
  {
    // See if we have an atlas entry for this kind of image at the requested
    // rating
    vtkSmartPointer<Alder::Image> newAtlasImage =
      this->rootImage->GetAtlasImage(this->rating());
    if (newAtlasImage->Get("Id").IsValid())
      this->atlasImage = newAtlasImage;
    else
      this->atlasImage = 0;
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidgetPrivate::previous()
{
  if (this->atlasImage)
  {
    vtkSmartPointer<Alder::Image> previousImage =
      this->atlasImage->GetPreviousAtlasImage(
        this->rating(), this->rootImage->Get("Id").ToInt());
    if (previousImage->Get("Id").IsValid())
      this->atlasImage = previousImage;
    else
      this->atlasImage = 0;
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidgetPrivate::next()
{
  if (this->atlasImage)
  {
    vtkSmartPointer<Alder::Image> nextImage =
      this->atlasImage->GetNextAtlasImage(
        this->rating(), this->rootImage->Get("Id").ToInt());
    if (nextImage->Get("Id").IsValid())
      this->atlasImage = nextImage;
    else
      this->atlasImage = 0;
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidgetPrivate::updateUi()
{
  Q_Q(QAlderAtlasWidget);
  QString helpString = "";
  QStringList noteString;
  QString interviewerString = QLabel::tr("N/A");
  QString siteString = QLabel::tr("N/A");
  QString dateString = QLabel::tr("N/A");
  QString uidString = QLabel::tr("N/A");
  QString codeString = QLabel::tr("N/A");

  bool enable = false;
  if (this->atlasImage)
  {
    vtkSmartPointer<Alder::Exam> exam;
    vtkSmartPointer<Alder::Interview> interview;
    vtkSmartPointer<Alder::Site> site;
    if (this->atlasImage->GetRecord(exam) &&
        exam->GetRecord(interview) &&
        interview->GetRecord(site))
    {
      std::vector<vtkSmartPointer<Alder::ImageNote>> noteList;
      this->atlasImage->GetList(&noteList);
      if (!noteList.empty())
      {
        for (auto it = noteList.begin(); it != noteList.end(); ++it)
        {
          noteString << (*it)->Get("Note").ToString().c_str();
        }
      }
      interviewerString = exam->Get("Interviewer").ToString().c_str();
      siteString = site->Get("Name").ToString().c_str();
      dateString = exam->Get("DatetimeAcquired").ToString().c_str();
      uidString = interview->Get("UId").ToString().c_str();
      codeString = this->atlasImage->GetCode().c_str();

      vtkSmartPointer<Alder::ScanType> scanType;
      exam->GetRecord(scanType);
      vtkSmartPointer<Alder::Modality> modality;
      scanType->GetRecord(modality);
      helpString = modality->Get("Help").ToString();
    }
    this->imageWidget->load(this->atlasImage->GetFileName().c_str());
    enable = true;
  }
  else
    this->imageWidget->reset();

  this->helpTextEdit->setPlainText(helpString);
  for (int i = 0; i < noteString.size(); ++i)
  {
    this->noteTextEdit->insertPlainText(noteString.at(i));
    this->noteTextEdit->moveCursor(QTextCursor::End);
  }
  this->infoInterviewerValueLabel->setText(interviewerString);
  this->infoSiteValueLabel->setText(siteString);
  this->infoDateValueLabel->setText(dateString);
  this->infoCodeValueLabel->setText(codeString);
  this->infoUIdValueLabel->setText(uidString);

  // set all widget enable states
  this->previousPushButton->setEnabled(enable);
  this->nextPushButton->setEnabled(enable);
  this->noteTextEdit->setEnabled(enable);
  this->helpTextEdit->setEnabled(enable);
  this->ratingComboBox->setEnabled(q->isVisible());
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QAlderAtlasWidgetPrivate::rating()
{
  return this->ratingComboBox->currentIndex() + 1;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidgetPrivate::updateRoot(const int& id)
{
  vtkSmartPointer<Alder::Image> newRootImage;
  if (newRootImage->Load("Id", id))
  {
    this->rootImage = newRootImage;
    bool getNewAtlasImage = false;

    vtkSmartPointer<Alder::Exam> exam;
    this->rootImage->GetRecord(exam);
    if (this->atlasImage)
    {
      vtkSmartPointer<Alder::Exam> atlasExam;
      this->atlasImage->GetRecord(atlasExam);
      if (exam->GetScanType() != atlasExam->GetScanType() ||
          exam->Get("Side").ToString() != atlasExam->Get("Side").ToString())
        getNewAtlasImage = true;
    }
    else
      getNewAtlasImage = true;

    if (getNewAtlasImage)
    {
      this->atlasImage = this->rootImage->GetAtlasImage(this->rating());
    }
  }
  else
  {
    this->rootImage = 0;
    this->atlasImage = 0;
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderAtlasWidget methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidget::QAlderAtlasWidget(QWidget* parent)
  : Superclass(parent)
  , d_ptr(new QAlderAtlasWidgetPrivate(*this))
{
  Q_D(QAlderAtlasWidget);
  d->setupUi(this);
  d->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidget::~QAlderAtlasWidget()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::showEvent(QShowEvent* event)
{
  Q_D(QAlderAtlasWidget);
  QWidget::showEvent(event);
  d->updateUi();
  emit showing(true);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::hideEvent(QHideEvent* event)
{
  QWidget::hideEvent(event);
  emit showing(false);
}

// called when a new image is displayed in the QAlderInterviewWidget
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::loadImage(int id)
{
  if (!this->isVisible()) return;
  Q_D(QAlderAtlasWidget);
  d->updateRoot(id);
  d->updateUi();
}
