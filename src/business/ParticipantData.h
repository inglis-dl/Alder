/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   ParticipantData.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class ParticipantData
 * @namespace Alder
 *
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Class which contains data records for one participant UID
 *
 */

#ifndef __ParticipantData_h
#define __ParticipantData_h

// Alder includes
#include <ModelObject.h>

// VTK includes
#include <vtkSmartPointer.h>

// C++ includes
#include <map>
#include <string>
#include <vector>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Exam;
  class Image;
  class Interview;
  class Wave;
  class ParticipantData : public ModelObject
  {
    public:
      static ParticipantData *New();
      vtkTypeMacro(ParticipantData, ModelObject);

    /** 
     * Load an Interview from a database record primary key.
     * @param id the Id of an interview record 
     * @return success or fail to load the interview
     */
    bool LoadInterview(const int& id);

    /** 
     * Load Wave, Interview, Exam, and Image records associated with a participant UID.
     * The private map containers are cleared and filled accordingly with ActiveRecord
     * instances that relate to the UID. This is a helper class for GUI elements
     * such as the QAlderInterviewWidget.
     * This method invokes Common::DataChangedEvent
     * @param id the UID of a participant
     * @return success or fail to load the data
     */
    bool LoadData(const std::string& _uid);

    /**
     * Reload the data for the current UID.  This method should be called
     * whenever a change to the database occurs that may affect the data.
     * This method invokes Common::DataChangedEvent
     */
    void ReloadData();

    /**
     * Given a Wave instance, populate a vector of child Interview records.
     * @param Wave a wave
     * @param vector<vtkSmartPointer<Interview>>* container pointer
     */
    void GetInterviewList(Wave*,
      std::vector<vtkSmartPointer<Interview>>* container);

    /**
     * Populate a vector of all Wave records in the database.
     * @param vector<vtkSmartPointer<Wave>>* container pointer
     */
    void GetWaveList(
      std::vector<vtkSmartPointer<Wave>>* container);

    /**
     * Given an Interview instance, populate a vector of child Exam records.
     * @param Interview an interview
     * @param vector<vtkSmartPointer<Exam>>* container pointer
     */
    void GetExamList(Interview*,
      std::vector<vtkSmartPointer<Exam>>* container);

    /**
     * Given an Exam instance, populate a vector of child Image records.
     * @param Exam an exam
     * @param vector<vtkSmartPointer<Image>>* container pointer
     */
    void GetParentImageList(Exam*,
      std::vector<vtkSmartPointer<Image>>* container);

    /**
     * Given an Image instance, populate a vector of child Image records.
     * @param Image an image
     * @param vector<vtkSmartPointer<Image>>* container pointer
     */
    void GetChildImageList(Image*,
      std::vector<vtkSmartPointer<Image>>* container);

    /**
     * Get the UID that the data relates to.
     * @return a UID
     */
    std::string GetUID() { return this->uid; }

    /**
     * Get the active Interview.
     * @return Interview
     */
    vtkGetObjectMacro(ActiveInterview, Interview);

    /**
  I  * Get the active Image.
     * @return Image
     */
    vtkGetObjectMacro(ActiveImage, Image);

    /**
     * Set the active Interview. The internal containers are updated
     * accordingly.  The active image is updated to either a similar image
     * if one is found or NULL if none is found.
     * This method invokes Common::InterviewChangedEvent
     * @param Interview an interview
     */
    void SetActiveInterview(Interview *interview);

    /**
     * Set the active Image.
     * This method invokes Common::ImageChangedEvent
     * @param Image an image
     */
    void SetActiveImage(Image *image);

    protected:
      ParticipantData();
      ~ParticipantData();

      void Clear();
      void BuildData();

      std::string uid;
      std::map<Exam*, std::vector<vtkSmartPointer<Image>>> examImageMap;
      std::map<Image*, std::vector<vtkSmartPointer<Image>>> childImageMap;
      std::map<Interview*, std::vector<vtkSmartPointer<Exam>>> interviewExamMap;
      std::map<Wave*, std::vector<vtkSmartPointer<Interview>>> waveInterviewMap;
      std::map<int, vtkSmartPointer<Wave>> rankWaveMap;

      Interview *ActiveInterview;
      Image *ActiveImage;

    private:
      ParticipantData(const ParticipantData&);  // Not implemented
      void operator=(const ParticipantData&);  // Not implemented
  };
}  // namespace Alder

/** @} end of doxygen group */

#endif
