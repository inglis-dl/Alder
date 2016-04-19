/*==============================================================================

  Module:    QAlderFramePlayerWidget.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Patrick Emond <emondpd AT mcmaster DOT ca>
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

  Library: MSVTK

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

==============================================================================*/
#include <QAlderFramePlayerWidget.h>
#include <ui_QAlderFramePlayerWidget.h>

// Alder includes
#include <Common.h>
#include <vtkMedicalImageViewer.h>
#include <QAlderSliceView.h>

// Qt includes
#include <QIcon>
#include <QTime>
#include <QTimer>

// VTK includes
#include <vtkEventQtSlotConnect.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QAlderFramePlayerWidgetPrivate : public Ui_QAlderFramePlayerWidget
{
  Q_DECLARE_PUBLIC(QAlderFramePlayerWidget);
protected:
  QAlderFramePlayerWidget* const q_ptr;

  vtkSmartPointer< vtkMedicalImageViewer > viewer;
  vtkSmartPointer< vtkEventQtSlotConnect > connector;

  double maxFrameRate;                    // Time Playing speed factor.
  QTimer* timer;                          // Timer to process the player.
  QTime realTime;                         // Time to reference the real one.
  QAbstractAnimation::Direction direction;// Sense of direction

public:
  QAlderFramePlayerWidgetPrivate(QAlderFramePlayerWidget& object);
  virtual ~QAlderFramePlayerWidgetPrivate();

  virtual void setupUi(QWidget*);
  virtual void updateUi();

  struct PipelineInfoType
  {
    PipelineInfoType();

    bool isConnected;
    unsigned int numberOfFrames;
    double frameRange[2];
    double currentFrame;
    int maxFrameRate;

    void printSelf() const;
    double clampTimeInterval(double, double) const; // Transform a frameRate into a time interval
    double validateFrame(double) const;    // Validate a frame
    double nextFrame() const;     // Get the next frame.
    double previousFrame() const; // Get the previous frame.
  };

  PipelineInfoType retrievePipelineInfo();            // Get pipeline information.
  virtual void processRequest(double);                // Request Data and Update
  virtual bool isConnected();                         // Check if the pipeline is ready
  virtual void processRequest(const PipelineInfoType&, double); // Request Data and Update
  virtual void requestData(const PipelineInfoType&, double);    // Request Data by time
  virtual void updateUi(const PipelineInfoType&);               // Update the widget giving pipeline statut
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderFramePlayerWidgetPrivate methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidgetPrivate::QAlderFramePlayerWidgetPrivate
(QAlderFramePlayerWidget& object)
  : q_ptr(&object)
{
  this->maxFrameRate = 60;          // 60 FPS by default
  this->viewer = 0;
  this->connector = 0;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidgetPrivate::~QAlderFramePlayerWidgetPrivate()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidgetPrivate::PipelineInfoType::PipelineInfoType()
  : isConnected(false)
  , numberOfFrames(0)
  , currentFrame(0)
  , maxFrameRate(60)
{
  this->frameRange[0] = 0;
  this->frameRange[1] = 0;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::PipelineInfoType::printSelf()const
{
  std::cout << "---------------------------------------------------------------" << std::endl
            << "Pipeline info: " << this << std::endl
            << "Number of image frames: " << this->numberOfFrames << std::endl
            << "Frame range: " << this->frameRange[0] << " " << this->frameRange[1] << std::endl
            << "Last frame request: " << this->currentFrame << std::endl
            << "Maximum frame rate: " << this->maxFrameRate << std::endl
            << "Is connected: " << this->isConnected << std::endl;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidgetPrivate::PipelineInfoType
QAlderFramePlayerWidgetPrivate::retrievePipelineInfo()
{
  Q_Q(QAlderFramePlayerWidget);
  PipelineInfoType pipeInfo;

  if( this->viewer )
  {
    pipeInfo.isConnected = this->isConnected();
    if( !pipeInfo.isConnected )
    {
      return pipeInfo;
    }
    if( 3 > this->viewer->GetImageDimensionality() )
    {
      return pipeInfo;
    }

    pipeInfo.frameRange[0] = this->viewer->GetSliceMin();
    pipeInfo.frameRange[1] = this->viewer->GetSliceMax();
    pipeInfo.numberOfFrames = this->viewer->GetNumberOfSlices();
    pipeInfo.currentFrame = this->viewer->GetSlice();
    pipeInfo.maxFrameRate = this->viewer->GetMaxFrameRate();
  }
  else if( !q->sliceViewPointer.isNull() )
  {
    pipeInfo.isConnected = !q->sliceViewPointer.isNull();
    if( !pipeInfo.isConnected )
    {
      return pipeInfo;
    }
    if( 3 > q->sliceViewPointer.data()->dimensionality() )
    {
      pipeInfo.isConnected = false;
      return pipeInfo;
    }

    pipeInfo.frameRange[0] = q->sliceViewPointer->sliceMin();
    pipeInfo.frameRange[1] = q->sliceViewPointer->sliceMax();
    pipeInfo.numberOfFrames = pipeInfo.frameRange[1] - pipeInfo.frameRange[0] + 1;
    pipeInfo.currentFrame = q->sliceViewPointer->slice();
    pipeInfo.maxFrameRate = q->sliceViewPointer->frameRate();
  }
  return pipeInfo;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidgetPrivate::PipelineInfoType::
clampTimeInterval(double playbackSpeed, double maxFrameRate) const
{
  Q_ASSERT(playbackSpeed > 0.);

  // the time interval is the time between QTimer Q_EMITting
  // timeout signals, which in turn fires onTick, wherein the
  // frame is selected and displayed by the viewer
  // the playback speed is set in frames per second: eg., 60 FPS

  // Clamp the frame rate
  double rate = qMin( playbackSpeed, maxFrameRate);

  // return the time interval
  return  1000. / rate;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidgetPrivate::PipelineInfoType::validateFrame(double frame) const
{
  if( 0 == this->numberOfFrames )
    return vtkMath::Nan();
  else if ( 1 == this->numberOfFrames )
    return this->frameRange[0];
  return frame;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidgetPrivate::PipelineInfoType::previousFrame() const
{
  return this->validateFrame( this->currentFrame - 1 );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidgetPrivate::PipelineInfoType::nextFrame() const
{
  return this->validateFrame( this->currentFrame + 1 );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::setupUi( QWidget* widget )
{
  Q_Q(QAlderFramePlayerWidget);

  this->Ui_QAlderFramePlayerWidget::setupUi( widget );
  this->timer = new QTimer( widget );

  // Connect Menu ToolBars actions
  q->connect( this->firstFrameButton, SIGNAL(pressed()), q, SLOT(goToFirstFrame()) );
  q->connect( this->previousFrameButton, SIGNAL(pressed()), q, SLOT(goToPreviousFrame()) );
  q->connect( this->playButton, SIGNAL(toggled(bool)), q, SLOT(playForward(bool)) );
  q->connect( this->playReverseButton, SIGNAL(toggled(bool)), q, SLOT(playBackward(bool)) );
  q->connect( this->nextFrameButton, SIGNAL(pressed()), q, SLOT(goToNextFrame()) );
  q->connect( this->lastFrameButton, SIGNAL(pressed()), q, SLOT(goToLastFrame()) );
  q->connect( this->speedSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setPlaySpeed(double)) );

  // Connect the time slider
  q->connect( this->frameSlider, SIGNAL(valueChanged(double)), q, SLOT(setCurrentFrame(double)) );
  this->frameSlider->setSuffix("");
  this->frameSlider->setDecimals(0);
  this->frameSlider->setSingleStep(1);

  // Connect the Timer for animation
  q->connect(this->timer, SIGNAL(timeout()), q, SLOT(onTick()));
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::updateUi()
{
  PipelineInfoType pipeInfo = this->retrievePipelineInfo();
  this->updateUi( pipeInfo );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::updateUi(const PipelineInfoType& pipeInfo)
{
  // Buttons
  this->firstFrameButton->setEnabled( (pipeInfo.currentFrame > pipeInfo.frameRange[0]) );
  this->previousFrameButton->setEnabled( (pipeInfo.currentFrame > pipeInfo.frameRange[0]) );
  this->playButton->setEnabled( (pipeInfo.numberOfFrames > 1) );
  this->playReverseButton->setEnabled( (pipeInfo.numberOfFrames > 1) );
  this->nextFrameButton->setEnabled( (pipeInfo.currentFrame < pipeInfo.frameRange[1]) );
  this->lastFrameButton->setEnabled( (pipeInfo.currentFrame < pipeInfo.frameRange[1]) );
  this->repeatButton->setEnabled( (pipeInfo.numberOfFrames > 1) );
  this->speedSpinBox->setEnabled( (pipeInfo.numberOfFrames > 1) );

  // Slider
  this->frameSlider->blockSignals( true );
  this->frameSlider->setEnabled( (pipeInfo.frameRange[0] != pipeInfo.frameRange[1]) );
  this->frameSlider->setRange( pipeInfo.frameRange[0], pipeInfo.frameRange[1] );
  this->frameSlider->setValue( pipeInfo.currentFrame );
  this->frameSlider->blockSignals( false );

  // SpinBox
  // the max frame rate from the pipeinfo object is set from the viewer's frame rate information.
  // The value of the speed spin box set here is a suggested
  // value.  The speed can be set and is clamped between 1 and whatever the max frame
  // rate set through the QAlderFramePlayerWidget's maxFrameRate property.
  this->speedSpinBox->blockSignals( true );
  this->speedSpinBox->setValue(
    qMin( this->speedSpinBox->value(), this->maxFrameRate ) );
  this->speedSpinBox->blockSignals( false );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::requestData(const PipelineInfoType& pipeInfo,
                                              double frame)
{
  Q_Q(QAlderFramePlayerWidget);

  // We clamp the time requested
  frame = qBound( pipeInfo.frameRange[0],
            static_cast<double>(vtkMath::Round(frame)),
            pipeInfo.frameRange[1] );

  // Abort the request
  if (!pipeInfo.isConnected || frame == pipeInfo.currentFrame)
    return;

  if( this->viewer )
  {
    this->viewer->SetSlice( frame );
  }
  else if( !q->sliceViewPointer.isNull() )
  {
    q->sliceViewPointer.data()->setSlice( frame );
  }

  Q_EMIT q->currentFrameChanged(frame); // Emit the change
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::processRequest(double frame)
{
  PipelineInfoType pipeInfo = this->retrievePipelineInfo();
  this->processRequest( pipeInfo, frame );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::processRequest(const PipelineInfoType& pipeInfo,
                                                 double frame)
{
  if( vtkMath::IsNan(frame) )
    return;

  this->requestData( pipeInfo, frame );
  this->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidgetPrivate::isConnected()
{
  return this->viewer && this->viewer->GetInput() && this->connector;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderFramePlayerWidget methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidget::QAlderFramePlayerWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new QAlderFramePlayerWidgetPrivate(*this))
{
  Q_D(QAlderFramePlayerWidget);
  d->setupUi(this);
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidget::~QAlderFramePlayerWidget()
{
  Q_D(QAlderFramePlayerWidget);
  this->stop();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setViewer(vtkMedicalImageViewer* viewer)
{
  Q_D(QAlderFramePlayerWidget);
  if( d->viewer && d->connector )
  {
    d->connector->Disconnect(
      d->viewer, Alder::Common::OrientationChangedEvent,
      this, SLOT( goToCurrentFrame() ) );
  }
  d->viewer = viewer;
  if( viewer )
  {
    if( !d->connector )
      d->connector =
        vtkSmartPointer< vtkEventQtSlotConnect >::New();

    d->connector->Connect(
      viewer, Alder::Common::OrientationChangedEvent,
      this, SLOT( goToCurrentFrame() ) );
  }
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkMedicalImageViewer* QAlderFramePlayerWidget::viewer() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->viewer;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::update()
{
  Q_D(QAlderFramePlayerWidget);
  if( !this->sliceViewPointer.isNull() )
  {
    int frameRate = this->sliceViewPointer.data()->frameRate();
    this->setMaxFrameRate( frameRate );
    this->setPlaySpeed( frameRate );
  }
  d->updateUi();
  this->goToCurrentFrame();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setSliceView( QAlderSliceView* view )
{
  Q_D(QAlderFramePlayerWidget);

  if( d->viewer )
    this->setViewer( 0 );

  this->sliceViewPointer = view;
  if( !this->sliceViewPointer.isNull() )
  {
    connect( this->sliceViewPointer.data(), SIGNAL( imageDataChanged() ),
             this, SLOT( update() ) );
  }
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::goToCurrentFrame()
{
  Q_D(QAlderFramePlayerWidget);

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.currentFrame);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::goToFirstFrame()
{
  Q_D(QAlderFramePlayerWidget);

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.frameRange[0]);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::goToPreviousFrame()
{
  Q_D(QAlderFramePlayerWidget);

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest( pipeInfo, pipeInfo.previousFrame() );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::goToNextFrame()
{
  Q_D(QAlderFramePlayerWidget);

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest( pipeInfo, pipeInfo.nextFrame() );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::goToLastFrame()
{
  Q_D(QAlderFramePlayerWidget);

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest( pipeInfo, pipeInfo.frameRange[1] );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::play(bool playPause)
{
  if( !playPause )
    this->pause();
  if( playPause )
    this->play();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::play()
{
  Q_D(QAlderFramePlayerWidget);

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();
  double period = pipeInfo.frameRange[1] - pipeInfo.frameRange[0];

  if( !pipeInfo.isConnected || 0 == period )
    return;

  if( QAbstractAnimation::Forward == d->direction )
  {
    d->playReverseButton->blockSignals(true);
    d->playReverseButton->setChecked(false);
    d->playReverseButton->blockSignals(false);

    // Use when set the play by script
    if( !d->playButton->isChecked() )
    {
      d->playButton->blockSignals(true);
      d->playButton->setChecked(true);
      d->playButton->blockSignals(false);
    }

    // We reset the Slider to the initial value if we play from the end
    if( pipeInfo.currentFrame == pipeInfo.frameRange[1] )
      d->frameSlider->setValue(pipeInfo.frameRange[0]);
  }
  else if( QAbstractAnimation::Backward == d->direction )
  {
    d->playButton->blockSignals(true);
    d->playButton->setChecked(false);
    d->playButton->blockSignals(false);

    // Use when set the play by script
    if( !d->playReverseButton->isChecked() )
    {
      d->playReverseButton->blockSignals(true);
      d->playReverseButton->setChecked(true);
      d->playReverseButton->blockSignals(false);
    }

    // We reset the Slider to the initial value if we play from the beginning
    if( pipeInfo.currentFrame == pipeInfo.frameRange[0] )
      d->frameSlider->setValue(pipeInfo.frameRange[1]);
  }

  double timeInterval =
    pipeInfo.clampTimeInterval( d->speedSpinBox->value(), d->maxFrameRate );

  d->realTime.start();
  d->timer->start( timeInterval );
  Q_EMIT this->playing( true );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::pause()
{
  Q_D(QAlderFramePlayerWidget);

  if( QAbstractAnimation::Forward == d->direction )
    d->playButton->setChecked(false);
  else if( QAbstractAnimation::Backward == d->direction )
    d->playReverseButton->setChecked(false);

  if( d->timer->isActive() )
  {
    d->timer->stop();
    Q_EMIT this->playing(false);
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::stop()
{
  this->pause();
  this->goToFirstFrame();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::playForward( bool play )
{
  this->setDirection( QAbstractAnimation::Forward );
  this->play( play );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::playBackward( bool play )
{
  this->setDirection( QAbstractAnimation::Backward );
  this->play( play );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::onTick()
{
  Q_D(QAlderFramePlayerWidget);

  // Forward the internal timer timeout signal
  Q_EMIT this->onTimeout();

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  // currentFrame + number of milliseconds since starting x speed x direction
  double sec = d->realTime.restart() / 1000.;
  double frameRequest = pipeInfo.currentFrame + sec *
                       d->speedSpinBox->value() *
                       ((d->direction == QAbstractAnimation::Forward) ? 1 : -1);

  if( d->playButton->isChecked() && !d->playReverseButton->isChecked() )
  {
    if( frameRequest > pipeInfo.frameRange[1] && !d->repeatButton->isChecked() )
    {
      d->processRequest( pipeInfo, frameRequest );
      this->playForward( false );
      return;
    }
    else if( frameRequest > pipeInfo.frameRange[1] &&
             d->repeatButton->isChecked() )
    { // We Loop
      frameRequest = pipeInfo.frameRange[0];
      Q_EMIT this->loop();
    }
  }
  else if( !d->playButton->isChecked() && d->playReverseButton->isChecked() )
  {
    if( frameRequest < pipeInfo.frameRange[0] && !d->repeatButton->isChecked() )
    {
      d->processRequest( pipeInfo, frameRequest );
      this->playBackward( false );
      return;
    }
    else if( frameRequest < pipeInfo.frameRange[0] &&
             d->repeatButton->isChecked())
    { // We Loop
      frameRequest = pipeInfo.frameRange[1];
      Q_EMIT this->loop();
    }
  }
  else
  {
    return; // Undefined status
  }

  d->processRequest( pipeInfo, frameRequest );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setCurrentFrame( double frame )
{
  Q_D(QAlderFramePlayerWidget);
  d->processRequest( frame );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setPlaySpeed( double speed )
{
  Q_D(QAlderFramePlayerWidget);
  speed = speed <= 0. ? 1. : speed;
  d->speedSpinBox->setValue( speed );

  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  double timeInterval =
    pipeInfo.clampTimeInterval(speed, d->maxFrameRate);
  d->timer->setInterval( timeInterval );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderFramePlayerWidget methods -- Widgets Interface
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setFirstFrameIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->firstFrameButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setPreviousFrameIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->previousFrameButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setPlayIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->playButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setPlayReverseIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->playReverseButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setNextFrameIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->nextFrameButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setLastFrameIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->lastFrameButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setRepeatIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->repeatButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::firstFrameIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->firstFrameButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::previousFrameIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->previousFrameButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::playIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->playButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::playReverseIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->playReverseButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::nextFrameIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->nextFrameButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::lastFrameIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->lastFrameButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::repeatIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->repeatButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setPlayReverseVisibility(bool visible)
{
  Q_D(QAlderFramePlayerWidget);
  d->playReverseButton->setVisible(visible);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setBoundFramesVisibility(bool visible)
{
  Q_D(QAlderFramePlayerWidget);

  d->firstFrameButton->setVisible(visible);
  d->lastFrameButton->setVisible(visible);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setGoToVisibility(bool visible)
{
  Q_D(QAlderFramePlayerWidget);

  d->previousFrameButton->setVisible(visible);
  d->nextFrameButton->setVisible(visible);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setFrameSpinBoxVisibility(bool visible)
{
  Q_D(QAlderFramePlayerWidget);
  d->frameSlider->setSpinBoxVisible(visible);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidget::playReverseVisibility() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->playReverseButton->isVisibleTo(
    const_cast<QAlderFramePlayerWidget*>(this));
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidget::boundFramesVisibility() const
{
  Q_D(const QAlderFramePlayerWidget);
  return (d->firstFrameButton->isVisibleTo(
            const_cast<QAlderFramePlayerWidget*>(this)) &&
          d->lastFrameButton->isVisibleTo(
            const_cast<QAlderFramePlayerWidget*>(this)));
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidget::goToVisibility() const
{
  Q_D(const QAlderFramePlayerWidget);
  return (d->previousFrameButton->isVisibleTo(
            const_cast<QAlderFramePlayerWidget*>(this)) &&
          d->nextFrameButton->isVisibleTo(
            const_cast<QAlderFramePlayerWidget*>(this)));
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidget::frameSpinBoxVisibility() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->frameSlider->spinBox()->isVisibleTo(
    const_cast<QAlderFramePlayerWidget*>(this));
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setSliderDecimals(int decimals)
{
  Q_D(QAlderFramePlayerWidget);
  d->frameSlider->setDecimals(decimals);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setSliderPageStep(double pageStep)
{
  Q_D(QAlderFramePlayerWidget);
  d->frameSlider->setPageStep(pageStep);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setSliderSingleStep(double singleStep)
{
  Q_D(QAlderFramePlayerWidget);

  if( singleStep < 0. )
    return;

  d->frameSlider->setSingleStep(singleStep);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QAlderFramePlayerWidget::sliderDecimals() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->frameSlider->decimals();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidget::sliderPageStep() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->frameSlider->pageStep();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidget::sliderSingleStep() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->frameSlider->singleStep();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setDirection(QAbstractAnimation::Direction direction)
{
  Q_D(QAlderFramePlayerWidget);

  if( direction != d->direction )
  {
    d->direction = direction;
    Q_EMIT this->directionChanged(direction);
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAbstractAnimation::Direction QAlderFramePlayerWidget::direction() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->direction;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setRepeat(bool repeat)
{
  Q_D(const QAlderFramePlayerWidget);
  d->repeatButton->setChecked(repeat);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidget::repeat() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->repeatButton->isChecked();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setMaxFrameRate(double frameRate)
{
  Q_D(QAlderFramePlayerWidget);
  // Clamp frameRate min value
  frameRate = (0 >= frameRate ) ? 60 : frameRate;
  d->maxFrameRate = frameRate;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidget::maxFrameRate() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->maxFrameRate;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidget::currentFrame() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->frameSlider->value();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidget::playSpeed() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->speedSpinBox->value();
}
