/*=======================================================================

  Module:    vtkMedicalImageViewer.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Patrick Emond <emondpd AT mcmaster DOT ca>
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class vtkMedicalImageViewer
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Display a 2D image.
 *
 * vtkMedicalImageViewer is a convenience class for
 * displaying a 2D image.  It packages up the functionality found in
 * vtkRenderWindow, vtkRenderer, vtkImageSlice and
 * vtkImageWindowLevel into a single easy to use
 * class.  This class also creates an image interactor style
 * (vtkCustomInteractorStyleImage) that allows zooming and panning of images, and
 * supports interactive window/level operations on the image. Note that
 * vtkMedicalImageViewer is simply a wrapper around these classes.
 *
 * vtkMedicalImageViewer uses the 3D rendering and texture mapping engine
 * to draw an image on a plane.  This allows for rapid rendering,
 * zooming, and panning. The image is placed in the 3D scene at a
 * depth based on the z-coordinate of the particular image slice. Each
 * call to SetSlice() changes the image data (slice) displayed AND
 * changes the depth of the displayed slice in the 3D scene. This can
 * be controlled by the AutoAdjustCameraClippingRange ivar of the
 * InteractorStyle member.
 *
 * It is possible to mix images and geometry, using the methods:
 * viewer->SetInputData( myImage );
 * viewer->GetRenderer()->AddActor( myActor );
 *
 * This can be used to annotate an image with a PolyData of "edges" or
 * or highlight sections of an image or display a 3D isosurface
 * with a slice from the volume, etc. Any portions of your geometry
 * that are in front of the displayed slice will be visible; any
 * portions of your geometry that are behind the displayed slice will
 * be obscured. A more general framework (with respect to viewing
 * direction) for achieving this effect is provided by the
 * vtkImagePlaneWidget .
 *
 * Note that pressing 'r' will reset the window/level and pressing
 * shift+'r' or control+'r' will reset the camera.
 *
 * @see vtkRenderWindow vtkRenderer vtkImageSlice vtkImageWindowLevel
 */
#ifndef __vtkMedicalImageViewer_h
#define __vtkMedicalImageViewer_h

#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include <vector>

class vtkAxes;
class vtkAxesActor;
class vtkCustomCornerAnnotation;
class vtkCustomInteractorStyleImage;
class vtkImageCoordinateWidget;
class vtkImageData;
class vtkImageSlice;
class vtkImageSliceMapper;
class vtkImageWindowLevel;
class vtkOrientationMarkerWidget;
class vtkOutlineCornerSource;
class vtkPolyDataMapper;
class vtkPropAssembly;
class vtkRenderer;
class vtkRenderWindow;
class vtkRenderWindowInteractor;

class vtkAnimationScene;
class vtkAnimationCue;
class vtkFrameAnimationPlayer;

class vtkMedicalImageViewer : public vtkObject
{
public:
  static vtkMedicalImageViewer *New();
  vtkTypeMacro( vtkMedicalImageViewer, vtkObject );
  void PrintSelf( ostream& os, vtkIndent indent );

  /**
   * Force render the display.
   * In some situations, a pipeline update will not cause a
   * refresh render.  This method allows for forcing a render.
   */
  virtual void Render();

  //@{
  /**
   * Set/Get the input image to the viewer.
   * @param input vtkImageData from the output of a reader
   */
  virtual void SetInputData( vtkImageData* input );
  virtual vtkImageData* GetInput();
  //@}

  /**
   * Get the input image to the vtkImageSlice.
   * The input is the vtkImageWindowLevel filter output.
   */
  virtual vtkImageData* GetDisplayInput();

  /**
   * Load an image from file and display.
   *
   * If fileName is valid, load the file via vtkGDCMImageReader and display it.
   * Returns fails if image fails to load.
   * @param fileName Name of a file on disk
   * @return boolean
   */
   bool Load( const std::string& fileName );

  /**
   * Enum constants for orthonormal slice orientations. */
  enum
  {
    VIEW_ORIENTATION_YZ = 0,  /**< enum value VIEW_ORIENTATION_YZ. */
    VIEW_ORIENTATION_XZ = 1,  /**< enum value VIEW_ORIENTATION_XZ. */
    VIEW_ORIENTATION_XY = 2   /**< enum value VIEW_ORIENTATION_XY. */
  };

  //@{
  /**
   * Set/Get the orthonormal view orientation. */
  vtkGetMacro( ViewOrientation, int );
  virtual void SetViewOrientation( const int& );
  virtual void SetViewOrientationToXY()
    { this->SetViewOrientation(vtkMedicalImageViewer::VIEW_ORIENTATION_XY); };
  virtual void SetViewOrientationToYZ()
    { this->SetViewOrientation(vtkMedicalImageViewer::VIEW_ORIENTATION_YZ); };
  virtual void SetViewOrientationToXZ()
    { this->SetViewOrientation(vtkMedicalImageViewer::VIEW_ORIENTATION_XZ); };
  //@}

  //@{
  /**
   * Rotate the camera. */
  void RotateCamera( const double& );
  //@}

  //@{
  /**
   * Flip the camera horizontally. */
  void FlipCameraHorizontal();
  //@}

  //@{
  /**
   * Flip the camera horizontally. */
  void FlipCameraVertical();
  //@}

  //@{
  /**
   * Set/Get the current slice index to display.
   * Depending on the orientation, this can be an X, Y or Z slice.
   * @see GetSliceLocation()
   */
  vtkGetMacro( Slice, int );
  virtual void SetSlice( int );
  //@}

  /**
   * Get the non-indexed location of the slice from the origin.
   * @sa GetSlice(), SetSlice()
   */
  double GetSliceLocation();

  /**
   * Update the display extent.
   * Update the display extent so that the proper slice for the
   * given orientation is displayed. It will also try to set a
   * reasonable camera clipping range.
   * This method is called automatically when the Input is changed, but
   * most of the time the input of this class is likely to remain the same,
   * i.e. connected to the output of a filter, or an image reader. When the
   * input of this filter or reader itself is changed, an error message might
   * be displayed since the current display extent is probably outside
   * the new whole extent. Calling this method will ensure that the display
   * extent is reset properly.
   */
  //virtual void UpdateDisplayExtent();

  /**
   * Get the minimum slice index.
   * @sa GetSliceMax(), GetSliceRange()
   */
  virtual int GetSliceMin();

  /**
   * Get the maximum slice index.
   * @sa GetSliceMin(), GetSliceRange()
   */
  virtual int GetSliceMax();

  /**
   * Get the number of slices.
   * @sa GetSliceMin(), GetSliceRange()
   */
  virtual int GetNumberOfSlices()
    { return this->GetSliceMax() - this->GetSliceMin() + 1; }

  /**
   * Return the minimum and maximum slice index values.
   * @param range int array to put the range of slices in.
   * @sa GetSliceMin(), GetSliceMax()
   */
  virtual void GetSliceRange( int range[2] )
    { this->GetSliceRange(range[0], range[1]); }

  /**
   * Return the minimum and maximum slice index values.
   * @param min fill the minimum slice by reference
   * @param max fill the maximum slice by reference
   * @sa GetSliceMin(), GetSliceMax()
   */
  virtual void GetSliceRange( int &min, int &max );

  /**
   * Return the minimum and maximum slice index values.
   * Warning: this method is NOT thread safe.
   * @return pointer to input image's whole extent array
   * @sa GetSliceMin(), GetSliceMax()
   */
  virtual int* GetSliceRange();

  //@{
  /**
   * Set/Get VTK object member ivars.
   * Class member ivars are accessible for connecting to third party
   * GUI toolkits (e.g., Qt), or for custom configuration.
   */
  virtual void SetRenderWindow( vtkRenderWindow* );
  virtual void SetRenderer( vtkRenderer* );
  virtual void SetInteractor( vtkRenderWindowInteractor* );
  virtual void SetInteractorStyle( vtkCustomInteractorStyleImage* );
  vtkGetObjectMacro( RenderWindow, vtkRenderWindow );
  vtkGetObjectMacro( Renderer, vtkRenderer );
  vtkGetObjectMacro( Interactor, vtkRenderWindowInteractor );
  vtkGetObjectMacro( InteractorStyle, vtkCustomInteractorStyleImage );
  vtkSmartPointer<vtkImageSlice> GetImageSlice(){ return this->ImageSlice; }
  vtkSmartPointer<vtkCustomCornerAnnotation> GetAnnotation(){ return this->Annotation; }
  //@}

  //@{
  /**
   * Configure the vtkImageWindowLevel ivar.
   * Configuration is set automatically according to the number
   * of scalar components per pixel/voxel.
   * @see SetInput()
   */
  void SetMappingToLuminance();
  void SetMappingToColorAlpha();
  void SetMappingToColor();
  //@}

  /**
   * Get window.
   * @sa Level, SetColorWindowLevel()
   * @return current window
   */
  double GetColorWindow() { return this->Window; }

  /**
   * Get level.
   * @sa Window, SetColorWindowLevel()
   * @return current level
   */
  double GetColorLevel() { return this->Level; }

  /**
   * Get window and level.
   * @sa  Window, Level, GetColorWindow(), GetColorLevel()
   * @return current window and level by reference
   */
  void SetColorWindowLevel( const double& , const double& );

  //@{
  /**
   * Set the window level of the next image to be the same as the last one.
   * Default is off.
   */
  vtkSetMacro( MaintainLastWindowLevel, int );
  vtkGetMacro( MaintainLastWindowLevel, int );
  vtkBooleanMacro( MaintainLastWindowLevel, int );
  //@}

  /** Record the current camera parameters */
  void RecordCameraView( int specified = -1 );

  //@}
  //@{
  /** Window level control for callbacks and VTK events. */
  void DoStartWindowLevel();
  void DoResetWindowLevel();
  void DoWindowLevel();
  void InvertWindowLevel();
  //@}

  /**
   * Get the dimensionality of the image (e.g., 2D, 3D).
   * @return dimension of the image
   */
  int GetImageDimensionality();

  /** Create a default 3D sinusoidal image. */
  void SetImageToSinusoid();

  /**
   * Scroll once through slices.  Internally, this class uses native VTK
   * vtkAnimationCue, vtkAnimationScene classes and a custom vtkAnimationPlayer
   * class to implement cineloop control.  If necessary, access to these
   * classes could be exposed to assign callbacks to the events they fire:
   * vtkAnimationCue: AnimationCueTickEvent, StartAnimationCueEvent, EndAnimationCueEvent
   * vtkAnimationPlayer: StartEvent, EndEvent,  ProgressEvent
   * Scrolling starts at the current slice and proceeds to the last slice.
   * Stops either when the last slice is reached or if CineStop() is called.
   * If looping is set on via CineLoop(true) the scrolling continues
   * until CineStop() is called.
   */
  void CinePlay();

  /**
   * Scroll continuously through slices.
   * Scrolling starts at the first slice, proceeds to the last slice and
   * then repeats.
   */
  void CineLoop( const bool& );

  /** Set the current slice to the first slice. */
  void CineRewind();

  /** Set the current slice to the last slice. */
  void CineForward();

  /** Set the current slice to the previous slice. */
  void CineStepBackward();

  /** Set the current slice to the next slice. */
  void CineStepForward();

  /**
   * Stop Scrolling through slices.
   */
  void CineStop();

  /**
   * Turn cursoring on or off.  Cursoring works in concert with
   * corner annotation.  If cursoring is off, the cursor widget
   * is disabled but the vtkCustomCornerAnnotation object could still be
   * used to display other textual overlay content.  If annotation
   * is off, cursoring can still be active with its output directed
   * via callback mechanism to another GUI element.
   */
  void SetCursor( const int& );
  vtkBooleanMacro( Cursor, const int& );
  vtkGetMacro( Cursor, int );

  /**
   * Turn annotation on or off in the render window.
   */
  void SetAnnotate( const int& );
  vtkBooleanMacro( Annotate, const int& );
  vtkGetMacro( Annotate, int );

  /**
   * Turn axes marker on or off in the render window.
   */
  void SetAxesDisplay( const int& );
  vtkBooleanMacro( AxesDisplay, const int& );
  vtkGetMacro( AxesDisplay, int );

  /**
   * Turn bounding box and origin axes on or off in the render window.
   */
  void SetBoxDisplay( const int& );
  vtkBooleanMacro( BoxDisplay, const int& );
  vtkGetMacro( BoxDisplay, int );

  /**
   * Turns interpolation on or off for both the cursor widget
   * and the image actor simultaneously.  Off state sets the
   * cursor widget to use discrete cursoring mode: the cursor
   * snaps to pixel centers and reads off discrete pixel values,
   * and the image actor show pixels with nearest neighbor interpolation.
   * On state sets the cursor widget to use continous cursoring
   * mode using interpolation of pixel values and the image actor
   * shows pixels with linear interpolation.
   */
  void SetInterpolate( const int& );
  int  GetInterpolate();
  vtkBooleanMacro( Interpolate, const int& );

  /**
   * Set/Get animation maximum frame rate
   */
  vtkSetClampMacro( MaxFrameRate, int, 0, 60 );
  vtkGetMacro( MaxFrameRate, int );

  /**
   * Set/Get animation frame rate
   */
  void SetFrameRate( const int& );
  vtkGetMacro( FrameRate, int );

  /**
   * Save the current slice view to file
   */
   void WriteSlice( const std::string& );

protected:
  vtkMedicalImageViewer();
  ~vtkMedicalImageViewer();

  //@{
  /**
   * Internal methods to build or dismantle the display pipeline.
   * WARNING: do not use without carefully considering side effects!
   */
  virtual void InstallPipeline();
  virtual void UnInstallPipeline();
  void InstallCursor();
  void UnInstallCursor();
  void InstallAnnotation();
  void UnInstallAnnotation();
  void InstallAxes();
  void UnInstallAxes();
  void InstallBox();
  void UnInstallBox();
  //@}

  //@{
  /** VTK object ivars that constitute the visualization/interaction pipeline. */
  vtkSmartPointer<vtkImageWindowLevel> WindowLevel;
  vtkRenderWindow *RenderWindow;
  vtkRenderer *Renderer;
  vtkSmartPointer<vtkImageSlice> ImageSlice;
  vtkSmartPointer<vtkImageSliceMapper> ImageSliceMapper;
  vtkRenderWindowInteractor *Interactor;
  vtkCustomInteractorStyleImage *InteractorStyle;
  vtkSmartPointer<vtkImageCoordinateWidget> CursorWidget;
  vtkSmartPointer<vtkCustomCornerAnnotation> Annotation;
  vtkSmartPointer<vtkAxesActor> AxesActor;
  vtkSmartPointer<vtkOrientationMarkerWidget> AxesWidget;
  vtkSmartPointer<vtkPropAssembly> BoxActor;
  vtkSmartPointer<vtkAxes> BoxAxes;
  vtkSmartPointer<vtkOutlineCornerSource> BoxOutline;
  vtkSmartPointer<vtkPolyDataMapper> BoxMapper;
  //@}

  int Cursor;
  int Annotate;
  int Interpolate;
  int AxesDisplay;
  int BoxDisplay;

  int Slice;        /**< Current slice index */
  int LastSlice[3]; /**< Keeps track of last slice when changing orientation */

  /** Maintain window level settings between image changes */
  int MaintainLastWindowLevel;
  double OriginalWindow; /**< Original window computed from input */
  double OriginalLevel;  /**< Original level computed from input */
  double InitialWindow;  /**< Initial window at start of interaction */
  double InitialLevel;   /**< Initial level at start of interaction */
  double Window;         /**< Current window */
  double Level;          /**< Current level */

  /**
   * Callback ids for install and uninstall of callbacks to the interactor.
   * Callback tags are set in InstallPipeline() and used for callback
   * removal in UnInstallPipeline()
   */
  std::vector<unsigned long> WindowLevelCallbackTags;

  unsigned long CharCallbackTag;

  /** Calculate the original window and level parameters
    * @sa OriginalWindow, OriginalLevel
    */
  void InitializeWindowLevel();

  /** Maintain view settings between image changes */
  int MaintainLastView;
  int ViewOrientation;              /**< Current view orientation: XY, XZ, YZ */
  double CameraPosition[3][3];      /**< Current camera position */
  double CameraFocalPoint[3][3];    /**< Current camera focal point */
  double CameraViewUp[3][3];        /**< Current camera view up */
  double CameraParallelScale[3];    /**< Current camera parallel scale (zoom) */
  double CameraDistance[3];         /**< Current camera distance */
  double CameraClippingRange[3][2]; /**< Current clipping range */

  /** Set up the default camera parameters based on input data dimensions etc. */
  void InitializeCameraViews();
  /** Update the camera from recorded parameters according to the current view orientation */
  void UpdateCameraView();

  void ComputeCameraFromCurrentSlice( bool useCamera = true );

  /** Animation control */
  vtkSmartPointer<vtkAnimationScene> AnimationScene;
  vtkSmartPointer<vtkAnimationCue> AnimationCue;
  vtkSmartPointer<vtkFrameAnimationPlayer> AnimationPlayer;
  int MaxFrameRate;
  int FrameRate;

private:
  vtkMedicalImageViewer(const vtkMedicalImageViewer&);  /** Not implemented. */
  void operator=(const vtkMedicalImageViewer&);  /** Not implemented. */
};

#endif
