/*=========================================================================

  Module:    vtkCustomCornerAnnotation.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Patrick Emond <emondpd AT mcmaster DOT ca>
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class vtkCustomCornerAnnotation
 *
 * @brief Text annotation in four corners
 *
 * This is an annotation object that manages four text actors / mappers
 * to provide annotation in the four corners of a viewport.
 * Special input text:
 * - <image> : will be replaced with slice number (relative number)
 * - <slice> : will be replaced with slice number (relative number)
 * - <image_and_max> : will be replaced with slice number and slice max (relative)
 * - <slice_and_max> : will be replaced with slice number and slice max (relative)
 * - <slice_pos> : will be replaced by the position of the current slice
 * - <window> : will be replaced with window value
 * - <level> : will be replaced with level value
 * - <window_level> : will be replaced with window and level value
 *
 * @see vtkActor2D, vtkTextMapper
 */

#ifndef __vtkCustomCornerAnnotation_h
#define __vtkCustomCornerAnnotation_h

// VTK includes
#include <vtkActor2D.h>
#include <vtkSmartPointer.h>

// C++ includes
#include <string>

class vtkImageSlice;
class vtkImageWindowLevel;
class vtkTextMapper;
class vtkTextProperty;

class vtkCustomCornerAnnotation : public vtkActor2D
{
  public:
    static vtkCustomCornerAnnotation* New();
    vtkTypeMacro(vtkCustomCornerAnnotation, vtkActor2D);
    void PrintSelf(ostream& os, vtkIndent indent);

    //@{
    /**
     * Draw the scalar bar and annotation text to the screen.
     */
    int RenderOpaqueGeometry(vtkViewport* viewport);
    virtual int RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
    {
      return 0;
    };
    int RenderOverlay(vtkViewport* viewport);
    //@}

    /** Does this prop have some translucent polygonal geometry? */
    virtual int HasTranslucentPolygonalGeometry();

    //@{
    /**
     * Set/Get the maximum height of a line of text as a
     * percentage of the vertical area allocated to this
     * scaled text actor. Defaults to 1.0
     */
    vtkSetMacro(MaximumLineHeight, double);
    vtkGetMacro(MaximumLineHeight, double);
    //@}

    //@{
    /**
     * Set/Get the minimum/maximum size font that will be shown.
     * If the font drops below the minimum size it will not be rendered.
     */
    vtkSetMacro(MinimumFontSize, int);
    vtkGetMacro(MinimumFontSize, int);
    vtkSetMacro(MaximumFontSize, int);
    vtkGetMacro(MaximumFontSize, int);
    //@}

    //@{
    /**
     * Set/Get font scaling factors
     * The font size, f, is calculated as the largest possible value
     * such that the annotations for the given viewport do not overlap.
     * This font size is scaled non-linearly with the viewport size,
     * to maintain an acceptable readable size at larger viewport sizes,
     * without being too big.
     * f' = linearScale * pow(f,nonlinearScale)
     */
    vtkSetMacro(LinearFontScaleFactor, double);
    vtkGetMacro(LinearFontScaleFactor, double);
    vtkSetMacro(NonlinearFontScaleFactor, double);
    vtkGetMacro(NonlinearFontScaleFactor, double);
    //@}

    /**
     * Release any graphics resources that are being consumed by this actor.
     * The parameter window could be used to determine which graphic
     * resources to release.
     */
    virtual void ReleaseGraphicsResources(vtkWindow *window);

    //@{
    /** Set/Get the text to be displayed for each corner */
    void SetText(int i, const char* text);
    const char* GetText(int i);
    void ClearAllTexts();
    void CopyAllTextsFrom(vtkCustomCornerAnnotation* annotation);
    //@}

    //@{
    /** Set an image actor to look at for slice information */
    void SetImageSlice(vtkImageSlice* slice);
    vtkGetObjectMacro(ImageSlice, vtkImageSlice);
    //@}

    //@{
    /**
     * Set an instance of vtkImageWindowLevel to use for
     * looking at window level changes
     */
    void SetWindowLevel(vtkImageWindowLevel* wl);
    vtkGetObjectMacro(WindowLevel, vtkImageWindowLevel);
    //@}

    //@{
    /** Set / Get the value to shift the level by. */
    vtkSetMacro(LevelShift, double);
    vtkGetMacro(LevelShift, double);
    //@}

    //@{
    /** Set the value to scale the level by. */
    vtkSetMacro(LevelScale, double);
    vtkGetMacro(LevelScale, double);
    //@}

    //@{
    /** Set/Get the text property of all corners. */
    virtual void SetTextProperty(vtkTextProperty* property);
    vtkGetObjectMacro(TextProperty, vtkTextProperty);
    //@}

    //@{
    /**
     * Even if there is an image actor, should `slice' and `image' be displayed?
     */
    vtkBooleanMacro(ShowSliceAndImage, int);
    vtkSetMacro(ShowSliceAndImage, int);
    vtkGetMacro(ShowSliceAndImage, int);
    //@}

  protected:
    vtkCustomCornerAnnotation();
    ~vtkCustomCornerAnnotation();

    double MaximumLineHeight;

    vtkTextProperty* TextProperty;

    vtkImageWindowLevel* WindowLevel;
    double LevelShift;
    double LevelScale;
    vtkImageSlice* ImageSlice;
    vtkImageSlice* LastImageSlice;

    std::string CornerText[4];

    int FontSize;
    vtkSmartPointer<vtkActor2D> TextActor[4];
    vtkTimeStamp BuildTime;
    int LastSize[2];
    vtkSmartPointer<vtkTextMapper> TextMapper[4];

    int MinimumFontSize;
    int MaximumFontSize;

    double LinearFontScaleFactor;
    double NonlinearFontScaleFactor;

    int ShowSliceAndImage;

    /** Search for replacable tokens and replace */
    virtual void TextReplace(
      vtkImageSlice* slice, vtkImageWindowLevel* wl);

    /**
     * Set text actor positions given a viewport size and justification
     */
    virtual void SetTextActorsPosition(int vsize[2]);
    virtual void SetTextActorsJustification();

  private:
    vtkCustomCornerAnnotation(const vtkCustomCornerAnnotation&);  /** Not implemented. */
    void operator=(const vtkCustomCornerAnnotation&);   /** Not implemented. */
};

#endif
