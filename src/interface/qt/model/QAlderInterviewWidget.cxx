/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderInterviewWidget.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QAlderInterviewWidget.h>
#include <QAlderInterviewWidget_p.h>

// Alder includes
#include <Application.h>
#include <Code.h>
#include <CodeType.h>
#include <Common.h>
#include <Exam.h>
#include <Image.h>
#include <ImageNote.h>
#include <Interview.h>
#include <Modality.h>
#include <ParticipantData.h>
#include <QueryModifier.h>
#include <Rating.h>
#include <Site.h>
#include <User.h>
#include <Wave.h>
#include <QAlderImageWidget.h>

// VTK includes
#include <vtkDataArray.h>
#include <vtkEventForwarderCommand.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkNew.h>

// Qt includes
#include <QFileInfo>
#include <QMessageBox>
#include <QTreeWidgetItem>

// C++ includes
#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderInterviewWidgetPrivate methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderInterviewWidgetPrivate::QAlderInterviewWidgetPrivate(
  QAlderInterviewWidget& object)
  : QObject(&object), q_ptr(&object)
{
  this->qvtkConnection = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->participantData = vtkSmartPointer<Alder::ParticipantData>::New();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderInterviewWidgetPrivate::~QAlderInterviewWidgetPrivate()
{
  this->qvtkConnection->Disconnect();
  this->waveLookup.clear();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::setupUi(QWidget* widget)
{
  Q_Q(QAlderInterviewWidget);

  this->Ui_QAlderInterviewWidget::setupUi(widget);

  this->treeWidget->header()->hide();

  this->treeWidget->setIndentation(10);
  this->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  this->treeWidget->setSelectionBehavior(QAbstractItemView::SelectItems);

  // force the branch area to be the color of the tree when selected
  this->treeWidget->setStyleSheet(
    "QTreeWidget::branch:selected {background: white;}");

  QObject::connect(
    this->previousPushButton, SIGNAL(clicked()),
    this, SLOT(previous()));
  QObject::connect(
    this->nextPushButton, SIGNAL(clicked()),
    this, SLOT(next()));
  QObject::connect(
    this->treeWidget, SIGNAL(itemSelectionChanged()),
    this, SLOT(treeSelectionChanged()));
  QObject::connect(
     this->codeTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)),
     this, SLOT(codeChanged(QTableWidgetItem*)));
  QObject::connect(
    this->ratingSlider, SIGNAL(valueChanged(int)),
    this, SLOT(ratingChanged(int)));
  QObject::connect(
    this->noteTextEdit, SIGNAL(textChanged()),
    this, SLOT(noteChanged()));
  QObject::connect(
    this->resetRatingPushButton, SIGNAL(clicked()),
    this, SLOT(resetRating()));
  QObject::connect(
    this->useDerivedCheckBox, SIGNAL(clicked()),
    this, SLOT(derivedRatingToggle()));
  QObject::connect(
    this->rgbFixPushButton, SIGNAL(clicked()),
    this, SLOT(imageYBRToRGB()));
  QObject::connect(
    this->anonymizePushButton, SIGNAL(clicked()),
    this, SLOT(imageAnonymize()));
  QObject::connect(
    this->swapSidesPushButton, SIGNAL(clicked()),
    this, SLOT(imageSideSwap()));

  this->qvtkConnection->Connect(Alder::Application::GetInstance(),
    Alder::Common::UserChangedEvent,
    q, SLOT(userChanged()));

  this->qvtkConnection->Connect(this->participantData,
    Alder::Common::InterviewChangedEvent,
    q, SLOT(interviewChanged()));

  this->qvtkConnection->Connect(this->participantData,
    Alder::Common::DataChangedEvent,
    this, SLOT(buildTree()));

  this->qvtkConnection->Connect(this->participantData,
    Alder::Common::ImageChangedEvent,
    q, SLOT(imageChanged()));

  std::vector<vtkSmartPointer<Alder::Wave>> wList;
  Alder::Wave::GetAll(&wList);

  QTreeWidgetItem *item = 0;
  this->waveLookup.clear();

  QTreeWidgetItem *root = new QTreeWidgetItem(this->treeWidget);
  root->setText(0, "UID");
  root->setExpanded(true);
  root->setDisabled(false);
  this->treeWidget->addTopLevelItem(root);

  for (auto w = wList.begin(); w != wList.end(); ++w)
  {
    vtkSmartPointer<Alder::Wave> wave = *w;
    QString name = wave->Get("Name").ToString().c_str();
    item = new QTreeWidgetItem(root);
    item->setText(0, name);
    item->setExpanded(false);
    item->setDisabled(true);
    this->waveLookup[name] = item;
  }
  item = 0;

  // build the map between modality and user permission
  std::vector<vtkSmartPointer<Alder::Modality>> mlist;
  Alder::Modality::GetAll(&mlist);
  for (auto m = mlist.begin(); m != mlist.end(); ++m)
  {
    QString name = (*m)->Get("Name").ToString().c_str();
    this->modalityPermission[name] = false;
  }

  this->filenameValueLabel->setWordWrap(true);
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::previous()
{
  Alder::Interview* activeInterview =
    this->participantData->GetActiveInterview();
  if (activeInterview)
  {
    vtkSmartPointer<Alder::Interview> interview =
      activeInterview->GetPrevious(
        this->loadedCheckBox->isChecked(),
        this->unratedCheckBox->isChecked());
    this->setActiveInterview(interview.GetPointer());
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::next()
{
  Alder::Interview* activeInterview =
    this->participantData->GetActiveInterview();
  if (activeInterview)
  {
    vtkSmartPointer<Alder::Interview> interview =
      activeInterview->GetNext(
        this->loadedCheckBox->isChecked(),
        this->unratedCheckBox->isChecked());
    this->setActiveInterview(interview.GetPointer());
  }
}

// change the active interview here
// called by next(), previous()
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::setActiveInterview(
  Alder::Interview* interview)
{
  Q_Q(QAlderInterviewWidget);

  // warn user if the neighbouring interview is an empty record (ie: no
  // neighbour found)
  if (!interview)
  {
    QMessageBox errorMessage(q);
    errorMessage.setWindowModality(Qt::WindowModal);
    errorMessage.setIcon(QMessageBox::Warning);
    errorMessage.setText(
      QDialog::tr("There are no remaining interviews available "\
                  "which meet your criteria."));
    errorMessage.exec();
  }
  else
  {
    if (!interview->HasExamData())
    {
      interview->UpdateExamData();
    }
    Alder::Application* app = Alder::Application::GetInstance();
    Alder::User* user = app->GetActiveUser();
    vtkSmartPointer<Alder::QueryModifier> modifier =
      vtkSmartPointer<Alder::QueryModifier>::New();
    user->InitializeExamModifier(modifier);

    Alder::Interview::ImageStatus status = interview->GetImageStatus(modifier);
    if (Alder::Interview::ImageStatus::None == status)
      return;

    if (Alder::Interview::ImageStatus::Pending == status)
    {
      interview->UpdateImageData();
      status = interview->GetImageStatus(modifier);
    }
    if (Alder::Interview::ImageStatus::Complete == status)
    {
      this->participantData->SetActiveInterview(interview);
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::treeSelectionChanged()
{
  // only one item at a time can be selected
  QList<QTreeWidgetItem*> list = this->treeWidget->selectedItems();
  if (!list.isEmpty())
  {
    QHash<QTreeWidgetItem*,
          vtkSmartPointer<Alder::ActiveRecord>>::iterator it =
      this->treeModelMap.find(list.front());
    if (it != this->treeModelMap.end())
    {
      Alder::Image* image = 0;
      if ((image = Alder::Image::SafeDownCast(it.value())))
      {
        this->participantData->SetActiveImage(image);
      }
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updatePermission()
{
  Alder::Application* app = Alder::Application::GetInstance();
  vtkSmartPointer<Alder::User> user = app->GetActiveUser();

  // modalities the user is permitted access to
  std::vector<vtkSmartPointer<Alder::Modality>> ulist;
  if (user)
    user->GetList(&ulist);

  for (QMap<QString, bool>::iterator it = this->modalityPermission.begin();
    it != modalityPermission.end(); ++it)
  {
    QString current = it.key();
    bool permission = it.value();

    // set the default permission for the modality to false
    this->modalityPermission[current] = false;

    // check if the user has permission to this modality
    for (auto u = ulist.begin(); u != ulist.end(); ++u)
    {
      QString name = (*u)->Get("Name").ToString().c_str();
      if (name == current)
      {
        this->modalityPermission[current] = true;
        break;
      }
    }
  }
  this->codeTableWidget->setEnabled(
    user && user->Get("Expert").ToInt());
}

// build the tree with all available data
// setting the disable state of items the user does not have modality
// permission for
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::buildTree()
{
  // stop the tree's signals until we are done
  bool oldSignalState = this->treeWidget->blockSignals(true);

  this->treeModelMap.clear();

  // clear from wave items down
  for (QMap<QString, QTreeWidgetItem*>::iterator it = this->waveLookup.begin();
       it != this->waveLookup.end(); ++it)
  {
     QTreeWidgetItem* item = it.value();
     item->setDisabled(true);
     item->setExpanded(false);
     qDeleteAll(item->takeChildren());
  }

  // set the root text with a UID
  std::string uid = this->participantData->GetUID();
  QTreeWidgetItem* root = this->treeWidget->topLevelItem(0);
  if (uid.empty())
  {
    root->setText(0, "UID");
    this->treeWidget->blockSignals(oldSignalState);
    return;
  }
  root->setText(0, uid.c_str());

  std::vector<vtkSmartPointer<Alder::Modality>> mlist;
  std::vector<vtkSmartPointer<Alder::Wave>> wlist;
  std::vector<vtkSmartPointer<Alder::Interview>> ilist;
  std::vector<vtkSmartPointer<Alder::Exam>> elist;
  std::vector<vtkSmartPointer<Alder::Image>> plist;
  std::vector<vtkSmartPointer<Alder::Image>> clist;

  QTreeWidgetItem* item = 0;
  QTreeWidgetItem* parent = 0;
  QTreeWidgetItem* grandParent = 0;
  QTreeWidgetItem* selected = 0;

  // waves pertinent to this UID
  Alder::Modality::GetAll(&mlist);
  this->participantData->GetWaveList(&wlist);
  for (auto w = wlist.begin(); w != wlist.end(); ++w)
  {
    vtkSmartPointer<Alder::Wave> wave = *w;

    // interviews pertinent to the current wave
    this->participantData->GetInterviewList(*w, &ilist);

    QString name = wave->Get("Name").ToString().c_str();
    QMap<QString, QTreeWidgetItem*>::iterator qit =
      this->waveLookup.find(name);
    if (this->waveLookup.end() == qit)
      continue;
    item = qit.value();

    // by default all interviews and waves are enabled
    item->setFlags(Qt::ItemIsEnabled);
    item->setDisabled(ilist.empty());
    item->setExpanded(!item->isDisabled());
    parent = item;
    item = 0;

    for (auto i = ilist.begin(); i != ilist.end(); ++i)
    {
      vtkSmartPointer<Alder::Interview> interview = *i;
      QString text = interview->Get("VisitDate").ToString().c_str();

      item = new QTreeWidgetItem(parent);
      item->setFlags(Qt::ItemIsEnabled);
      this->treeModelMap[item] = interview;
      item->setText(0, text);
      item->setDisabled(false);
      item->setExpanded(true);
      parent = item;
      item = 0;

      QMap<QString, QTreeWidgetItem*> modalityLookup;

      // add all modalities by name
      for (auto m = mlist.begin(); m != mlist.end(); ++m)
      {
        vtkSmartPointer<Alder::Modality> modality = *m;
        name = modality->Get("Name").ToString().c_str();

        item = new QTreeWidgetItem(parent);
        item->setFlags(Qt::ItemIsEnabled);
        item->setText(0, name);
        // enable modality if the user is permitted access to it
        item->setDisabled(!this->modalityPermission.value(name));
        item->setExpanded(!item->isDisabled());
        modalityLookup[name] = item;
        item = 0;
      }

      this->participantData->GetExamList(*i, &elist);
      for (auto e = elist.begin(); e != elist.end(); ++e)
      {
        vtkSmartPointer<Alder::Exam> exam = *e;
        QString examModality = exam->GetModalityName().c_str();
        QString type = exam->GetScanType().c_str();
        name = "Exam: ";
        QString side = exam->Get("Side").ToString().c_str();
        if ("none" != side)
        {
          name += side;
          name += " ";
        }
        name += type;

        parent = modalityLookup[examModality];
        item = new QTreeWidgetItem(parent);
        item->setFlags(Qt::ItemIsEnabled);
        item->setText(0, name);
        item->setDisabled(parent->isDisabled());
        item->setExpanded(!item->isDisabled());

        // display the status of the exam
        this->participantData->GetParentImageList(*e, &plist);
        item->setIcon(0,
          plist.empty() ?
          QIcon(":/icons/x-icon") :
          QIcon(":/icons/eye-visible-icon"));
        parent = item;
        item = 0;

        // add the images for this exam
        for (auto p = plist.begin(); p != plist.end(); ++p)
        {
          vtkSmartPointer<Alder::Image> image = *p;
          name = "Image #";
          name += image->Get("Acquisition").ToString().c_str();

          item = new QTreeWidgetItem(parent);
          item->setFirstColumnSpanned(false);
          item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
          this->treeModelMap[item] = image;
          item->setText(0, name);

          if (3 == image->Get("Dimensionality").ToInt())
            item->setIcon(0, QIcon(":/icons/movie-icon"));

          if (parent->isDisabled())
            item->setDisabled(true);
            else
              item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
          item->setExpanded(!item->isDisabled());

          vtkSmartPointer<Alder::Image> active =
            this->participantData->GetActiveImage();
          if (active && active->Get("Id") == image->Get("Id"))
            selected = item;

          grandParent = item;
          item = 0;

          // add child images for this image
          this->participantData->GetChildImageList(*p, &clist);
          for (auto c = clist.begin(); c != clist.end(); ++c)
          {
            vtkSmartPointer<Alder::Image> image = *c;
            name = "Image #";
            name += image->Get("Acquisition").ToString().c_str();

            item = new QTreeWidgetItem(grandParent);
            item->setFirstColumnSpanned(false);
            item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            this->treeModelMap[item] = image;
            item->setText(0, name);
            item->setDisabled(parent->isDisabled());
            item->setExpanded(!item->isDisabled());

            vtkSmartPointer<Alder::Image> active =
              this->participantData->GetActiveImage();
            if (active && active->Get("Id") == image->Get("Id"))
              selected = item;

            item = 0;
          }  // end child image items
        }  // end parent image items
      }  // end exam items
    }  // end interview items
  }  // end wave items

  // re-enable the tree's signals
  this->treeWidget->blockSignals(oldSignalState);

  // set and expand the selected item after restoring signals
  // so that other UI elements get updated
  if (selected)
  {
    QTreeWidgetItem* item = selected;  // an image node
    do
    {
      item->setDisabled(false);
      item->setExpanded(true);
      item = item->parent();
    } while (item);
    this->treeWidget->setCurrentItem(selected);
  }
}

// recursion function to set the disable state of a tree item's children
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void setDisableChildren(QTreeWidgetItem* item, const bool& disable)
{
  if (0 == item) return;
  item->setDisabled(disable);
  item->setExpanded(!item->isDisabled());
  for (int i = 0; i < item->childCount(); ++i)
  {
    setDisableChildren(item->child(i), disable);
  }
}

// called when an interview is changed, or the user modality permission changes
// if a new data set is required, participantData will fire DataChangedEvent
// and the buildTree slot is call first
// if the user modality permission changes, recursively set the disable state
// of the tree items
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateTree()
{
  vtkSmartPointer<Alder::Interview> interview =
    this->participantData->GetActiveInterview();
  QTreeWidgetItem* selected = 0;
  if (interview)
  {
    for (QHash<QTreeWidgetItem*,
               vtkSmartPointer<Alder::ActiveRecord>>::iterator it =
      this->treeModelMap.begin(); it != this->treeModelMap.end(); ++it)
    {
      Alder::Interview* record = Alder::Interview::SafeDownCast(it.value());
      if (record && *record == *interview)
      {
        selected = it.key();
        break;
      }
    }
  }

  if (0 == selected)
  {
    this->buildTree();
  }
  else
  {
    QTreeWidgetItem* item = selected;
    // enable the interview up to the wave
    do
    {
      item->setDisabled(false);
      item->setExpanded(true);
    } while (item = item->parent());

    for (QMap<QString, bool>::iterator m = this->modalityPermission.begin();
      m != this->modalityPermission.end(); ++m)
    {
      // find all the items that are a modality
      QList<QTreeWidgetItem*> list = this->treeWidget->findItems(
        m.key(), Qt::MatchExactly|Qt::MatchRecursive);
      bool disable = !m.value();
      for (QList<QTreeWidgetItem*>::iterator i = list.begin();
        i != list.end(); ++i)
      {
        item = *i;
        setDisableChildren(item, disable);
      }
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateSelected()
{
  // return if there is already a selected item
  QList<QTreeWidgetItem*> list = this->treeWidget->selectedItems();
  if (!list.isEmpty())
    return;

  // select the active image in the tree programmatcially
  vtkSmartPointer<Alder::Image> image =
    this->participantData->GetActiveImage();
  QTreeWidgetItem* selected = 0;
  if (image)
  {
    for (QHash<QTreeWidgetItem*,
               vtkSmartPointer<Alder::ActiveRecord>>::iterator it =
      this->treeModelMap.begin(); it != this->treeModelMap.end(); ++it)
    {
      Alder::Image* record = Alder::Image::SafeDownCast(it.value());
      if (record && *record == *image)
      {
        selected = it.key();
        break;
      }
    }
  }
  if (selected && !selected->isSelected())
  {
    this->treeWidget->setCurrentItem(selected);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateInfo()
{
  QString noteString = "";
  QString interviewerString = QLabel::tr("N/A");
  QString siteString = QLabel::tr("N/A");
  QString dateString = QLabel::tr("N/A");
  QString codeString = QLabel::tr("N/A");

  QString originString = QLabel::tr("N/A");
  QString dimensionsString = QLabel::tr("N/A");
  QString spacingString = QLabel::tr("N/A");
  QString bytesString = QLabel::tr("N/A");
  QString filenameString = QLabel::tr("N/A");
  QString typeString = QLabel::tr("N/A");
  QString channelsString = QLabel::tr("N/A");
  QString rangeString = QLabel::tr("N/A");

  // fill in the active exam information
  Alder::Interview* interview = this->participantData->GetActiveInterview();
  Alder::Image* image = this->participantData->GetActiveImage();
  if (interview && image)
  {
    // get exam from active image
    vtkSmartPointer<Alder::Exam> exam;
    if (image->GetRecord(exam))
    {
      Alder::Application* app = Alder::Application::GetInstance();
      Alder::User* user = app->GetActiveUser();
      if (user)
      {
        vtkNew<Alder::ImageNote> note;
        std::map<std::string, std::string> map;
        map["UserId"] = user->Get("Id").ToString();
        map["ImageId"] = image->Get("Id").ToString();
        if (note->Load(map))
          noteString = note->Get("Note").ToString();
      }
      interviewerString =
        QLabel::tr(exam->Get("Interviewer").ToString().c_str());
      vtkSmartPointer<Alder::Site> site = vtkSmartPointer<Alder::Site>::New();
      interview->GetRecord(site);
      vtkSmartPointer<Alder::Wave> wave = vtkSmartPointer<Alder::Wave>::New();
      interview->GetRecord(wave);
      siteString = QLabel::tr(site->Get("Name").ToString().c_str());
      siteString += "/";
      siteString += QLabel::tr(wave->Get("Name").ToString().c_str());
      dateString = QLabel::tr(exam->Get("DatetimeAcquired").ToString().c_str());
      codeString = QLabel::tr(image->GetCode().c_str());

      vtkImageData* vtkimage = this->imageWidget->imageData();
      if (vtkimage)
      {
        int n = vtkimage->GetNumberOfScalarComponents();
        channelsString = QString::number(n);

        originString.clear();
        spacingString.clear();
        dimensionsString.clear();
        QString sep = ", ";
        double* o = vtkimage->GetOrigin();
        double* s = vtkimage->GetSpacing();
        int* d = vtkimage->GetDimensions();
        for (int i = 0; i < 3; ++i)
        {
          originString += QString::number(o[i]);
          spacingString += QString::number(s[i]);
          dimensionsString += QString::number(d[i]);
          if (i < 2)
          {
            originString += sep;
            spacingString += sep;
            dimensionsString += sep;
          }
        }

        typeString = vtkimage->GetScalarTypeAsString();

        double range[2];
        double min = vtkimage->GetScalarTypeMax();
        double max = vtkimage->GetScalarTypeMin();
        vtkDataArray* data = vtkimage->GetPointData()->GetScalars();
        for (int i = 0; i < n; ++i)
        {
          data->GetRange(range, i);
          min = min < range[0] ? min : range[0];
          max = max > range[1] ? max : range[1];
        }
        rangeString = "[";
        rangeString += QString::number(min);
        rangeString += sep;
        rangeString += QString::number(max);
        rangeString += "]";

        filenameString = image->GetFileName().c_str();
        QFileInfo info(filenameString);

        bytesString = QString::number(info.size());

        int count = 0;
        for (int i = filenameString.size()-1; i >= 0;  --i)
        {
          if (filenameString[i] == QChar('/'))
          {
            count++;
            if (3 == count)
            {
              filenameString.insert(i+1, QString("\n"));
              break;
            }
          }
        }
      }
    }
  }

  // set the text edit content from the exam note
  bool oldSignalState = this->noteTextEdit->blockSignals(true);
  this->noteTextEdit->setPlainText(noteString);
  this->noteTextEdit->blockSignals(oldSignalState);

  this->infoInterviewerValueLabel->setText(interviewerString);
  this->infoSiteValueLabel->setText(siteString);
  this->infoDateValueLabel->setText(dateString);
  this->infoCodeValueLabel->setText(codeString);

  this->dimensionsValueLabel->setText(dimensionsString);
  this->spacingValueLabel->setText(spacingString);
  this->bytesValueLabel->setText(bytesString);
  this->channelsValueLabel->setText(channelsString);
  this->originValueLabel->setText(originString);
  this->rangeValueLabel->setText(rangeString);
  this->filenameValueLabel->setText(filenameString);
  this->typeValueLabel->setText(typeString);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateCodeList()
{
  Alder::Image* image = this->participantData->GetActiveImage();
  if (!image)
  {
    return;
  }
  vtkSmartPointer<Alder::Exam> exam;
  image->GetRecord(exam);
  std::map<int, std::string> codeMap = exam->GetCodeTypeData();

  int colCount = 4;
  int imax = codeMap.size();
  if (colCount > imax) colCount = imax;
  int rowCount = 0;
  for (int i = 0; i < imax; i += colCount) rowCount++;

  this->codeTableWidget->clear();
  this->codeTableWidget->setColumnCount(colCount);
  this->codeTableWidget->setRowCount(rowCount);

  auto it = codeMap.begin();
  for (int row = 0; row < rowCount;  ++row)
  {
    for (int column = 0; column < colCount; ++column)
    {
      if (it != codeMap.end())
      {
        QTableWidgetItem* item = new QTableWidgetItem();
        this->codeTableWidget->setItem(row, column, item);
        QCheckBox* box = new QCheckBox(tr(it->second.c_str()));
        this->codeTableWidget->setCellWidget(row, column, box);
        box->setProperty("column", QVariant(column));
        box->setProperty("row", QVariant(row));
        box->setProperty("codeTypeId", QVariant(it->first));
        QObject::connect(box, SIGNAL(toggled(bool)),
          this, SLOT(codeSelected()));
        Qt::ItemFlags flags = item->flags();
        flags &= ~(Qt::ItemIsDropEnabled|Qt::ItemIsDragEnabled|
                   Qt::ItemIsSelectable|Qt::ItemIsEditable);
        item->setFlags(flags);
        ++it;
      }
      else
      {
        QTableWidgetItem* item = this->codeTableWidget->itemAt(row, column);
        Qt::ItemFlags flags = Qt::NoItemFlags;
        item->setFlags(flags);
      }
    }
  }
  this->codeTableWidget->resizeColumnsToContents();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateViewer()
{
  Alder::Image* image = this->participantData->GetActiveImage();
  if (image)
  {
    if (3 == image->Get("Dimensionality").ToInt())
    {
      Alder::Application* app = Alder::Application::GetInstance();
      vtkNew<vtkEventForwarderCommand> forward;
      forward->SetTarget(app);
      app->SetAbortFlag(0);
      std::string message = "Reading image data...";
      app->InvokeEvent(
        vtkCommand::StartEvent,
        reinterpret_cast<void*>(const_cast<char*>(message.c_str())));
      this->imageWidget->load(
        image->GetFileName().c_str(), forward.GetPointer());
      app->InvokeEvent(vtkCommand::EndEvent);
    }
    else
    {
      this->imageWidget->load(image->GetFileName().c_str());
    }
  }
  else
    this->imageWidget->reset();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateEnabled()
{
  Alder::Interview* interview = this->participantData->GetActiveInterview();
  Alder::Image* image = this->participantData->GetActiveImage();
  Alder::User* user = Alder::Application::GetInstance()->GetActiveUser();

  // set all widget enable states
  this->unratedCheckBox->setEnabled(interview);
  this->loadedCheckBox->setEnabled(interview);
  this->previousPushButton->setEnabled(interview);
  this->nextPushButton->setEnabled(interview);
  this->treeWidget->setEnabled(interview);

  bool expert = user && user->Get("Expert").ToInt();
  this->useDerivedCheckBox->setEnabled(image && expert);
  this->resetRatingPushButton->setEnabled(image && expert);
  this->codeTableWidget->setEnabled(image && expert);
  this->ratingSlider->setEnabled(image && expert);
  if (this->ratingSlider->isEnabled())
     this->ratingSlider->setEnabled(!this->useDerivedCheckBox->isChecked());
  this->noteTextEdit->setEnabled(image && expert);

  // the action buttons depend on the type and content of the image
  vtkSmartPointer<Alder::Exam> exam;
  if (image && image->GetRecord(exam))
  {
    Alder::Exam::SideStatus status = exam->GetSideStatus();
    this->swapSidesPushButton->setEnabled(
      Alder::Exam::SideStatus::Fixed < status);
    bool isdicom = exam->IsDICOM();
    if (isdicom)
    {
      std::string ybr = "YBR_FULL_422";
      this->rgbFixPushButton->setEnabled(
        (ybr == image->GetDICOMTag("PhotometricInterpretation")));
    }
    this->anonymizePushButton->setEnabled(isdicom);
  }
  else
  {
    this->swapSidesPushButton->setEnabled(false);
    this->rgbFixPushButton->setEnabled(false);
    this->anonymizePushButton->setEnabled(false);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::updateRating()
{
  // stop the rating slider's signals until we are done
  bool oldSignalState = this->ratingSlider->blockSignals(true);

  int ratingValue = 0;
  int derivedRating = 0;
  Alder::Application* app = Alder::Application::GetInstance();

  // make sure we have an active image
  Alder::User* user = app->GetActiveUser();
  Alder::Image* image = this->participantData->GetActiveImage();
  vtkVariant userId;
  vtkVariant imageId;

  if (user && image)
  {
    std::map<std::string, std::string> map;
    userId = user->Get("Id");
    imageId = image->Get("Id");
    map["UserId"] = userId.ToString();
    map["ImageId"] = imageId.ToString();
    vtkNew<Alder::Rating> rating;
    if (rating->Load(map))
    {
      vtkVariant v = rating->Get("Rating");
      if (v.IsValid()) ratingValue = v.ToInt();
      v = rating->Get("DerivedRating");
      if (v.IsValid()) derivedRating = v.ToInt();
    }
    std::vector<vtkSmartPointer<Alder::Code>> codeList;
    vtkSmartPointer<Alder::QueryModifier> modifier =
      vtkSmartPointer<Alder::QueryModifier>::New();
    modifier->Where("UserId", "=" , userId);
    modifier->Where("ImageId", "=" , imageId);
    Alder::Code::GetAll(&codeList, modifier);
    std::vector<std::string> codeString;
    QStringList qlist;
    for (auto it = codeList.begin(); it != codeList.end(); ++it)
    {
      vtkSmartPointer<Alder::CodeType> codeType;
      if ((*it)->GetRecord(codeType))
        qlist << codeType->Get("Code").ToString().c_str();
    }
    qlist.removeDuplicates();

    for (int row = 0; row < this->codeTableWidget->rowCount(); ++row)
      for (int col = 0; col < this->codeTableWidget->columnCount(); ++col)
      {
        QCheckBox* box =
          qobject_cast<QCheckBox*>(
            this->codeTableWidget->cellWidget(row, col));
        if (box && -1 != qlist.indexOf(box->text()))
        {
          box->blockSignals(true);
          box->setChecked(true);
          box->blockSignals(false);
        }
      }
  }

  this->ratingSlider->setValue(ratingValue);
  this->ratingValueLabel->setText(0 == ratingValue ?
    QLabel::tr("N/A") :
    QString::number(ratingValue));
  this->derivedRatingLabel->setText("Derived Rating " +
    (0 == derivedRating ? QLabel::tr("N/A") : QString::number(derivedRating)));

  this->useDerivedCheckBox->setChecked(
    (derivedRating == ratingValue ? true : false));

  // re-enable the rating slider's signals
  this->ratingSlider->blockSignals(oldSignalState);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::derivedRatingToggle()
{
  bool useDerived = this->useDerivedCheckBox->isChecked();
  if (useDerived)
  {
    Alder::Application* app = Alder::Application::GetInstance();

    // make sure we have an active image
    Alder::User* user = app->GetActiveUser();
    Alder::Image* image = this->participantData->GetActiveImage();
    if (user && image)
    {
      std::map<std::string, std::string> map;
      map["UserId"] = user->Get("Id").ToString();
      map["ImageId"] = image->Get("Id").ToString();
      vtkNew<Alder::Rating> rating;
      int ratingValue;
      int derivedRating;
      if (rating->Load(map))
      {
        vtkVariant v = rating->Get("Rating");
        if (v.IsValid()) ratingValue = v.ToInt();
        v = rating->Get("DerivedRating");
        if (v.IsValid()) derivedRating = v.ToInt();
        if (derivedRating != ratingValue)
        {
          this->ratingSlider->setEnabled(true);
          this->ratingSlider->setValue(v.ToInt());
        }
      }
    }
  }
  this->ratingSlider->setEnabled(!useDerived);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::ratingChanged(int value)
{
  Alder::Application* app = Alder::Application::GetInstance();

  // make sure we have an active user and image
  Alder::User* user = app->GetActiveUser();
  Alder::Image* image = this->participantData->GetActiveImage();
  if (user && image)
  {
    // See if we have a rating for this user and image
    std::map<std::string, std::string> map;
    map["UserId"] = user->Get("Id").ToString();
    map["ImageId"] = image->Get("Id").ToString();
    vtkNew<Alder::Rating> rating;
    if (!rating->Load(map))
    { // no record exists, set the user and image ids
      rating->Set(map);
    }

    if (0 == value)
      rating->SetNull("Rating");
    else
      rating->Set("Rating", value);

    rating->Save();
  }
  this->ratingValueLabel->setText(0 == value ?
    QLabel::tr("N/A") : QString::number(value));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::resetRating()
{
  Alder::Application* app = Alder::Application::GetInstance();

  // make sure we have an active image
  Alder::User* user = app->GetActiveUser();
  Alder::Image* image = this->participantData->GetActiveImage();

  if (user && image)
  {
    std::map<std::string, std::string> map;
    vtkVariant userId = user->Get("Id");
    vtkVariant imageId = image->Get("Id");
    map["UserId"] = userId.ToString();
    map["ImageId"] = imageId.ToString();
    vtkNew<Alder::Rating> rating;
    if (rating->Load(map))
      rating->Remove();

    std::vector<vtkSmartPointer<Alder::Code>> codeList;
    vtkSmartPointer<Alder::QueryModifier> modifier =
      vtkSmartPointer<Alder::QueryModifier>::New();
    modifier->Where("UserId", "=" , userId);
    modifier->Where("ImageId", "=" , imageId);
    Alder::Code::GetAll(&codeList, modifier);
    for (auto it = codeList.begin(); it != codeList.end(); ++it)
      (*it)->Remove();

    for (int row = 0; row < this->codeTableWidget->rowCount(); ++row)
      for (int col = 0; col < this->codeTableWidget->columnCount(); ++col)
      {
        QCheckBox* box =
          qobject_cast<QCheckBox*>(
            this->codeTableWidget->cellWidget(row, col));
        if (box)
        {
          box->blockSignals(true);
          box->setChecked(false);
          box->blockSignals(false);
        }
      }

    this->derivedRatingLabel->setText(tr("Derived Rating N/A"));

    bool oldSignalState = this->ratingSlider->blockSignals(true);
    this->ratingSlider->setValue(0);
    this->ratingValueLabel->setText(QLabel::tr("N/A"));
    this->ratingSlider->blockSignals(oldSignalState);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::codeChanged(QTableWidgetItem* item)
{
  QCheckBox* box =
    qobject_cast<QCheckBox*>(
      this->codeTableWidget->cellWidget(item->row(), item->column()));
  if (box)
  {
    box->setChecked(!box->isChecked());
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::codeSelected()
{
  // who sent this?
  QCheckBox* box = qobject_cast<QCheckBox*>(sender());
  if (box)
  {
    int column = box->property("column").toInt();
    int row = box->property("row").toInt();
    std::string codeTypeId =
      box->property("codeTypeId").toString().toStdString();
    bool codeSelected = box->isChecked();
    std::string codeStr = box->text().toStdString();
    // update the rating, the string of codes, and the slider

    Alder::Application* app = Alder::Application::GetInstance();

    // make sure we have an active image
    Alder::User* user = app->GetActiveUser();
    Alder::Image* image = this->participantData->GetActiveImage();

    if (user && image)
    {
      // get the codetype
      vtkNew<Alder::CodeType> codeType;
      if (codeType->Load("Id", codeTypeId))
      {
        std::map<std::string, std::string> map;
        map["UserId"] = user->Get("Id").ToString();
        map["ImageId"] = image->Get("Id").ToString();
        vtkNew<Alder::Rating> rating;
        if (!rating->Load(map))
        {
          // no record exists, set the user and image ids
          rating->Set(map);
          rating->Save();
        }

        vtkNew<Alder::Code> code;
        map["CodeTypeId"] = codeTypeId;
        if (codeSelected)
        {
          if (!code->Load(map))
          {
            code->Set(map);
            code->Save();
          }
        }
        else
        {
          if (code->Load(map))
            code->Remove();
        }

        bool useDerived = this->useDerivedCheckBox->isChecked();
        rating->UpdateDerivedRating(useDerived);
        vtkVariant derivedRating = rating->Get("DerivedRating");
        this->derivedRatingLabel->setText(
          QLabel::tr("Derived Rating ") + derivedRating.ToString().c_str());
        if (useDerived)
        {
          bool oldSignalState = this->ratingSlider->blockSignals(true);
          this->ratingSlider->setValue(derivedRating.ToInt());
          this->ratingValueLabel->setText(
            QString::number(derivedRating.ToInt()));
          this->ratingSlider->blockSignals(oldSignalState);
        }
      }
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::noteChanged()
{
  Alder::Application* app = Alder::Application::GetInstance();

  // make sure we have an active user and image
  Alder::User* user = app->GetActiveUser();
  Alder::Image* image = this->participantData->GetActiveImage();
  std::string noteStr = this->noteTextEdit->toPlainText().toStdString();
  if (user && image)
  {
    vtkNew<Alder::ImageNote> note;
    std::map<std::string, std::string> map;
    map["UserId"] = user->Get("Id").ToString();
    map["ImageId"] = image->Get("Id").ToString();
    if (note->Load(map))
    {
      if (noteStr.empty())
      {
        note->Remove();
      }
      else
      {
        note->Set("Note", noteStr);
        note->Save();
      }
    }
    else
    {
      if (!noteStr.empty())
      {
        map["Note"] = noteStr;
        note->Set(map);
        note->Save();
      }
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::imageAnonymize()
{
  Alder::Image* image = this->participantData->GetActiveImage();
  if (image && image->IsDICOM() && image->AnonymizeDICOM())
  {
    if (image->CleanHologicDICOM())
      this->updateViewer();
    this->updateInfo();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::imageYBRToRGB()
{
  Alder::Image* image = this->participantData->GetActiveImage();
  if (image && image->IsDICOM() && image->YBRToRGB())
  {
    this->updateViewer();
    this->updateInfo();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidgetPrivate::imageSideSwap()
{
  Q_Q(QAlderInterviewWidget);
  Alder::Image* image = this->participantData->GetActiveImage();
  if (image)
  {
    // if this image is a child image, warn the user
    vtkVariant v = image->Get("ParentImageId");
    if (v.IsValid())
    {
      QString message = "This image is a child image. ";
      message += "Consider selecting the parent ";
      message += "of this image for swapping instead. ";
      message += "Continue with this image?";
      if (QMessageBox::No == QMessageBox::question(
        q, QDialog::tr("Swap Images"), message,
        QMessageBox::Yes|QMessageBox::No))
        return;
    }
    if (image->SwapExamSide())
    {
      this->updateTree();
      this->updateViewer();
      this->updateInfo();
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderInterviewWidget methods
//
// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderInterviewWidget::QAlderInterviewWidget(QWidget* parent)
  : Superclass(parent)
  , d_ptr(new QAlderInterviewWidgetPrivate(*this))
{
  Q_D(QAlderInterviewWidget);
  d->setupUi(this);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderInterviewWidget::~QAlderInterviewWidget()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::saveImage(const QString& fileName)
{
  Q_D(QAlderInterviewWidget);
  d->imageWidget->save(fileName);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::interviewChanged()
{
  Q_D(QAlderInterviewWidget);
  d->updateTree();
  d->updateEnabled();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::hideControls(bool hide)
{
  Q_D(QAlderInterviewWidget);
  // hide/show all the widgets to show only the image viewer
  QList<QLayout*> layouts;
  layouts.append(d->verticalLayout);
  layouts.append(d->infoLayout);
  layouts.append(d->ratingLayout);
  layouts.append(d->buttonLayout);

  QList<QWidget*> widgets;
  for (int i = 0; i < layouts.count(); ++i)
  {
     QLayout* layout = layouts.at(i);
    for (int j = 0; j < layout->count(); ++j)
    {
      if (QWidgetItem* item = dynamic_cast<QWidgetItem*>(
          layout->itemAt(j)))
      {
        widgets.append(item->widget());
      }
    }
  }
  for (int i = 0; i < widgets.count(); ++i)
  {
    QWidget* item = qobject_cast<QWidget*>(widgets.at(i));
    if (hide) item->hide();
    else item->show();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::loadInterview(int id)
{
  Q_D(QAlderInterviewWidget);
  if (d->participantData->LoadInterview(id))
  {
    Alder::Application* app = Alder::Application::GetInstance();
    vtkSmartPointer<Alder::User> user = app->GetActiveUser();
    user->Set("InterviewId", id);
    user->Save();
    d->updateEnabled();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::userChanged()
{
  Q_D(QAlderInterviewWidget);
  Alder::Application* app = Alder::Application::GetInstance();
  vtkSmartPointer<Alder::User> user = app->GetActiveUser();

  d->updatePermission();

  // load the user's last interview
  vtkSmartPointer<Alder::Interview> interview;
  if (user && user->GetRecord(interview))
  {
    d->participantData->SetActiveInterview(interview);
  }
  else
  {
    d->participantData->SetActiveInterview(NULL);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::updatePermission()
{
  Q_D(QAlderInterviewWidget);
  d->updatePermission();
  d->updateTree();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QAlderInterviewWidget::activeInterviewId()
{
  Q_D(QAlderInterviewWidget);
  Alder::Interview* interview = d->participantData->GetActiveInterview();
  return (interview ? interview->Get("Id").ToInt() : 0);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::imageChanged()
{
  Q_D(QAlderInterviewWidget);
  d->updateCodeList();
  d->updateRating();
  d->updateViewer();
  d->updateSelected();
  d->updateInfo();
  d->updateEnabled();
  Alder::Image* image = d->participantData->GetActiveImage();
  emit imageSelected((image ? image->Get("Id").ToInt() : 0));
}
