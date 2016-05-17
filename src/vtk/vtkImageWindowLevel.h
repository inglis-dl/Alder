/*=========================================================================

  Module:    vtkImageWindowLevel.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Patrick Emond <emondpd AT mcmaster DOT ca>
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class vtkImageWindowLevel
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Map the input image through a lookup table and window / level it
 *
 * The vtkImageWindowLevel filter will take an input image of any
 * valid scalar type, and map the components of the image through a
 * lookup table.  This resulting color will be modulated with value obtained
 * by a window / level operation. The result is an image of type
 * VTK_UNSIGNED_CHAR. This class is based on vtkImageMapToWindowLevelColors
 * and will revert to only modulating the first component of a multi-component
 * image if the lookup table is set.
 *
 * @see vtkLookupTable, vtkScalarsToColors
 */

#ifndef __vtkImageWindowLevel_h
#define __vtkImageWindowLevel_h

// VTK includes
#include <vtkImageMapToColors.h>

class vtkImageWindowLevel : public vtkImageMapToColors
{
  public:
    static vtkImageWindowLevel* New();
    vtkTypeMacro(vtkImageWindowLevel, vtkImageMapToColors);
    void PrintSelf(ostream& os, vtkIndent indent);

    //@{
    /**
     * Set / Get the Window to use -> modulation will be performed on the
     * color based on (S - (L - W/2))/W where S is the scalar value, L is
     * the level and W is the window.
     */
    vtkSetMacro(Window, double);
    vtkGetMacro(Window, double);
    //@}

    //@{
    /**
     * Set / Get the Level to use -> modulation will be performed on the
     * color based on (S - (L - W/2))/W where S is the scalar value, L is
     * the level and W is the window.
     */
    vtkSetMacro(Level, double);
    vtkGetMacro(Level, double);
    //@}

  protected:
    vtkImageWindowLevel();
    ~vtkImageWindowLevel();

    virtual int RequestInformation(
      vtkInformation* request,
      vtkInformationVector** inputVector,
      vtkInformationVector* outputVector);
    void ThreadedRequestData(
      vtkInformation* request,
      vtkInformationVector** inputVector,
      vtkInformationVector* outputVector,
      vtkImageData*** inData, vtkImageData** outData,
      int extent[6], int id);
    virtual int RequestData(
      vtkInformation* request,
      vtkInformationVector** inputVector,
      vtkInformationVector* outputVector);

    double Window;
    double Level;

  private:
    vtkImageWindowLevel(const vtkImageWindowLevel&);  /** Not implemented.*/
    void operator=(const vtkImageWindowLevel&);  /** Not implemented.*/
};

#endif
