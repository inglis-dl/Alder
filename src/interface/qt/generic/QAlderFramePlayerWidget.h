/*==============================================================================

  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Module:    QAlderFramePlayerWidget.cxx
  Language:  C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

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

/**
 * @class QAlderFramePlayerWidget
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Qt widget for playing cineloops.
 *
 * It encapsulates the VTK time animation control functionality and provides
 * slots and signals to manage them in a Qt application. The widget connects
 * itself to a *Viewer*, which controls display of an image.
 *
 * This class was adapted from the MSVTK library for playing cineloops.
 *
 * @see QAlderDoubleSlider, QAlderSliderWidget, vtkMedicalImageViewer, QAlderSliceView
 */
#ifndef __QAlderFramePlayerWidget_h
#define __QAlderFramePlayerWidget_h

// Qt includes
#include <QAbstractAnimation>
#include <QIcon>
#include <QPointer>
#include <QWidget>

// Alder includes
class vtkMedicalImageViewer;
class QAlderSliceView;
class QAlderFramePlayerWidgetPrivate;

class QAlderFramePlayerWidget : public QWidget
{
  Q_OBJECT

  /**
   * This property holds the firstFrame button's icon.
   * @see firstFrameIcon(), setFirstFrameIcon()
   */
  Q_PROPERTY(QIcon firstFrameIcon READ firstFrameIcon WRITE setFirstFrameIcon)

  /**
   * This property holds the previousFrame button's icon.
   * @see previousFrameIcon(), setPreviousFrameIcon()
   */
  Q_PROPERTY(QIcon previousFrameIcon READ previousFrameIcon WRITE setPreviousFrameIcon)

  /**
   * This property holds the play button's icon.
   * @see playIcon(), setPlayIcon()
   */
  Q_PROPERTY(QIcon playIcon READ playIcon WRITE setPlayIcon)

  /**
   * This property holds the play reverse button's icon.
   * @see playReverseIcon(), setPlayReverseIcon()
   */
  Q_PROPERTY(QIcon playReverseIcon READ playReverseIcon WRITE setPlayReverseIcon)

  /**
   * This property holds the nextFrame button's icon.
   * @see nextFrameIcon(), setNextFrameIcon()
   */
  Q_PROPERTY(QIcon nextFrameIcon READ nextFrameIcon WRITE setNextFrameIcon)

  /**
   * This property holds the lastFrame button's icon.
   * @see lastFrameIcon(), setLastFrameIcon()
   */
  Q_PROPERTY(QIcon lastFrameIcon READ lastFrameIcon WRITE setLastFrameIcon)

  /**
   * This property holds the repeat button's icon.
   * @see repeatIcon(), setRepeatIcon()
   */
  Q_PROPERTY(QIcon repeatIcon READ repeatIcon WRITE setRepeatIcon)

  /**
   * Enable/Disable the visibility of play reverse button.
   * @see playReverseVisibility(), setPlayReverseVisibility()
   */
  Q_PROPERTY(bool playReverseVisibility READ playReverseVisibility WRITE setPlayReverseVisibility)

  /**
   * Enable/Disable the visibility of the firstFrame and lastFrame buttons.
   * @see boundFramesVisibility(), setBoundFramesVisibility()
   */
  Q_PROPERTY(bool boundFramesVisibility READ boundFramesVisibility WRITE setBoundFramesVisibility)

  /**
   * Enable/Disable the visibility of the seekBackward and seekForward buttons.
   * @see goToVisibility(), setGoToVisibility()
   */
  Q_PROPERTY(bool goToVisibility READ goToVisibility WRITE setGoToVisibility)

  /**
   * Enable/Disable the visibility of time spinBox
   * @see frameSpinBoxVisibility(), setFrameSpinBoxVisibility()
   */
  Q_PROPERTY(bool frameSpinBoxVisibility READ frameSpinBoxVisibility WRITE setFrameSpinBoxVisibility)

  /**
   * This property holds the number of decimal digits for the frameSlider.
   * @see sliderDecimals(), setSliderDecimals()
   */
  Q_PROPERTY(int sliderDecimals READ sliderDecimals WRITE setSliderDecimals)

  /**
   * This property holds the page step for the frameSlider.
   * @see sliderPageStep(), setSliderPageStep()
   */
  Q_PROPERTY(double sliderPageStep READ sliderPageStep WRITE setSliderPageStep)

  /**
   * This property holds the single step for the frameSlider.
   * The automatic mode (by default) compute the step corresponding to one
   * frame. To get back to the automatic mode, set a negative value.
   * @see sliderSingleStep(), setSliderSingleStep()
   */
  Q_PROPERTY(double sliderSingleStep READ sliderSingleStep WRITE setSliderSingleStep)

  /**
   * This property holds the number of the higher frame per seconds rate the
   * the player will be willing to handle.
   * @see maxFrameRate(), setMaxFrameRate()
   */
  Q_PROPERTY(double maxFrameRate READ maxFrameRate WRITE setMaxFrameRate)

  /**
   * This property holds the direction on which the widget will do the
   * animation.
   * @see direction(), setDirection()
   */
  Q_PROPERTY(QAbstractAnimation::Direction direction READ direction WRITE setDirection NOTIFY directionChanged)

  /**
   *This property holds if the animation is repeated when we reach the end.
   * @see repeat(), setRepeat()
   */
  Q_PROPERTY(bool repeat READ repeat WRITE setRepeat)

  /**
   * This property holds the speed in frames per second of the animation.
   * @see playSpeed(), setPlaySpeed()
   */
  Q_PROPERTY(double playSpeed READ playSpeed WRITE setPlaySpeed)

  /**
   * This property is an accessor to the widget's current frame.
   * @see currentTime(), setCurrentTime()
   */
  Q_PROPERTY(double currentFrame READ currentFrame WRITE setCurrentFrame NOTIFY currentFrameChanged)

public:
  typedef QWidget Superclass;
  QAlderFramePlayerWidget(QWidget* parent=0);
  virtual ~QAlderFramePlayerWidget();

  //@{
  /** Set/Get the viewer to connect with. */
  void setViewer(vtkMedicalImageViewer* viewer);
  vtkMedicalImageViewer* viewer() const;
  //@}

  void setSliceView( QAlderSliceView* );

  //@{
  /** Set/Get the first frame icon. */
  void setFirstFrameIcon(const QIcon&);
  QIcon firstFrameIcon() const;
  //@}

  //@{
  /** Set/Get the previous frame icon. */
  void setPreviousFrameIcon(const QIcon&);
  QIcon previousFrameIcon() const;
  //@}

  /** Set/Get the play icon. */
  void setPlayIcon(const QIcon&);
  QIcon playIcon() const;
  //@}

  //@{
  /** Set/Get the reverse icon. */
  void setPlayReverseIcon(const QIcon&);
  QIcon playReverseIcon() const;
  //@}

  //@{
  /** Set/Get the icon of the next frame button. */
  void setNextFrameIcon(const QIcon&);
  QIcon nextFrameIcon() const;
  //@}

  //@{
  /** Set/Get the icon of the last frame button. */
  void setLastFrameIcon(const QIcon&);
  QIcon lastFrameIcon() const;
  //@}

  //@{
  /** Set the icon of the repeat button. */
  void setRepeatIcon(const QIcon&);
  QIcon repeatIcon() const;
  //@}

  //@{
  /** Set/Get playReverseVisibility */
  void setPlayReverseVisibility(bool visible);
  bool playReverseVisibility() const;
  //@}

  //@{
  /** Set/Get boundFramesVisibility */
  void setBoundFramesVisibility(bool visible);
  bool boundFramesVisibility() const;
  //@}

  //@{
  /** Set/Get goToVisibility */
  void setGoToVisibility(bool visible);
  bool goToVisibility() const;
  //@}

  //@{
  /** Set/Get frameSpinBoxVisibility */
  void setFrameSpinBoxVisibility(bool visible);
  bool frameSpinBoxVisibility() const;
  //@}

  //@{
  /** Set/Get sliderDecimals */
  void setSliderDecimals(int decimals);
  int sliderDecimals() const;
  //@}

  //@{
  /** Set/Get sliderPageStep */
  void setSliderPageStep(double pageStep);
  double sliderPageStep() const;
  //@}

  //@{
  /** Set/Get sliderSingleStep */
  void setSliderSingleStep(double singleStep);
  double sliderSingleStep() const;
  //@}

  //@{
  /** Set/Get playback direction */
  void setDirection(QAbstractAnimation::Direction playDirection);
  QAbstractAnimation::Direction direction() const;
  //@}

  //@{
  /** Set/Get repeat */
  void setRepeat(bool repeat);
  bool repeat() const;
  //@}

  //@{
  /** Set/Get maxFrameRate in frames per second */
  void setMaxFrameRate(double);
  double maxFrameRate() const;
  //@}

  /** Set currentFrame */
  double currentFrame() const;

  /** Set playSpeed in frames per second */
  double playSpeed() const;

public slots:
  /** Set currentFrame */
  virtual void setCurrentFrame(double frame);

  /** Set playSpeed in frames per second */
  virtual void setPlaySpeed(double speed);

  /**
   * Set the current frame to the current frame provided by the source.
   * @see goToPreviousFrame(), goToNextFrame(), goToLastFrame()
   */
  virtual void goToCurrentFrame();

  /**
   * Set the current frame to the first frame.
   * @see goToPreviousFrame(), goToNextFrame(), goToLastFrame()
   */
  virtual void goToFirstFrame();

  /**
   * Set the current frame to the previous frame.
   * @see goToFirstFrame(), goToNextFrame(), goToLastFrame()
   */
  virtual void goToPreviousFrame();

  /**
   * Set the current frame to the next frame.
   * @see goToFirstFrame(), goToPreviousFrame(), goToLastFrame()
   */
  virtual void goToNextFrame();

  /**
   * Set the current frame to the last frame.
   * @see goToFirstFrame(), goToPreviousFrame(), goToNextFrame(), goToLastFrame()
   */
  virtual void goToLastFrame();

  /**
   * Automatically browse all the frames in the direction order.
   * @see pause(), stop(), direction
   */
  virtual void play();

  /**
   * Pause the browse of the frames. To resume, call play().
   * @see play(), pause()
   */
  virtual void pause();

  /**
   * Call play() on true, pause() on false.
   * @see play(), pause()
   */
  void play(bool playPause);

  /**
   * Browse the frames in forward order.
   * @see playBackward(), play(), orientation
   */
  void playForward(bool playPause);

  /**
   * Browse the frames in backward order.
   * @see playForward(), play(), orientation
   */
  void playBackward(bool playPause);

  /**
   * Stop the browsing of the frames and go back to the first frame.
   * @see play(), pause()
   */
  void stop();

  /**
   * force an update to the widget's UI elements
   */
  virtual void update();

protected Q_SLOTS:
  virtual void onTick();

Q_SIGNALS:
  /** Emitted when the frame has been changed */
  void currentFrameChanged(double);

  /** Emitted when the internal timer sends a timeout */
  void onTimeout();

  /**
   * Emitted with playing(true) when play begins and
  * playing(false) when play ends.
   */
  void playing(bool);

  /** Emitted when the player loops the animation */
  void loop();

  /** Emitted when the playback direction is changed */
  void directionChanged(QAbstractAnimation::Direction);

protected:
  QScopedPointer<QAlderFramePlayerWidgetPrivate> d_ptr;
  QPointer<QAlderSliceView> sliceViewPointer;

private:
  Q_DECLARE_PRIVATE(QAlderFramePlayerWidget);
  Q_DISABLE_COPY(QAlderFramePlayerWidget);
};

#endif
