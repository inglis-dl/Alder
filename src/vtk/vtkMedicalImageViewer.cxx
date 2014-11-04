/*=========================================================================

  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Module:    vtkMedicalImageViewer.cxx
  Language:  C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <vtkMedicalImageViewer.h>

#include <vtkAnimationCue.h>
#include <vtkAnimationScene.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkCustomCornerAnnotation.h>
#include <vtkDataArray.h>
#include <vtkFrameAnimationPlayer.h>
#include <vtkImageActor.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageClip.h>
#include <vtkImageCoordinateWidget.h>
#include <vtkImageData.h>
#include <vtkImageDataReader.h>
#include <vtkImageDataWriter.h>
#include <vtkImagePermute.h>
#include <vtkImageSinusoidSource.h>
#include <vtkImageWindowLevel.h>
#include <vtkCustomInteractorStyleImage.h>
#include <vtkMedicalImageProperties.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
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
      this->Viewer->SetSlice( this->Player->GetFrameNo() );
    }

  vtkAnimationCueCallback():Viewer( 0 ), Player( 0 ){}
  ~vtkAnimationCueCallback(){ this->Viewer = 0; this->Player = 0; } 
  vtkMedicalImageViewer* Viewer;
  vtkFrameAnimationPlayer* Player;
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkMedicalImageViewer::vtkMedicalImageViewer()
{
  this->RenderWindow    = 0;
  this->Renderer        = 0;
  this->ImageActor      = vtkSmartPointer<vtkImageActor>::New();
  this->WindowLevel     = vtkSmartPointer<vtkImageWindowLevel>::New();
  this->Interactor      = 0;
  this->InteractorStyle = 0;
  this->CursorWidget    = vtkSmartPointer<vtkImageCoordinateWidget>::New();
  this->Annotation      = vtkSmartPointer<vtkCustomCornerAnnotation>::New();
  this->AnimationCue    = vtkSmartPointer<vtkAnimationCue>::New();
  this->AnimationScene  = vtkSmartPointer<vtkAnimationScene>::New();
  this->AnimationPlayer = vtkSmartPointer<vtkFrameAnimationPlayer>::New();

  this->Annotation->SetMaximumLineHeight( 0.07 );
  this->Annotation->SetText( 2, "<slice_and_max>" );
  this->Annotation->SetText( 3, "<window>\n<level>" );
  // setting the max font size and linear font scale factor
  // forces vtkCustomCornerAnnotation to keep its constituent text mappers'
  // font sizes the same, otherwise, when the location and value
  // text field dynamically changes width, the font size changes:
  // see RenderOpaqueGeometry in vtkCustomCornerAnnotation.cxx for details
  // TODO: the maximum font size should be set via callback mechanism
  // tied to when the render window changes its size

  this->Annotation->SetMaximumFontSize( 15  );
  this->Annotation->SetLinearFontScaleFactor( 100 );

  this->Cursor = 1;
  this->Annotate = 1;
  this->Interpolate = 0;

  this->MaxFrameRate = 60;
  this->FrameRate = 25;

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
    this->CameraParallelScale[i] = VTK_FLOAT_MIN;
    this->LastSlice[i] = 0;
  }

  // Setup the pipeline
  this->SetRenderWindow( vtkSmartPointer< vtkRenderWindow >::New() );
  this->SetRenderer( vtkSmartPointer< vtkRenderer >::New() );
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
  
  this->WindowLevel->SetInputConnection( input->GetProducerPort() );
  this->ImageActor->SetInput( this->WindowLevel->GetOutput() );

  input->Update();
  int components = input->GetNumberOfScalarComponents();
  switch( components )
  {    
    case 1: this->SetMappingToLuminance(); break;
    case 2:
    case 3: this->SetMappingToColor(); break;
    case 4: this->SetMappingToColorAlpha(); break;
  }    

  this->InitializeWindowLevel();
  this->InitializeCameraViews();

  this->InstallPipeline();
  this->UpdateDisplayExtent();
  this->Render(); 
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
      if( this->GetImageDimensionality() == 3 )
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
void vtkMedicalImageViewer::WriteSlice( const std::string& fileName )
{
  vtkImageData* image = this->GetInput();
  if( image && vtkImageDataWriter::IsValidFileName( fileName.c_str() ) )
  {
    vtkNew< vtkImageDataWriter > writer;
    writer->SetFileName( fileName.c_str() );
    vtkNew< vtkImageClip > clip;
    clip->SetOutputWholeExtent( this->ImageActor->GetDisplayExtent() );
    clip->ClipDataOn();
    clip->SetInputConnection( image->GetProducerPort() );
    clip->Update();

    int ec[6];
    clip->GetOutput()->GetWholeExtent(ec);

    int axis = this->GetViewOrientation();
    int et[] = {0,0,0};     // extent translation
    int op[] = {0,1,2};     // output axes permutation

    // have to translate to a purely x-y plane:
    // 1) change extents by vtkImageChangeInformation
    // 2) permute the axes
    switch ( axis )
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
    change->SetExtentTranslation(et);
    change->SetInputConnection(clip->GetOutputPort());

    vtkNew< vtkImagePermute > permute;
    permute->SetFilteredAxes(op);
    permute->SetInputConnection(change->GetOutputPort());

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

  if( this->Interactor == 0 && this->RenderWindow )
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

    this->InteractorStyle->AutoAdjustCameraClippingRangeOn();     
    this->Interactor->SetInteractorStyle( this->InteractorStyle );

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
  }

  if( this->Renderer )
  {
    this->Renderer->GetActiveCamera()->ParallelProjectionOn();
    this->Renderer->AddViewProp( this->ImageActor );
  }
  
  this->InstallAnnotation();
  this->InstallCursor();

  if( this->Renderer && this->GetInput() )
  {
    this->AnimationCue->SetEndTime( 
      this->GetNumberOfSlices() / this->AnimationScene->GetFrameRate() );
    this->AnimationScene->SetStartTime(0.);
    this->AnimationScene->SetEndTime( this->AnimationCue->GetEndTime());
    this->AnimationPlayer->SetNumberOfFrames( this->GetNumberOfSlices() );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::UnInstallPipeline()
{
  this->UnInstallCursor();
  this->UnInstallAnnotation();

  if( this->InteractorStyle && !this->WindowLevelCallbackTags.empty() )
  {
    for( auto it = this->WindowLevelCallbackTags.begin(); 
              it != this->WindowLevelCallbackTags.end(); ++it )
      this->InteractorStyle->RemoveObserver( ( *it ) );
  }    

  if( this->RenderWindow && this->Renderer )
    this->RenderWindow->RemoveRenderer( this->Renderer );

  if( this->Renderer )
    this->Renderer->RemoveViewProp( this->ImageActor );

  if( this->Interactor )
  {
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

  if( components == 1 )
  {
    dataMin = input->GetScalarRange()[0];
    dataMax = input->GetScalarRange()[1];
  }
  else
  {
    vtkDataArray* data = 0;
    if( input->GetPointData() && (data = input->GetPointData()->GetScalars()) != 0 )
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

  if( fabs( this->OriginalWindow ) < 0.001 )
  {
    this->OriginalWindow = 0.001 * ( this->OriginalWindow < 0.0 ? -1 : 1 );
  }

  if( fabs( this->OriginalLevel ) < 0.001 )
  {
    this->OriginalLevel = 0.001 * ( this->OriginalLevel < 0.0 ? -1 : 1 );
  }
  
  // VTK_LUMINANCE is defined in vtkSystemIncludes.h
  if( this->WindowLevel->GetOutputFormat() == VTK_LUMINANCE )
  {
    this->SetColorWindowLevel( this->OriginalWindow, this->OriginalLevel );
  }
  else
  {
    this->OriginalWindow = 255;
    this->OriginalLevel = 127.5;
    this->SetColorWindowLevel( 255, 127.5 );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::InitializeCameraViews()
{
  vtkImageData* input = this->GetInput();
  if( !input ) return;

  input->UpdateInformation();
  double* origin = input->GetOrigin();
  double* spacing = input->GetSpacing();
  int* extent = input->GetWholeExtent();

  int u, v;
  double fpt[3];
  double pos[3];

  for( int w = 0; w < 3; ++w )
  {
    double vup[3] = { 0.0, 0.0, 0.0 };
    switch( w )
    {
      case 0: u = 1; v = 2; vup[2] = 1.0; break;
      case 1: u = 0; v = 2; vup[2] = 1.0; break;
      case 2: u = 0; v = 1; vup[1] = 1.0; break;
    }

    // setup the center
    fpt[u] = pos[u] = origin[u] + 0.5 * spacing[u] * ( extent[2*u] + extent[2*u+1] );
    fpt[v] = pos[v] = origin[v] + 0.5 * spacing[v] * ( extent[2*v] + extent[2*v+1] );

    // setup the in and out
    fpt[w] = origin[w] + spacing[w] * extent[2*w];
    pos[w] = origin[w] + ( w == 1 ? -1.0 : 1.0 ) * spacing[w] * extent[2*w+1];

    for( int i = 0; i < 3; ++i )
    {
      this->CameraPosition[w][i]   = pos[i];
      this->CameraFocalPoint[w][i] = fpt[i];
      this->CameraViewUp[w][i]     = vup[i];
    }
    this->CameraParallelScale[w] = VTK_FLOAT_MIN;

    int* range = input->GetWholeExtent() + 2*w;
    if( range )
    {
      this->LastSlice[w] = range[1];
    }
  }
  this->Slice = this->LastSlice[this->ViewOrientation];
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::RecordCameraView()
{
  vtkCamera *cam = this->Renderer ? this->Renderer->GetActiveCamera() : 0;

  if( cam )  // record camera view
  {
    double pos[3];
    double fpt[3];
    double v[3];
    cam->GetPosition( pos );
    cam->GetFocalPoint( fpt );
    cam->GetViewUp( v );
    for( int i = 0; i < 3; ++i )
    {   
      this->CameraPosition[this->ViewOrientation][i]   = pos[i];
      this->CameraFocalPoint[this->ViewOrientation][i] = fpt[i];
      this->CameraViewUp[this->ViewOrientation][i]     = v[i];
    }   
    this->CameraParallelScale[this->ViewOrientation] = cam->GetParallelScale();
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

  if( this->Slice == slice ) return;

  this->LastSlice[this->ViewOrientation] =  this->Slice;
  this->RecordCameraView();
 
  this->Slice = slice;
  this->Modified();

  this->UpdateDisplayExtent();

  if( this->Cursor && this->Annotate )
  {
    this->CursorWidget->UpdateMessageString();
    this->Annotation->SetText( 0, this->CursorWidget->GetMessageString() );
  }   

  this->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::SetViewOrientation( const int& orientation )
{
  if( orientation < vtkMedicalImageViewer::VIEW_ORIENTATION_YZ ||
      orientation > vtkMedicalImageViewer::VIEW_ORIENTATION_XY )
  {
    return;
  }
  
  if( this->ViewOrientation == orientation ) return;
    
  this->ViewOrientation = orientation;

  this->RecordCameraView();

  // Update the viewer 

  int *range = this->GetSliceRange();
  if( range )
  {
    this->Slice = range[1];
    this->Slice = static_cast<int>( ( range[0]+range[1] )*0.5 );
  }

  this->UpdateDisplayExtent();
  this->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::UpdateDisplayExtent()
{
  vtkImageData *input = this->GetInput();
  if( !input ) return;

  input->UpdateInformation();
  int *w_ext = input->GetWholeExtent();

  // Is the slice in range ? If not, fix it

  int slice_min = w_ext[this->ViewOrientation * 2];
  int slice_max = w_ext[this->ViewOrientation * 2 + 1];
  if( this->Slice < slice_min || this->Slice > slice_max )
  {
    this->Slice = static_cast<int>( ( slice_min + slice_max ) * 0.5 );
  }

  // Set the image actor

  switch ( this->ViewOrientation )
  {
    case vtkMedicalImageViewer::VIEW_ORIENTATION_XY:
      this->ImageActor->SetDisplayExtent(
        w_ext[0], w_ext[1], w_ext[2], w_ext[3], this->Slice, this->Slice );
      break;

    case vtkMedicalImageViewer::VIEW_ORIENTATION_XZ:
      this->ImageActor->SetDisplayExtent(
        w_ext[0], w_ext[1], this->Slice, this->Slice, w_ext[4], w_ext[5] );
      break;

    case vtkMedicalImageViewer::VIEW_ORIENTATION_YZ:
      this->ImageActor->SetDisplayExtent(
        this->Slice, this->Slice, w_ext[2], w_ext[3], w_ext[4], w_ext[5] );
      break;
  }

  this->WindowLevel->GetOutput()->SetUpdateExtent(
    this->ImageActor->GetDisplayExtent() );

  if( this->Renderer )
  {
    if( this->CameraParallelScale[this->ViewOrientation] == VTK_FLOAT_MIN )
    {
      this->Renderer->ResetCamera();
      this->RecordCameraView();
    }
    else
    {
      this->Renderer->ResetCameraClippingRange( this->ImageActor->GetBounds() );
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::Render()
{
  if( this->RenderWindow ) this->RenderWindow->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double vtkMedicalImageViewer::GetSliceLocation()
{
  return this->ImageActor->GetBounds()[2*this->ViewOrientation];
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

  this->Annotation->SetImageActor( this->ImageActor );
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
  this->Annotation->SetImageActor( 0 );
  this->Annotation->SetWindowLevel( 0 );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkMedicalImageViewer::InstallCursor()
{
  if( this->Interactor && this->Renderer && this->GetInput() )
  {
    this->CursorWidget->SetDefaultRenderer( this->Renderer );
    this->CursorWidget->SetInteractor( this->Interactor );
    this->CursorWidget->SetInput( this->GetInput() );
    this->CursorWidget->AddViewProp( this->ImageActor );

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
void vtkMedicalImageViewer::SetInterpolate( const int& arg )
{
  this->Interpolate = arg;
  this->CursorWidget->SetCursoringMode( this->Interpolate ?
    vtkImageCoordinateWidget::Continuous :
    vtkImageCoordinateWidget::Discrete );

  this->ImageActor->SetInterpolate( this->Interpolate );

  if( this->GetInput() ) this->Render();
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
void vtkMedicalImageViewer::SetFrameRate( const int& arg )
{
  if( this->FrameRate == arg ) return;
  if( arg > this->MaxFrameRate || arg < 0 ) return;
  this->FrameRate = arg;
  this->AnimationScene->SetFrameRate( this->FrameRate );
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

  os << indent << "ImageActor:\n";
  this->ImageActor->PrintSelf( os, indent.GetNextIndent() );
  
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
