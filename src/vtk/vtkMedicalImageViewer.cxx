/*=========================================================================

  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Module:    vtkMedicalImageViewer.cxx
  Language:  C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <vtkMedicalImageViewer.h>

// Alder includes
#include <Common.h>

// VTK includes
#include <vtkAnimationCue.h>
#include <vtkAnimationScene.h>
#include <vtkAxes.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkCustomCornerAnnotation.h>
#include <vtkCustomInteractorStyleImage.h>
#include <vtkDataArray.h>
#include <vtkFrameAnimationPlayer.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageClip.h>
#include <vtkImageCoordinateWidget.h>
#include <vtkImageData.h>
#include <vtkImageDataReader.h>
#include <vtkImageDataWriter.h>
#include <vtkImagePermute.h>
#include <vtkImageProperty.h>
#include <vtkImageSinusoidSource.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageWindowLevel.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkMedicalImageProperties.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOutlineCornerSource.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPropAssembly.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro( vtkMedicalImageViewer );
vtkCxxSetObjectMacro(vtkMedicalImageViewer, InteractorStyle, vtkCustomInteractorStyleImage);

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class vtkWindowLevelCallback : public vtkCommand
{
public:
  static vtkWindowLevelCallback *New() { return new vtkWindowLevelCallback; }

  void Execute( vtkObject *vtkNotUsed( caller ), unsigned long event,
                void *vtkNotUsed( callData ) )
  {
    if( !this->Viewer ) return;
    switch( event )
    {
      case vtkCommand::ResetWindowLevelEvent:
        this->Viewer->DoResetWindowLevel();
        break;
      case vtkCommand::StartWindowLevelEvent:
        this->Viewer->DoStartWindowLevel();
        break;
      case vtkCommand::WindowLevelEvent:
        this->Viewer->DoWindowLevel();
        break;
      case vtkCommand::EndWindowLevelEvent: break;
    }
  }

  vtkWindowLevelCallback():Viewer( 0 ){}
  ~vtkWindowLevelCallback(){ this->Viewer = 0; }

  vtkMedicalImageViewer* Viewer;
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class vtkCursorWidgetToAnnotationCallback : public vtkCommand
{
public:
  static vtkCursorWidgetToAnnotationCallback *New() {
    return new vtkCursorWidgetToAnnotationCallback; }

  void Execute( vtkObject *caller, unsigned long vtkNotUsed( event ),
                void *vtkNotUsed( callData ) )
  {
    vtkImageCoordinateWidget* self =
      reinterpret_cast< vtkImageCoordinateWidget* >( caller );
    if ( !self || !this->Viewer ) { return; }

    this->Viewer->GetAnnotation()->SetText( 0, self->GetMessageString() );
    this->Viewer->Render();
  }

  vtkCursorWidgetToAnnotationCallback():Viewer( 0 ){}
  ~vtkCursorWidgetToAnnotationCallback(){ this->Viewer = 0; }

  vtkMedicalImageViewer *Viewer;
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class vtkAnimationCueCallback : public vtkCommand
{
  public:
    static vtkAnimationCueCallback *New() {
      return new vtkAnimationCueCallback; }

    void Execute( vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(event),
                  void * vtkNotUsed(callData) )
    {
      if( !this->Viewer ) return;
      this->Viewer->SetSlice( this->Player->GetFrameNo() );
    }

  vtkAnimationCueCallback():Viewer( 0 ), Player( 0 ){}
  ~vtkAnimationCueCallback(){ this->Viewer = 0; this->Player = 0; }
  vtkMedicalImageViewer* Viewer;
  vtkFrameAnimationPlayer* Player;
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class vtkOrientationCharCallback : public vtkCommand {
  public:
    static vtkOrientationCharCallback *New() {
      return new vtkOrientationCharCallback; }

  void Execute( vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(event),
    void* vtkNotUsed(callData) )
  {
    if( !this->Viewer ) return;
    if ( 3 != this->Viewer->GetImageDimensionality() ) return;
    vtkRenderWindowInteractor *rwi = this->Viewer->GetInteractor();
    if( rwi )
    {
      switch (rwi->GetKeyCode())
      {
        case 'x' :
        case 'X' :
          {
            this->Viewer->SetViewOrientationToYZ();
          }
          break;

        case 'y' :
        case 'Y' :
          {
            this->Viewer->SetViewOrientationToXZ();
          }
          break;

        case 'z' :
        case 'Z' :
          {
            this->Viewer->SetViewOrientationToXY();
          }
          break;
      }
    }
  }

  vtkOrientationCharCallback():Viewer( 0 ){}
  ~vtkOrientationCharCallback(){ this->Viewer = 0; }

  vtkMedicalImageViewer *Viewer;
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkMedicalImageViewer::vtkMedicalImageViewer()
{
  this->RenderWindow     = 0;
  this->Renderer         = 0;
  this->ImageSlice       = vtkSmartPointer<vtkImageSlice>::New();
  this->ImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->WindowLevel      = vtkSmartPointer<vtkImageWindowLevel>::New();
  this->Interactor       = 0;
  this->InteractorStyle  = 0;
  this->CursorWidget     = vtkSmartPointer<vtkImageCoordinateWidget>::New();
  this->Annotation       = vtkSmartPointer<vtkCustomCornerAnnotation>::New();
  this->AnimationCue     = vtkSmartPointer<vtkAnimationCue>::New();
  this->AnimationScene   = vtkSmartPointer<vtkAnimationScene>::New();
  this->AnimationPlayer  = vtkSmartPointer<vtkFrameAnimationPlayer>::New();
  this->AxesActor        = vtkSmartPointer<vtkAxesActor>::New();
  this->AxesWidget       = vtkSmartPointer<vtkOrientationMarkerWidget>::New();

  this->AxesWidget->SetOrientationMarker( this->AxesActor );
  this->AxesWidget->KeyPressActivationOff();

  // setting the max font size and linear font scale factor
  // forces vtkCustomCornerAnnotation to keep its constituent text mappers'
  // font sizes the same, otherwise, when the location and value
  // text field dynamically changes width, the font size changes:
  // see RenderOpaqueGeometry in vtkCustomCornerAnnotation.cxx for details
  // TODO: the maximum font size should be set via callback mechanism
  // tied to when the render window changes its size

  this->Annotation->SetMaximumFontSize( 15  );
  this->Annotation->SetLinearFontScaleFactor( 100 );
  this->Annotation->SetMaximumLineHeight( 0.07 );
  this->Annotation->SetText( 2, "<slice_and_max>" );
  this->Annotation->SetText( 3, "<window>\n<level>" );

  this->BoxOutline      = vtkSmartPointer<vtkOutlineCornerSource>::New();
  this->BoxOutline->SetBounds(0,1,0,1,0,1);
  this->BoxOutline->SetCornerFactor(0.05);
  this->BoxOutline->Update();

  vtkSmartPointer<vtkPolyDataMapper>
    boxOutlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  boxOutlineMapper->SetInput( this->BoxOutline->GetOutput() );
  vtkSmartPointer<vtkActor>
    boxOutlineActor = vtkSmartPointer<vtkActor>::New();
  boxOutlineActor->SetMapper( boxOutlineMapper );
  boxOutlineActor->GetProperty()->SetRepresentationToWireframe();
  boxOutlineActor->PickableOff();

  this->BoxAxes = vtkSmartPointer<vtkAxes>::New();
  this->BoxAxes->SymmetricOff();
  this->BoxAxes->SetOrigin(0,0,0);
  this->BoxAxes->ComputeNormalsOff();
  this->BoxAxes->Update();

  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  vtkDataArray* data = this->BoxAxes->GetOutput()->GetPointData()->GetScalars();
  lut->SetTableRange( data->GetRange() );
  lut->SetNumberOfTableValues(3);
  lut->SetTableValue(0,1,0,0);
  lut->SetTableValue(1,0,1,0);
  lut->SetTableValue(2,0,0,1);
  lut->SetRampToLinear();
  lut->Build();

  vtkSmartPointer<vtkPolyDataMapper>
    boxAxesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  boxAxesMapper->SetColorModeToMapScalars();
  boxAxesMapper->UseLookupTableScalarRangeOn();
  boxAxesMapper->SetLookupTable( lut );
  boxAxesMapper->SetInput( this->BoxAxes->GetOutput() );
  vtkSmartPointer<vtkActor>
    boxAxesActor = vtkSmartPointer<vtkActor>::New();
  boxAxesActor->SetMapper( boxAxesMapper );

  boxAxesActor->GetProperty()->SetRepresentationToWireframe();
  boxAxesActor->GetProperty()->SetLineWidth(1.5);
  boxAxesActor->PickableOff();

  this->BoxActor = vtkSmartPointer<vtkPropAssembly>::New();
  this->BoxActor->AddPart( boxAxesActor );
  this->BoxActor->AddPart( boxOutlineActor );

  this->Cursor = 1;
  this->Annotate = 1;
  this->AxesDisplay = 1;
  this->BoxDisplay = 0;

  this->FrameRate = 25;
  this->MaxFrameRate = 60;

  this->AnimationCue->SetStartTime(0.);
  this->AnimationScene->AddCue( this->AnimationCue );
  this->AnimationScene->SetModeToRealTime();
  this->AnimationScene->SetFrameRate( this->FrameRate );

  this->AnimationPlayer->SetAnimationScene( this->AnimationScene );

  vtkSmartPointer<vtkAnimationCueCallback> cbk =
    vtkSmartPointer<vtkAnimationCueCallback>::New();
  cbk->Viewer = this;
  cbk->Player = this->AnimationPlayer;
  this->AnimationCue->AddObserver( vtkCommand::AnimationCueTickEvent, cbk );

  this->MaintainLastWindowLevel = 0;
  this->OriginalWindow = 255.0;
  this->OriginalLevel = 127.5;
  this->Window = 255.0;
  this->Level = 127.5;

  this->Slice = 0;
  this->ViewOrientation = vtkMedicalImageViewer::VIEW_ORIENTATION_XY;

  this->SetMappingToLuminance();

  // loop over slice orientations
  double p[3] = { 1.0, -1.0, 1.0 };
  for( int i = 0; i < 3; ++i )
  {
    for( int j = 0; j < 3; ++j )
    {
      this->CameraPosition[i][j] = ( i == j ? p[j] : 0.0 );
      this->CameraFocalPoint[i][j] = 0.0;
      this->CameraViewUp[i][j] = 0.0;
    }
    this->CameraViewUp[i][( i != 2 ? 2 : 1 )] = 1.0;
    this->CameraParallelScale[i] = 1.0;
    this->CameraDistance[i] = 1.0;
    this->CameraClippingRange[i][0]=0.001;
    this->CameraClippingRange[i][1]=1000.;
    this->LastSlice[i] = 0;
  }

  // Setup the pipeline
  this->SetRenderWindow( vtkSmartPointer< vtkRenderWindow >::New() );
  this->SetRenderer( vtkSmartPointer< vtkRenderer >::New() );

  this->ImageSlice->SetMapper( this->ImageSliceMapper );
  vtkImageProperty* property = this->ImageSlice->GetProperty();
  property->SetInterpolationTypeToLinear();

  this->ImageSliceMapper->SetOrientation(this->ViewOrientation);
  this->ImageSliceMapper->SliceFacesCameraOff();
  this->ImageSliceMapper->SliceAtFocalPointOff();
  this->ImageSliceMapper->BorderOff();
  this->ImageSliceMapper->CroppingOff();

  this->SetInteractorStyle( vtkSmartPointer<vtkCustomInteractorStyleImage>::New() );
  this->InteractorStyle->AutoAdjustCameraClippingRangeOn();
  this->InteractorStyle->SetInteractionModeToImage2D();
  this->InteractorStyle->SetXViewRightVector(0,1,0);
  this->InteractorStyle->SetXViewUpVector(0,0,1);
  this->InteractorStyle->SetYViewRightVector(1,0,0);
  this->InteractorStyle->SetYViewUpVector(0,0,1);
  this->InteractorStyle->SetZViewRightVector(1,0,0);
  this->InteractorStyle->SetZViewUpVector(0,1,0);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkMedicalImageViewer::~vtkMedicalImageViewer()
{
  this->UnInstallPipeline();
  this->SetInteractorStyle(0);

  if( this->Renderer )
  {
    this->Renderer->Delete();
  }

  if( this->RenderWindow )
  {
    this->RenderWindow->Delete();
  }

  if( this->Interactor )
  {
    this->Interactor->Delete();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetInput( vtkImageData* input )
{
  this->UnInstallPipeline();
  if( !input ) return;
  input->Update();

  vtkImageData* lastInput = this->GetInput();
  int initCamera = 1;
  if( lastInput )
  {
    double *lastSpacing = lastInput->GetSpacing();
    double *spacing = input->GetSpacing();
    double *lastOrigin = lastInput->GetOrigin();
    double *origin = input->GetOrigin();
    int *lastExtent = lastInput->GetWholeExtent();
    int *extent = input->GetWholeExtent();
    initCamera = 0;
    for( int i = 0; i < 3; ++i )
    {
      if( lastSpacing[i]    != spacing[i]  ||
          lastOrigin[i]     != origin[i]   ||
          lastExtent[2*i]   != extent[2*i] ||
          lastExtent[2*i+1] != extent[2*i+1] )
      {
        initCamera = 1;
        break;
      }
    }
  }

  this->WindowLevel->SetInput( input );
  this->ImageSliceMapper->SetInput( this->WindowLevel->GetOutput() );

  int components = input->GetNumberOfScalarComponents();
  switch( components )
  {
    case 1: this->SetMappingToLuminance(); break;
    case 2:
    case 3: this->SetMappingToColor(); break;
    case 4: this->SetMappingToColorAlpha(); break;
  }

  this->InstallPipeline();
  this->InitializeWindowLevel();
  if( initCamera )
    this->InitializeCameraViews();
  this->SetSlice( this->GetSliceMin() );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool vtkMedicalImageViewer::Load( const std::string& fileName )
{
  bool success = false;
  if( vtkImageDataReader::IsValidFileName( fileName.c_str() ) )
  {
    vtkNew< vtkImageDataReader > reader;
    reader->SetFileName( fileName.c_str() );
    vtkImageData* image = reader->GetOutput();
    if( image )
    {
      this->SetInput( image );
      success = true;
      vtkMedicalImageProperties* properties = reader->GetMedicalImageProperties();

      // vtkMedicalImageProperties has a bug which crashes if the CineRate is checked for
      // images with no 3rd dimension, so only set the frame rate for 3D images
      if( 3 == this->GetImageDimensionality() )
      {
        if( NULL != properties->GetUserDefinedValue( "CineRate" ) )
        {
          this->SetMaxFrameRate(
            vtkVariant( properties->GetUserDefinedValue( "CineRate" ) ).ToInt() );
          this->SetFrameRate( this->MaxFrameRate );
        }
      }
    }
  }

  return success;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageData* vtkMedicalImageViewer::GetInput()
{
  return vtkImageData::SafeDownCast( this->WindowLevel->GetInput() );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageData* vtkMedicalImageViewer::GetDisplayInput()
{
  return vtkImageData::SafeDownCast( this->ImageSliceMapper->GetInput() );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::WriteSlice( const std::string& fileName )
{
  vtkImageData* input = this->GetInput();
  if( input && vtkImageDataWriter::IsValidFileName( fileName.c_str() ) )
  {
    vtkNew< vtkImageDataWriter > writer;
    writer->SetFileName( fileName.c_str() );
    vtkNew< vtkImageClip > clip;

    int extent[6];
    int u, v, w = this->ViewOrientation;
    switch( w )
    {
      case 0: u = 1; v = 2; break;
      case 1: u = 0; v = 2; break;
      case 2: u = 0; v = 1; break;
    }
    int *w_ext = input->GetWholeExtent();
    extent[2*u] = w_ext[2*u];
    extent[2*u+1] = w_ext[2*u+1];
    extent[2*v] = w_ext[2*v];
    extent[2*v+1] = w_ext[2*v+1];
    extent[2*w] = this->Slice;
    extent[2*w+1] = this->Slice;

    clip->SetOutputWholeExtent( extent );
    clip->ClipDataOn();
    clip->SetInput( input );
    clip->Update();

    int ec[6];
    clip->GetOutput()->GetWholeExtent(ec);

    int et[] = {0,0,0};     // extent translation
    int op[] = {0,1,2};     // output axes permutation

    // translate to an x-y plane:
    // 1) change extents via vtkImageChangeInformation
    // 2) permute the axes
    switch ( w )
    {
      case 0: et[0] = -ec[0];
              op[0] = 2; op[1] = 0; op[2] = 1;
              break;  //YZ
      case 1: et[1] = -ec[2];
              op[0] = 0; op[1] = 2; op[2] = 1;
              break;  //XZ
      case 2: et[2] = -ec[4];
              break;  //XY
    }

    vtkNew< vtkImageChangeInformation > change;
    change->SetExtentTranslation( et );
    change->SetInput( clip->GetOutput() );

    vtkNew< vtkImagePermute > permute;
    permute->SetFilteredAxes( op );
    permute->SetInput( change->GetOutput() );

    writer->SetInput( permute->GetOutput() );
    writer->Write();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetMappingToLuminance()
{
  this->WindowLevel->SetActiveComponent( 0 );
  this->WindowLevel->PassAlphaToOutputOff();
  this->WindowLevel->SetOutputFormatToLuminance();
  this->WindowLevel->Modified();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetMappingToColor()
{
  this->WindowLevel->SetOutputFormatToRGB();
  this->WindowLevel->PassAlphaToOutputOff();
  this->WindowLevel->Modified();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetMappingToColorAlpha()
{
  this->WindowLevel->SetOutputFormatToRGBA();
  this->WindowLevel->PassAlphaToOutputOn();
  this->WindowLevel->Modified();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetInteractor( vtkRenderWindowInteractor *arg )
{
  if( this->Interactor == arg ) return;

  this->UnInstallPipeline();

  if( this->Interactor )
    this->Interactor->UnRegister( this );

  this->Interactor = arg;

  if( this->Interactor )
    this->Interactor->Register( this );

  this->InstallPipeline();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetRenderWindow( vtkRenderWindow *arg )
{
  if( this->RenderWindow == arg ) return;

  this->UnInstallPipeline();

  if( this->RenderWindow )
    this->RenderWindow->UnRegister( this );

  this->RenderWindow = arg;

  if( this->RenderWindow )
    this->RenderWindow->Register( this );

  this->InstallPipeline();

  if( 0 == this->Interactor && this->RenderWindow )
  {
    this->SetInteractor( this->RenderWindow->GetInteractor() );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetRenderer( vtkRenderer *arg )
{
  if( this->Renderer == arg ) return;

  this->UnInstallPipeline();

  if( this->Renderer )
    this->Renderer->UnRegister( this );

 this->Renderer = arg;

  if( this->Renderer )
    this->Renderer->Register( this );

  this->InstallPipeline();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::InstallPipeline()
{
  // setup the render window
  if( this->RenderWindow && this->Renderer )
    this->RenderWindow->AddRenderer( this->Renderer );

  // setup the interactor
  if( this->Interactor  )
  {
    // create an interactor style if we don't already have one
    if( !this->InteractorStyle )
    {
      this->SetInteractorStyle( vtkSmartPointer<vtkCustomInteractorStyleImage>::New() );
    }

    this->Interactor->SetInteractorStyle( this->InteractorStyle );
    if( 3 == this->GetImageDimensionality() )
      this->InteractorStyle->SetInteractionModeToImage3D();
    else
      this->InteractorStyle->SetInteractionModeToImage2D();

    if( this->RenderWindow )
      this->Interactor->SetRenderWindow( this->RenderWindow );
  }

  if( this->Interactor && this->InteractorStyle )
  {
    vtkSmartPointer< vtkWindowLevelCallback > cbk = vtkSmartPointer< vtkWindowLevelCallback >::New();
    cbk->Viewer = this;

    this->WindowLevelCallbackTags.push_back(
      this->InteractorStyle->AddObserver( vtkCommand::StartWindowLevelEvent, cbk ) );
    this->WindowLevelCallbackTags.push_back(
      this->InteractorStyle->AddObserver( vtkCommand::WindowLevelEvent, cbk ) );
    this->WindowLevelCallbackTags.push_back(
      this->InteractorStyle->AddObserver( vtkCommand::EndWindowLevelEvent, cbk ) );
    this->WindowLevelCallbackTags.push_back(
      this->InteractorStyle->AddObserver( vtkCommand::ResetWindowLevelEvent, cbk ) );

    vtkSmartPointer< vtkOrientationCharCallback > charCbk = vtkSmartPointer< vtkOrientationCharCallback >::New();
    charCbk->Viewer = this;
    this->CharCallbackTag =
      this->Interactor->AddObserver( vtkCommand::CharEvent, charCbk );
  }

  if( this->Renderer )
  {
    this->Renderer->GetActiveCamera()->ParallelProjectionOn();
    this->Renderer->AddViewProp( this->ImageSlice );
  }

  this->InstallAnnotation();
  this->InstallCursor();
  this->InstallAxes();
  this->InstallBox();

  if( this->Renderer && this->GetInput() )
  {
    this->AnimationCue->SetEndTime(
      this->GetNumberOfSlices() / this->AnimationScene->GetFrameRate() );
    this->AnimationScene->SetStartTime( 0. );
    this->AnimationScene->SetEndTime( this->AnimationCue->GetEndTime());
    this->AnimationPlayer->SetNumberOfFrames( this->GetNumberOfSlices() );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::UnInstallPipeline()
{
  this->UnInstallCursor();
  this->UnInstallAnnotation();
  this->UnInstallAxes();
  this->UnInstallBox();

  if( this->InteractorStyle && !this->WindowLevelCallbackTags.empty() )
  {
    std::vector<unsigned long>::iterator it;

    for( it = this->WindowLevelCallbackTags.begin();
         it != this->WindowLevelCallbackTags.end(); it++ )
    {
      this->InteractorStyle->RemoveObserver( ( *it ) );
    }
  }

  if( this->RenderWindow && this->Renderer )
    this->RenderWindow->RemoveRenderer( this->Renderer );

  if( this->Renderer )
    this->Renderer->RemoveViewProp( this->ImageSlice );

  if( this->Interactor )
  {
    this->Interactor->RemoveObserver( this->CharCallbackTag );
    this->Interactor->SetInteractorStyle( 0 );
    this->Interactor->SetRenderWindow( 0 );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::InitializeWindowLevel()
{
  vtkImageData* input = this->GetInput();
  if( !input ) return;

  input->UpdateInformation();
  input->Update();

  int components = input->GetNumberOfScalarComponents();

  double dataMin = input->GetScalarTypeMax();
  double dataMax = input->GetScalarTypeMin();

  if( 1 == components )
  {
    dataMin = input->GetScalarRange()[0];
    dataMax = input->GetScalarRange()[1];
  }
  else
  {
    vtkDataArray* data = 0;
    if( input->GetPointData() && 0 != (data = input->GetPointData()->GetScalars()) )
    {
      for( int i = 0; i < components; ++i )
      {
        double min = data->GetRange(i)[0];
        double max = data->GetRange(i)[1];
        if( dataMin > min ) dataMin = min;
        if( dataMax < max ) dataMax = max;
      }
    }
  }

  if( this->MaintainLastWindowLevel )
  {
    this->OriginalWindow = this->Window;
    this->OriginalLevel =  this->Level;
  }
  else
  {
    this->OriginalWindow = dataMax - dataMin;
    this->OriginalLevel =  0.5 * ( dataMin + dataMax );
  }

  double tol = 0.001;
  if( tol > fabs( this->OriginalWindow ) )
  {
    this->OriginalWindow = this->OriginalWindow < 0.0 ? -tol : tol;
  }

  if( tol > fabs( this->OriginalLevel ) )
  {
    this->OriginalLevel = this->OriginalLevel < 0.0 ? -tol : tol;
  }

  // VTK_LUMINANCE is defined in vtkSystemIncludes.h
  if( VTK_LUMINANCE != this->WindowLevel->GetOutputFormat() )
  {
    this->OriginalWindow = 255;
    this->OriginalLevel = 127.5;
  }

  this->SetColorWindowLevel( this->OriginalWindow, this->OriginalLevel );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::InitializeCameraViews()
{
  vtkImageData* input = this->GetInput();
  input->UpdateInformation();
  double* origin = input->GetOrigin();
  double* spacing = input->GetSpacing();
  int* extent = input->GetWholeExtent();
  int u, v;
  double fpt[3];
  double pos[3];
  bool is3D = 3 == this->GetImageDimensionality();

  for( int w = 0; w < 3; ++w )
  {
    double vup[3] = { 0.0, 0.0, 0.0 };
    double vpn[3] = { 0.0, 0.0, 0.0 };
    switch( w )
    {
      case 0: u = 1; v = 2; break; // YZ
      case 1: u = 0; v = 2; break; // XZ
      case 2: u = 0; v = 1; break; // XY
    }
    vup[v] = 1.0;
    vpn[w] = 1 == w ? -1.0 : 1.0;

    // compute the bounds for the first slice of each orientation
    double bounds[6];
    bounds[2*u]   = origin[u] + spacing[u] * extent[2*u];
    bounds[2*u+1] = origin[u] + spacing[u] * extent[2*u+1];
    bounds[2*v]   = origin[v] + spacing[v] * extent[2*v];
    bounds[2*v+1] = origin[v] + spacing[v] * extent[2*v+1];
    bounds[2*w]   = origin[w] + spacing[w] * extent[2*w];
    bounds[2*w+1] = bounds[2*w];

    fpt[u] = pos[u] = origin[u] + 0.5 * spacing[u] * ( extent[2*u] + extent[2*u+1] );
    fpt[v] = pos[v] = origin[v] + 0.5 * spacing[v] * ( extent[2*v] + extent[2*v+1] );
    fpt[w] = origin[w] + spacing[w] * ( 1 == w ? extent[2*w+1] : extent[2*w] );
    if( is3D )
      pos[w] = fpt[w] + vpn[w] * spacing[w] * ( extent[2*w+1] - extent[2*w] );
    else
      pos[w] = fpt[w] + vpn[w] * spacing[w];

    vtkCamera * camera = this->Renderer->GetActiveCamera();
    camera->SetFocalPoint( fpt );
    camera->SetPosition( pos );
    camera->SetViewUp( vup );

    this->Renderer->ResetCamera( bounds );
    this->RecordCameraView( w );

    this->LastSlice[w] = extent[2*w];
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::RecordCameraView( int specified )
{
  vtkCamera *camera = this->Renderer->GetActiveCamera();

  if( camera )
  {
    double pos[3];
    double fpt[3];
    double v[3];
    camera->GetPosition( pos );
    camera->GetFocalPoint( fpt );
    camera->GetViewUp( v );
    int w = (-1 == specified) ? this->ViewOrientation : specified;
    for( int i = 0; i < 3; ++i )
    {
      this->CameraPosition[w][i]   = pos[i];
      this->CameraFocalPoint[w][i] = fpt[i];
      this->CameraViewUp[w][i]     = v[i];
    }
    this->CameraParallelScale[w] = camera->GetParallelScale();
    this->CameraDistance[w] = camera->GetDistance();
    this->CameraClippingRange[w][0] = camera->GetClippingRange()[0];
    this->CameraClippingRange[w][1] = camera->GetClippingRange()[1];
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::UpdateCameraView()
{
  vtkCamera *camera = this->Renderer->GetActiveCamera();

  if( camera )
  {
    double pos[3];
    double fpt[3];
    double v[3];
    for( int i = 0; i < 3; ++i )
    {
      pos[i] = this->CameraPosition[this->ViewOrientation][i];
      fpt[i] = this->CameraFocalPoint[this->ViewOrientation][i];
      v[i]   = this->CameraViewUp[this->ViewOrientation][i];
    }
    camera->SetPosition( pos );
    camera->SetFocalPoint( fpt );
    camera->SetViewUp( v );
    camera->SetParallelScale( this->CameraParallelScale[this->ViewOrientation] );
    camera->SetClippingRange( this->CameraClippingRange[this->ViewOrientation] );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkMedicalImageViewer::GetImageDimensionality()
{
  int dim = 0;
  if( this->GetInput() )
  {
    int* dims = this->GetInput()->GetDimensions();
    dim = 1 >= dims[2] ? 2 : 3;
  }
  return dim;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetImageToSinusoid()
{
  // Create the sinusoid default image like MicroView does
  vtkNew< vtkImageSinusoidSource > sinusoid;
  sinusoid->SetPeriod( 32 );
  sinusoid->SetPhase( 0 );
  sinusoid->SetAmplitude( 255 );
  sinusoid->SetWholeExtent( 0, 127, 0, 127, 0, 31 );
  sinusoid->SetDirection( 0.5, -0.5, 1.0 / sqrt( 2.0 ) );
  sinusoid->GetOutput()->UpdateInformation();
  sinusoid->GetOutput()->Update();

  this->SetInput( sinusoid->GetOutput() );
  this->SetSlice( 15 );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::GetSliceRange( int &min, int &max )
{
  vtkImageData *input = this->GetInput();
  if( input )
  {
    input->UpdateInformation();
    int *w_ext = input->GetWholeExtent();
    min = w_ext[this->ViewOrientation * 2];
    max = w_ext[this->ViewOrientation * 2 + 1];
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int* vtkMedicalImageViewer::GetSliceRange()
{
  vtkImageData *input = this->GetInput();
  if( input )
  {
    input->UpdateInformation();
    return input->GetWholeExtent() + this->ViewOrientation * 2;
  }
  return 0;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkMedicalImageViewer::GetSliceMin()
{
  int *range = this->GetSliceRange();
  if( range )
  {
    return range[0];
  }
  return 0;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkMedicalImageViewer::GetSliceMax()
{
  int *range = this->GetSliceRange();
  if( range )
  {
    return range[1];
  }
  return 0;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::ComputeCameraFromCurrentSlice( bool useCamera )
{
  vtkImageData* image = this->GetInput();
  if( !image ) return;

  vtkCamera *camera = this->Renderer->GetActiveCamera();
  if( camera )
  {
    int u, v, w = this->ViewOrientation;
    switch( w )
    {
      case 0: u = 1; v = 2; break;
      case 1: u = 0; v = 2; break;
      case 2: u = 0; v = 1; break;
    }

    double* origin = image->GetOrigin();
    double* spacing = image->GetSpacing();
    int* extent = image->GetWholeExtent();
    double fpt[3];
    fpt[u] = origin[u] + 0.5 * spacing[u] * ( extent[2*u] + extent[2*u+1] );
    fpt[v] = origin[v] + 0.5 * spacing[v] * ( extent[2*v] + extent[2*v+1] );
    fpt[w] = origin[w] + spacing[w] * this->Slice;
    if( useCamera )
    {
      double* vpn = camera->GetViewPlaneNormal();
      double d = camera->GetDistance();
      for( int i = 0; i < 3; ++i )
      {
        this->CameraFocalPoint[w][i] = fpt[i];
        this->CameraPosition[w][i] = fpt[i] + d * vpn[i];
      }
      this->CameraDistance[w] = d;
    }
    else
    {
      double pos[3];
      double vpn[3] = { 0.0,0.0,0.0 };
      vpn[w] = 1 == w ? -1.0 : 1.0;
      pos[u] = fpt[u];
      pos[v] = fpt[v];
      pos[w] = fpt[w] + this->CameraDistance[w] * vpn[w];
      for( int i = 0; i < 3; ++i )
      {
        this->CameraFocalPoint[w][i] = fpt[i];
        this->CameraPosition[w][i] = pos[i];
      }
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetSlice( int slice )
{
  int *range = this->GetSliceRange();
  if( range )
  {
    if( slice < range[0] )
    {
      slice = range[0];
    }
    else if( slice > range[1] )
    {
      slice = range[1];
    }
  }

  this->RecordCameraView();
  this->LastSlice[this->ViewOrientation] = this->Slice;
  this->Slice = slice;

  this->ImageSliceMapper->SetSliceNumber( this->Slice );
  this->ImageSliceMapper->Update();

  this->ComputeCameraFromCurrentSlice();
  this->UpdateCameraView();
  this->Render();

  if( this->Cursor && this->Annotate )
  {
    this->CursorWidget->UpdateMessageString();
    this->Annotation->SetText( 0, this->CursorWidget->GetMessageString() );
    this->Annotation->Modified();
    this->Render();
  }

  this->InvokeEvent( Alder::Common::SliceChangedEvent );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetViewOrientation( const int& orientation )
{
  if( orientation < vtkMedicalImageViewer::VIEW_ORIENTATION_YZ ||
      orientation > vtkMedicalImageViewer::VIEW_ORIENTATION_XY )
  {
    return;
  }

  if( orientation == this->ViewOrientation ) return;

  this->RecordCameraView();
  this->LastSlice[this->ViewOrientation] = this->Slice;
  this->ViewOrientation = orientation;
  this->Slice = this->LastSlice[this->ViewOrientation];

  this->ImageSliceMapper->SetOrientation( orientation );
  this->ImageSliceMapper->SetSliceNumber( this->Slice );
  this->ImageSliceMapper->Update();

  this->ComputeCameraFromCurrentSlice( false );
  this->UpdateCameraView();
  this->Render();

  if( this->Cursor && this->Annotate )
  {
    this->CursorWidget->UpdateMessageString();
    this->Annotation->SetText( 0, this->CursorWidget->GetMessageString() );
    this->Annotation->Modified();
    this->Render();
  }

  this->InvokeEvent( Alder::Common::OrientationChangedEvent );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::Render()
{
  this->RenderWindow->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double vtkMedicalImageViewer::GetSliceLocation()
{
  return this->ImageSliceMapper->GetBounds()[2*this->ViewOrientation];
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetColorWindowLevel( const double& w, const double& l )
{
  if( this->Window == w && this->Level == l ) return;
  this->Window = w;
  this->Level = l;

  this->WindowLevel->SetWindow( this->Window );
  this->WindowLevel->SetLevel( this->Level );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::InvertWindowLevel()
{
  this->SetColorWindowLevel( -1.0*this->Window, this->Level );
  this->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::DoStartWindowLevel()
{
  this->InitialWindow = this->Window;
  this->InitialLevel = this->Level;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::DoResetWindowLevel()
{
  this->SetColorWindowLevel( this->OriginalWindow, this->OriginalLevel );
  this->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::DoWindowLevel()
{
  if( !this->InteractorStyle ) return;

  int *size = this->RenderWindow->GetSize();
  double window = this->InitialWindow;
  double level = this->InitialLevel;

  // Compute normalized delta

  double dx = 4.0 *
    ( this->InteractorStyle->GetWindowLevelCurrentPosition()[0] -
      this->InteractorStyle->GetWindowLevelStartPosition()[0] ) / size[0];
  double dy = 4.0 *
    ( this->InteractorStyle->GetWindowLevelStartPosition()[1] -
      this->InteractorStyle->GetWindowLevelCurrentPosition()[1] ) / size[1];

  // Scale by current values

  if( fabs( window ) > 0.01 )
  {
    dx = dx * window;
  }
  else
  {
    dx = dx * ( window < 0 ? -0.01 : 0.01 );
  }
  if( fabs( level ) > 0.01 )
  {
    dy = dy * level;
  }
  else
  {
    dy = dy * ( level < 0 ? -0.01 : 0.01 );
  }

  // Abs so that direction does not flip

  if( window < 0.0 )
  {
    dx = -1 * dx;
  }
  if( level < 0.0 )
  {
    dy = -1 * dy;
  }

  // Compute new window level

  double newWindow = dx + window;
  double newLevel;

  newLevel = level - dy;

  if( fabs( newWindow ) < 0.01 )
  {
    newWindow = 0.01 * ( newWindow < 0 ? -1 : 1 );
  }
  if( fabs( newLevel ) < 0.01 )
  {
    newLevel = 0.01 * ( newLevel < 0 ? -1 : 1 );
  }

  this->SetColorWindowLevel( newWindow, newLevel );
  this->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::CineLoop( const bool& loop )
{
  bool inplay = this->AnimationPlayer->IsInPlay();
  if( inplay ) this->AnimationPlayer->Stop();
  this->AnimationPlayer->SetLoop(loop);
  if( inplay ) this->AnimationPlayer->Play();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::CineStop()
{
  this->AnimationPlayer->Stop();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::CinePlay()
{
  this->AnimationPlayer->Play();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::CineRewind()
{
  this->AnimationPlayer->GoToFirst();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::CineForward()
{
  this->AnimationPlayer->GoToLast();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::CineStepBackward()
{
  this->AnimationPlayer->GoToPrevious();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::CineStepForward()
{
  this->AnimationPlayer->GoToNext();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::InstallAnnotation()
{
  if( !this->Renderer || !this->Annotation ) return;

  this->Annotation->SetImageSlice( this->ImageSlice );
  this->Annotation->SetWindowLevel( this->WindowLevel );
  this->Annotation->SetVisibility( this->Annotate );
  this->Renderer->AddViewProp( this->Annotation );

  this->Annotation->SetText( 2, "" );
  if( this->GetInput() && this->GetImageDimensionality() > 2 )
  {
    this->Annotation->SetText( 2, "<slice_and_max>" );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::UnInstallAnnotation()
{
  if( !this->Renderer || !this->Annotation ) return;

  this->Annotation->VisibilityOff();
  this->Renderer->RemoveViewProp( this->Annotation );
  this->Annotation->SetImageSlice( 0 );
  this->Annotation->SetWindowLevel( 0 );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::InstallCursor()
{
  if( this->Interactor && this->Renderer && this->GetInput() && this->Cursor )
  {
    this->CursorWidget->SetDefaultRenderer( this->Renderer );
    this->CursorWidget->SetInteractor( this->Interactor );
    this->CursorWidget->SetInput( this->GetInput() );
    this->CursorWidget->AddViewProp( this->ImageSlice );

    if( this->Annotate )
    {
      vtkSmartPointer< vtkCursorWidgetToAnnotationCallback > cbk =
        vtkSmartPointer< vtkCursorWidgetToAnnotationCallback >::New();
      cbk->Viewer = this;
      this->CursorWidget->AddObserver( vtkCommand::InteractionEvent, cbk );
    }
    this->CursorWidget->On();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::UnInstallCursor()
{
  if( this->CursorWidget->GetEnabled() )
  {
    this->CursorWidget->Off();
  }
  this->CursorWidget->RemoveAllProps( );
  this->CursorWidget->SetInput( 0 );
  this->CursorWidget->RemoveObservers( vtkCommand::InteractionEvent );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::InstallAxes()
{
  if( this->Interactor && this->Renderer && this->GetInput() && this->AxesDisplay )
  {
    this->AxesWidget->SetDefaultRenderer( this->Renderer );
    this->AxesWidget->SetInteractor( this->Interactor );
    this->AxesWidget->SetViewport( 0.8, 0.0, 1.0, 0.2 );
    this->AxesWidget->On();
    this->AxesWidget->InteractiveOff();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::UnInstallAxes()
{
  if( this->AxesWidget->GetEnabled() )
  {
    this->AxesWidget->Off();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::InstallBox()
{
  if( this->Interactor && this->Renderer && this->GetInput() &&
      3 == this->GetImageDimensionality() && this->BoxDisplay )
  {
    this->Renderer->AddViewProp( this->BoxActor );
    vtkImageData* input = this->GetInput();
    this->BoxAxes->SetOrigin( input->GetOrigin() );
    double* bounds = input->GetBounds();
    double scale = VTK_FLOAT_MAX;
    for( int i = 0; i < 3; ++i )
    {
      double r = bounds[2*i+1]-bounds[2*i];
      if( r < scale )
        scale = r;
    }
    this->BoxAxes->SetScaleFactor( scale );
    this->BoxOutline->SetBounds( input->GetBounds() );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::UnInstallBox()
{
  if( this->Renderer )
  {
    this->Renderer->RemoveViewProp( this->BoxActor );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetCursor( const int& arg )
{
  if( this->Cursor == arg )
  {
    return;
  }
  this->Cursor = arg;

  if( this->Cursor )
    this->InstallCursor();
  else
    this->UnInstallCursor();

  if( this->GetInput() ) this->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetInterpolate( const int& i)
{
  vtkImageProperty* property = this->ImageSlice->GetProperty();
  if( property )
  {
    if( i )
    {
      if( property->GetInterpolationType() != VTK_LINEAR_INTERPOLATION )
      {
        property->SetInterpolationTypeToLinear();
        this->ImageSlice->Modified();
        this->CursorWidget->SetCursoringMode( vtkImageCoordinateWidget::Continuous );
      }
    }
    else
    {
      if( property->GetInterpolationType() != VTK_NEAREST_INTERPOLATION )
      {
        property->SetInterpolationTypeToNearest();
        this->ImageSlice->Modified();
        this->CursorWidget->SetCursoringMode( vtkImageCoordinateWidget::Discrete );
      }
    }
  }
  if( this->GetInput() ) this->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkMedicalImageViewer::GetInterpolate()
{
  vtkImageProperty* property = this->ImageSlice->GetProperty();
  if( property &&
      property->GetInterpolationType() != VTK_NEAREST_INTERPOLATION )
  {
    return 1;
  }
  return 0;
}


//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetAnnotate( const int& arg )
{
  this->Annotate = arg;

  if( this->Annotate )
    this->InstallAnnotation();
  else
    this->UnInstallAnnotation();

  if( this->GetInput() ) this->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetAxesDisplay( const int& arg )
{
  this->AxesDisplay = arg;

  if( this->AxesDisplay )
    this->InstallAxes();
  else
    this->UnInstallAxes();

  if( this->GetInput() ) this->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetBoxDisplay( const int& arg )
{
  this->BoxDisplay = arg;

  if( this->BoxDisplay )
    this->InstallBox();
  else
    this->UnInstallBox();

  if( this->GetInput() ) this->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetFrameRate( const int& arg )
{
  if( this->FrameRate == arg ) return;
  if( arg > this->MaxFrameRate || arg < 0 ) return;
  this->FrameRate = arg;
  this->AnimationScene->SetFrameRate( this->FrameRate );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::RotateCamera( const double& angle )
{
  if( this->Renderer )
  {
    this->Renderer->GetActiveCamera()->Roll( -angle );
    this->Render();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::FlipCameraHorizontal()
{
  if( this->Renderer )
  {
    this->RecordCameraView();
    for( int i = 0; i < 3; i++ )
    {
      this->CameraPosition[this->ViewOrientation][i] =
        2.0*this->CameraFocalPoint[this->ViewOrientation][i] -
        this->CameraPosition[this->ViewOrientation][i];
    }
    this->Renderer->GetActiveCamera()->SetPosition(
      this->CameraPosition[this->ViewOrientation] );
    this->Renderer->ResetCameraClippingRange();
    this->Render();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::FlipCameraVertical()
{
  if( this->Renderer )
  {
    this->RecordCameraView();
    for( int i = 0; i < 3; i++ )
    {
      this->CameraViewUp[this->ViewOrientation][i] =
        -this->CameraViewUp[this->ViewOrientation][i];
      this->CameraPosition[this->ViewOrientation][i] =
        2.0*this->CameraFocalPoint[this->ViewOrientation][i] -
        this->CameraPosition[this->ViewOrientation][i];
    }
    this->Renderer->GetActiveCamera()->SetPosition(
      this->CameraPosition[this->ViewOrientation] );
    this->Renderer->GetActiveCamera()->SetViewUp(
      this->CameraViewUp[this->ViewOrientation] );
    this->Renderer->ResetCameraClippingRange();
    this->Render();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );

  os << indent << "RenderWindow:\n";
  if( this->RenderWindow )
  {
    this->RenderWindow->PrintSelf( os, indent.GetNextIndent() );
  }
  else
  {
    os << "None";
  }

  os << indent << "Renderer:\n";
  if( this->Renderer )
  {
    this->Renderer->PrintSelf( os, indent.GetNextIndent() );
  }
  else
  {
    os << "None";
  }

  os << indent << "InteractorStyle: " << endl;
  if( this->InteractorStyle )
  {
    os << "\n";
    this->InteractorStyle->PrintSelf( os, indent.GetNextIndent() );
  }
  else
  {
    os << "None";
  }

  os << indent << "Interactor: " << endl;
  if( this->Interactor )
  {
    os << "\n";
    this->Interactor->PrintSelf( os, indent.GetNextIndent() );
  }
  else
  {
    os << "None";
  }

  os << indent << "ImageSlice:\n";
  this->ImageSlice->PrintSelf( os, indent.GetNextIndent() );

  os << indent << "WindowLevel:\n" << endl;
  this->WindowLevel->PrintSelf( os, indent.GetNextIndent() );

  os << indent << "Slice: " << this->Slice << endl;
  os << indent << "ViewOrientation: " << this->ViewOrientation << endl;
  os << indent << "MaintainLastWindowLevel: " << this->MaintainLastWindowLevel << endl;
  os << indent << "Annotate: " << this->Annotate << endl;
  os << indent << "Cursor: " << this->Cursor << endl;
  os << indent << "Interpolate: " << this->Interpolate << endl;
  os << indent << "MaxFrameRate: " << this->MaxFrameRate << endl;
  os << indent << "FrameRate: " << this->FrameRate << endl;
}
