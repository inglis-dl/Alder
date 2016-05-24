/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderDicomTagWidget.cxx
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QAlderDicomTagWidget.h>
#include <ui_QAlderDicomTagWidget.h>

// Qt includes
#include <QTableWidgetItem>

// vtk-dicom includes
#include <vtkDICOMConfig.h>
#include <vtkDICOMParser.h>
#include <vtkDICOMDictionary.h>
#include <vtkDICOMMetaData.h>
#include <vtkDICOMItem.h>
#include <vtkDICOMReader.h>
#include <vtkDICOMUtilities.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkVariant.h>

// C includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// C++ includes
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class QAlderDicomTagWidgetPrivate : public Ui_QAlderDicomTagWidget
{
  Q_DECLARE_PUBLIC(QAlderDicomTagWidget);

  protected:
    QAlderDicomTagWidget* const q_ptr;

  public:
    explicit QAlderDicomTagWidgetPrivate(QAlderDicomTagWidget& object);
    virtual ~QAlderDicomTagWidgetPrivate(){}

    virtual void setupUi(QWidget* widget);
    virtual void updateUi();
    virtual void setFileName(const QString& fileName);
    virtual void buildDicomStrings();

  private:
    std::vector<std::vector<std::string>> dicomStrings;
    QString fileName;
    vtkSmartPointer<vtkDICOMParser> parser;
    vtkSmartPointer<vtkDICOMMetaData> data;
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderDicomTagWidgetPrivate::QAlderDicomTagWidgetPrivate(
  QAlderDicomTagWidget& object) : q_ptr(&object)
{
  this->parser = vtkSmartPointer<vtkDICOMParser>::New();
  this->data = vtkSmartPointer<vtkDICOMMetaData>::New();
  this->parser->SetMetaData(data);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderDicomTagWidgetPrivate::setupUi(QWidget* widget)
{
  Q_Q(QAlderDicomTagWidget);
  this->Ui_QAlderDicomTagWidget::setupUi(widget);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderDicomTagWidgetPrivate::setFileName(const QString& aFileName)
{
  this->fileName = aFileName;
  this->updateUi();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
// The following recursion function was adapted from vtk-dicom
// Programs/dicomcump.cxx
//
#define MAX_INDENT 24
#define INDENT_SIZE 2
#define MAX_LENGTH 120

void printElement(vtkDICOMMetaData* meta, const vtkDICOMItem* item,
  const vtkDICOMDataElementIterator& iter, int depth,
  unsigned int pixelDataVL, std::vector<std::vector<std::string>>* collection)
{
  vtkDICOMTag tag = iter->GetTag();
  int g = tag.GetGroup();
  int e = tag.GetElement();
  vtkDICOMVR vr = iter->GetVR();
  const char* name = "";
  vtkDICOMDictEntry d;
  if (item)
  {
    d = item->FindDictEntry(tag);
  }
  else if (meta)
  {
    d = meta->FindDictEntry(tag);
  }
  if (d.IsValid())
  {
    name = d.GetName();
  }
  else if ((tag.GetGroup() & 0xFFFE) != 0 && tag.GetElement() == 0)
  {
    // group is even, element is zero
    name = "GroupLength";
  }
  else if ((tag.GetGroup() & 0x0001) != 0 &&
           (tag.GetElement() & 0xFF00) == 0)
  {
    // group is odd, element is a creator element
    name = "PrivateCreator";
  }
  // allow multiple values (i.e. for each image in series)
  vtkDICOMValue v = iter->GetValue();
  unsigned int vn = v.GetNumberOfValues();
  const vtkDICOMValue* vp = v.GetMultiplexData();
  if (vp == 0)
  {
    vp = &v;
    vn = 1;
  }
  if (MAX_INDENT < INDENT_SIZE*depth)
  {
    depth = MAX_INDENT/INDENT_SIZE;
  }

  for (unsigned int vi = 0; vi < vn; vi++)
  {
    v = vp[vi];
    unsigned int vl = v.GetVL();
    if (tag == DC::PixelData ||
       tag == DC::FloatPixelData ||
       tag == DC::DoubleFloatPixelData)
    {
      vl = (0 == depth && 0 == vl ? pixelDataVL : v.GetVL());
    }
    std::string s;
    if (vr == vtkDICOMVR::UN ||
       vr == vtkDICOMVR::SQ)
    {
      // sequences are printed later
      s = (vl > 0 ? "..." : "");
    }
    else if (vr == vtkDICOMVR::LT ||
            vr == vtkDICOMVR::ST ||
            vr == vtkDICOMVR::UT)
    {
      // replace breaks with "\\", cap length to MAX_LENGTH
      size_t l = (MAX_LENGTH < vl ? MAX_LENGTH-4 : vl);
      const char* cp = v.GetCharData();
      std::string utf8;
      if (v.GetCharacterSet() != vtkDICOMCharacterSet::ISO_IR_6)
      {
        utf8 = v.GetCharacterSet().ConvertToUTF8(cp, l);
        l = utf8.length();
        cp = utf8.data();
      }
      size_t j = 0;
      while (j < l && cp[j] != '\0')
      {
        size_t k = j;
        size_t m = j;
        for (; j < l && cp[j] != '\0'; ++j)
        {
          m = j;
          if (cp[j] == '\r' || cp[j] == '\n' || cp[j] == '\f')
          {
            do
            {
              j++;
            } while (j < l && (cp[j] == '\r' || cp[j] == '\n'
                     || cp[j] == '\f'));
            break;
          }
          m++;
        }
        if (j == l)
        {
          while (m > 0 && cp[m-1] == ' ') { m--; }
        }
        if (k != 0)
        {
          s.append("\\\\");
        }
        s.append(&cp[k], m-k);
        if (MAX_LENGTH < vl)
        {
          s.append("...");
          break;
        }
      }
    }
    else
    {
      // print any other VR via conversion to string
      unsigned int n = v.GetNumberOfValues();
      size_t pos = 0;
      for (unsigned int i = 0; i < n; ++i)
      {
        v.AppendValueToUTF8String(s, i);
        if ((n - 1) > i)
        {
          s.append("\\");
        }
        if ((MAX_LENGTH-4) < s.size())
        {
          s.resize(pos);
          s.append("...");
          break;
        }
        pos = s.size();
      }
    }

    std::vector<std::string> current;
    std::string value;
    std::string quantity;
    if (meta && 0 == vi)
    {
      char buffer[8];
      snprintf(buffer, sizeof(buffer), "%04X", g);
      current.push_back(buffer);
      snprintf(buffer, sizeof(buffer), "%04X", e);
      current.push_back(buffer);
      current.push_back(vr.GetText());
      current.push_back(name);
    }
    if (meta && 1 < vn)
    {
      if (0 == vi)
        quantity = "multiple values";
    }
    if (vr == vtkDICOMVR::SQ)
    {
      size_t m = v.GetNumberOfValues();
      const vtkDICOMItem* items = v.GetSequenceData();
      quantity = vtkVariant(m).ToString() + (1 == m ? "item" : "items");
      current.push_back(value);
      current.push_back(quantity);
      if (collection)
        collection->push_back(current);
      for (size_t j = 0; j < m; ++j)
      {
        vtkDICOMDataElementIterator siter = items[j].Begin();
        vtkDICOMDataElementIterator siterEnd = items[j].End();
        for (; siter != siterEnd; ++siter)
        {
          printElement(
            meta, &items[j], siter, depth+1, pixelDataVL, collection);
        }
      }
    }
    else if (vl == 0xffffffffu)
    {
      value = "...";
      if (tag == DC::PixelData ||
         tag == DC::FloatPixelData ||
         tag == DC::DoubleFloatPixelData)
      {
        quantity= "compressed";
      }
      else
      {
        quantity = "delimited";
      }
      current.push_back(value);
      current.push_back(quantity);
      if (collection)
        collection->push_back(current);
    }
    else
    {
      const char* uidName = "";
      if (vr == vtkDICOMVR::UI)
      {
        uidName = vtkDICOMUtilities::GetUIDName(s.c_str());
      }
      if ('\0' != uidName[0])
      {
        value = s.c_str();
        value += " {";
        value += uidName;
        value += "}";
        quantity = vtkVariant(vl).ToString();
        quantity += " bytes";
      }
      else if (vr == vtkDICOMVR::OB ||
              vr == vtkDICOMVR::OW ||
              vr == vtkDICOMVR::OF ||
              vr == vtkDICOMVR::OD)
      {
        value = (0 == vl ? "" : "...");
        quantity = vtkVariant(vl).ToString();
        quantity += " bytes";
      }
      else
      {
        value = s;
        quantity = vtkVariant(vl).ToString();
        quantity += " bytes";
      }
      current.push_back(value);
      current.push_back(quantity);
      if (collection)
        collection->push_back(current);
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderDicomTagWidgetPrivate::buildDicomStrings()
{
  if (this->fileName.isEmpty() || this->fileName.isNull())
  {
    return;
  }

  this->data->Clear();
  this->data->SetNumberOfInstances(1);
  std::string name = this->fileName.toStdString();

  // make sure we can read the file
  vtkNew<vtkDICOMReader> reader;
  if (!reader->CanReadFile(name.c_str())) return;

  this->parser->SetFileName(name.c_str());
  this->parser->Update();
  vtkDICOMDataElementIterator iter = this->data->Begin();
  vtkDICOMDataElementIterator iterEnd = this->data->End();
  unsigned int pixelDataVL = parser->GetPixelDataVL();
  for (; iter != iterEnd; ++iter)
  {
    printElement(this->data, 0, iter, 0, pixelDataVL, &this->dicomStrings);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderDicomTagWidgetPrivate::updateUi()
{
  this->tableWidget->clearContents();
  QStringList labels;
  labels << "Group,Element";
  labels << "VR";
  labels << "Description";
  labels << "Value";
  labels << "Size";
  this->tableWidget->setHorizontalHeaderLabels(labels);
  this->tableWidget->setColumnCount(labels.size());

  this->dicomStrings.clear();
  this->buildDicomStrings();

  if (!this->dicomStrings.empty())
  {
    this->tableWidget->setRowCount(this->dicomStrings.size());
    int row = 0;
    for (auto it = this->dicomStrings.begin(); it != this->dicomStrings.end();
      ++it)
    {
      std::stringstream tag;
      int i = 0;
      int col = 0;
      for (auto vIt = (*it).begin(); vIt != (*it).end(); ++vIt)
      {
        if (0 == i) tag << "(" << *vIt << ",";
        else if (1 == i)
        {
          tag << *vIt << ")";
          QTableWidgetItem* item = new QTableWidgetItem();
          item->setText(tag.str().c_str());
          this->tableWidget->setItem(row, col++, item);
        }
        else
        {
          QTableWidgetItem* item = new QTableWidgetItem();
          item->setText((*vIt).c_str());
          this->tableWidget->setItem(row, col++, item);
        }
        i++;
      }
      row++;
    }
    this->tableWidget->resizeColumnsToContents();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderDicomTagWidget::QAlderDicomTagWidget(QWidget* parent)
  : Superclass(parent)
  , d_ptr(new QAlderDicomTagWidgetPrivate(*this))
{
  Q_D(QAlderDicomTagWidget);
  d->setupUi(this);
  d->updateUi();
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderDicomTagWidget::~QAlderDicomTagWidget()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderDicomTagWidget::load(const QString& fileName)
{
  Q_D(QAlderDicomTagWidget);
  d->setFileName(fileName);
  d->updateUi();
}
