/*=========================================================================

  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Module:    QAlderSliderWidget.cxx
  Language:  C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/
#include <QAlderSliderWidget.h>
#include <ui_QAlderSliderWidget.h>

// Qt includes
#include <QDebug>
#include <QMouseEvent>

// STD includes
#include <cmath>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QAlderSliderWidgetPrivate: public Ui_QAlderSliderWidget
{
  Q_DECLARE_PUBLIC(QAlderSliderWidget);
  protected:
    QAlderSliderWidget* const q_ptr;

  public:
    explicit QAlderSliderWidgetPrivate(QAlderSliderWidget& object);
    virtual ~QAlderSliderWidgetPrivate();

    void updateSpinBoxWidth();
    int synchronizedSpinBoxWidth() const;
    void synchronizeSiblingSpinBox(int newWidth);
    bool equal(double spinBoxValue, double sliderValue) const
    {
      return qAbs(sliderValue - spinBoxValue) <
             std::pow(10., -this->SpinBox->decimals());
    }

    bool Tracking;
    bool Changing;
    double ValueBeforeChange;
    bool AutoSpinBoxWidth;
};

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderSliderWidgetPrivate::QAlderSliderWidgetPrivate(QAlderSliderWidget& object)
  :q_ptr(&object)
{
  this->Tracking = true;
  this->Changing = false;
  this->ValueBeforeChange = 0.;
  this->AutoSpinBoxWidth = true;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderSliderWidgetPrivate::~QAlderSliderWidgetPrivate()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidgetPrivate::updateSpinBoxWidth()
{
  int spinBoxWidth = this->synchronizedSpinBoxWidth();
  if (this->AutoSpinBoxWidth)
    {
    this->SpinBox->setMinimumWidth(spinBoxWidth);
    }
  else
    {
    this->SpinBox->setMinimumWidth(0);
    }
  this->synchronizeSiblingSpinBox(spinBoxWidth);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QAlderSliderWidgetPrivate::synchronizedSpinBoxWidth() const
{
  Q_Q(const QAlderSliderWidget);
  int maxWidth = this->SpinBox->sizeHint().width();
  if (!q->parent())
    {
    return maxWidth;
    }
  QList<QAlderSliderWidget*> siblings =
    q->parent()->findChildren<QAlderSliderWidget*>();
  foreach(QAlderSliderWidget* sibling, siblings)
    {
    maxWidth = qMax(maxWidth, sibling->d_func()->SpinBox->sizeHint().width());
    }
  return maxWidth;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidgetPrivate::synchronizeSiblingSpinBox(int width)
{
  Q_Q(const QAlderSliderWidget);
  QList<QAlderSliderWidget*> siblings =
    q->parent()->findChildren<QAlderSliderWidget*>();
  foreach(QAlderSliderWidget* sibling, siblings)
    {
    if (sibling != q && sibling->isAutoSpinBoxWidth())
      {
      sibling->d_func()->SpinBox->setMinimumWidth(width);
      }
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderSliderWidget::QAlderSliderWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new QAlderSliderWidgetPrivate(*this))
{
  Q_D(QAlderSliderWidget);

  d->setupUi(this);

  d->Slider->setMaximum(d->SpinBox->maximum());
  d->Slider->setMinimum(d->SpinBox->minimum());

  this->connect(d->SpinBox, SIGNAL(valueChanged(double)),
    d->Slider, SLOT(setValue(double)));
  this->connect(d->Slider, SIGNAL(sliderPressed()),
    this, SLOT(startChanging()));
  this->connect(d->Slider, SIGNAL(sliderReleased()),
    this, SLOT(stopChanging()));
  this->connect(d->Slider, SIGNAL(valueChanged(double)),
    this, SLOT(changeValue(double)));
  d->SpinBox->installEventFilter(this);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderSliderWidget::~QAlderSliderWidget()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderSliderWidget::minimum() const
{
  Q_D(const QAlderSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  return d->Slider->minimum();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderSliderWidget::maximum() const
{
  Q_D(const QAlderSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
  return d->Slider->maximum();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setMinimum(double min)
{
  Q_D(QAlderSliderWidget);
  bool wasBlocked = d->SpinBox->blockSignals(true);
  d->SpinBox->setMinimum(min);
  d->SpinBox->blockSignals(wasBlocked);

  // SpinBox can truncate min (depending on decimals).
  // use Spinbox's min to set Slider's min
  d->Slider->setMinimum(d->SpinBox->minimum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setMaximum(double max)
{
  Q_D(QAlderSliderWidget);
  bool wasBlocked = d->SpinBox->blockSignals(true);
  d->SpinBox->setMaximum(max);
  d->SpinBox->blockSignals(wasBlocked);

  // SpinBox can truncate max (depending on decimals).
  // use Spinbox's max to set Slider's max
  d->Slider->setMaximum(d->SpinBox->maximum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setRange(double min, double max)
{
  Q_D(QAlderSliderWidget);

  bool wasBlocked = d->SpinBox->blockSignals(true);
  d->SpinBox->setRange(min, max);
  d->SpinBox->blockSignals(wasBlocked);

  // SpinBox can truncate the range (depending on decimals).
  // use Spinbox's range to set Slider's range
  d->Slider->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderSliderWidget::value() const
{
  Q_D(const QAlderSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  return d->Changing ? d->ValueBeforeChange : d->Slider->value();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setValue(double _value)
{
  Q_D(QAlderSliderWidget);
  // disable the tracking temporally to emit the
  // signal valueChanged if changeValue() is called
  bool isChanging = d->Changing;
  d->Changing = false;
  d->SpinBox->setValue(_value);
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
  // restore the prop
  d->Changing = isChanging;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::startChanging()
{
  Q_D(QAlderSliderWidget);
  if (d->Tracking)
    {
    return;
    }
  d->Changing = true;
  d->ValueBeforeChange = this->value();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::stopChanging()
{
  Q_D(QAlderSliderWidget);
  if (d->Tracking)
    {
    return;
    }
  d->Changing = false;
  if (qAbs(this->value() - d->ValueBeforeChange) >
      (this->singleStep() * 0.000000001))
    {
    emit this->valueChanged(this->value());
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::changeValue(double newValue)
{
  Q_D(QAlderSliderWidget);

  bool wasBlocked = d->SpinBox->blockSignals(true);
  d->SpinBox->setValue(newValue);
  d->SpinBox->blockSignals(wasBlocked);
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));

  if (!d->Tracking)
    {
    emit this->valueIsChanging(newValue);
    }
  if (!d->Changing)
    {
    emit this->valueChanged(newValue);
    }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderSliderWidget::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::MouseButtonPress)
    {
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent->button() & Qt::LeftButton)
      {
      this->startChanging();
      }
    }
  else if (event->type() == QEvent::MouseButtonRelease)
    {
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent->button() & Qt::LeftButton)
      {
      // here we might prevent QAlderSliderWidget::stopChanging
      // from sending a valueChanged() event as the spinbox might
      // send a valueChanged() after eventFilter() is done.
      this->stopChanging();
      }
    }
  // standard event processing
  return this->Superclass::eventFilter(obj, event);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderSliderWidget::singleStep() const
{
  Q_D(const QAlderSliderWidget);
  Q_ASSERT(d->equal(d->SpinBox->singleStep(), d->Slider->singleStep()));
  return d->Slider->singleStep();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setSingleStep(double step)
{
  Q_D(QAlderSliderWidget);
  d->SpinBox->setSingleStep(step);
  d->Slider->setSingleStep(d->SpinBox->singleStep());
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderSliderWidget::pageStep() const
{
  Q_D(const QAlderSliderWidget);
  return d->Slider->pageStep();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setPageStep(double step)
{
  Q_D(QAlderSliderWidget);
  d->Slider->setPageStep(step);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QAlderSliderWidget::decimals() const
{
  Q_D(const QAlderSliderWidget);
  return d->SpinBox->decimals();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setDecimals(int newDecimals)
{
  Q_D(QAlderSliderWidget);
  d->SpinBox->setDecimals(newDecimals);
  // The number of decimals can change the range values
  // i.e. 50.55 with 2 decimals -> 51 with 0 decimals
  // As the SpinBox range change doesn't fire signals,
  // we have to do the synchronization manually here
  d->Slider->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
  Q_ASSERT(d->equal(d->SpinBox->minimum(), d->Slider->minimum()));
  Q_ASSERT(d->equal(d->SpinBox->value(), d->Slider->value()));
  Q_ASSERT(d->equal(d->SpinBox->maximum(), d->Slider->maximum()));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QAlderSliderWidget::prefix() const
{
  Q_D(const QAlderSliderWidget);
  return d->SpinBox->prefix();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setPrefix(const QString& newPrefix)
{
  Q_D(QAlderSliderWidget);
  d->SpinBox->setPrefix(newPrefix);
#if QT_VERSION < 0x040800
  /// Setting the prefix doesn't recompute the sizehint, do it manually here:
  /// See: http://bugreports.qt.nokia.com/browse/QTBUG-9530
  d->SpinBox->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
#endif
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QString QAlderSliderWidget::suffix() const
{
  Q_D(const QAlderSliderWidget);
  return d->SpinBox->suffix();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setSuffix(const QString& newSuffix)
{
  Q_D(QAlderSliderWidget);
  d->SpinBox->setSuffix(newSuffix);
#if QT_VERSION < 0x040800
  /// Setting the suffix doesn't recompute the sizehint, do it manually here:
  /// See: http://bugreports.qt.nokia.com/browse/QTBUG-9530
  d->SpinBox->setRange(d->SpinBox->minimum(), d->SpinBox->maximum());
#endif
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderSliderWidget::tickInterval() const
{
  Q_D(const QAlderSliderWidget);
  return d->Slider->tickInterval();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setTickInterval(double ti)
{
  Q_D(QAlderSliderWidget);
  d->Slider->setTickInterval(ti);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::reset()
{
  this->setValue(0.);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setSpinBoxAlignment(Qt::Alignment alignment)
{
  Q_D(QAlderSliderWidget);
  return d->SpinBox->setAlignment(alignment);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
Qt::Alignment QAlderSliderWidget::spinBoxAlignment() const
{
  Q_D(const QAlderSliderWidget);
  return d->SpinBox->alignment();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setTracking(bool enable)
{
  Q_D(QAlderSliderWidget);
  d->Tracking = enable;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderSliderWidget::hasTracking() const
{
  Q_D(const QAlderSliderWidget);
  return d->Tracking;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderSliderWidget::isAutoSpinBoxWidth() const
{
  Q_D(const QAlderSliderWidget);
  return d->AutoSpinBoxWidth;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setAutoSpinBoxWidth(bool autoWidth)
{
  Q_D(QAlderSliderWidget);
  d->AutoSpinBoxWidth = autoWidth;
  d->updateSpinBoxWidth();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderSliderWidget::isSpinBoxVisible() const
{
  Q_D(const QAlderSliderWidget);
  return d->SpinBox->isVisibleTo(const_cast<QAlderSliderWidget*>(this));
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderSliderWidget::setSpinBoxVisible(bool visible)
{
  Q_D(QAlderSliderWidget);
  d->SpinBox->setVisible(visible);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QDoubleSpinBox* QAlderSliderWidget::spinBox()
{
  Q_D(QAlderSliderWidget);
  return d->SpinBox;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderDoubleSlider* QAlderSliderWidget::slider()
{
  Q_D(QAlderSliderWidget);
  return d->Slider;
}
