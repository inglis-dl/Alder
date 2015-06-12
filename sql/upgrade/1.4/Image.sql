UPDATE Image
JOIN Exam ON Exam.Id=Image.ExamId
JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
JOIN Modality ON Modality.Id=ScanType.ModalityId
SET Dimensionality=2
WHERE Dimensionality IS NULL
AND Modality.Name IN ('Retinal','Dexa');

UPDATE Image
JOIN Exam ON Exam.Id=Image.ExamId
JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
SET Dimensionality=3
WHERE Dimensionality IS NULL
AND ScanType.Type IN ('CarotidIntima','Plaque')
AND ParentImageId IS NULL;

UPDATE Image
JOIN Exam ON Exam.Id=Image.ExamId
JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
SET Dimensionality=2
WHERE Dimensionality IS NULL
AND ScanType.Type='CarotidIntima'
AND ParentImageId IS NOT NULL;

UPDATE Image
JOIN Rating ON Image.Id=Rating.ImageId
JOIN Exam ON Exam.Id=Image.ExamId
JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
JOIN Modality ON Modality.Id=ScanType.ModalityId
SET Note = CONCAT_WS(', ',Note, 'ZE')
WHERE Rating.Rating = 1
AND Modality.Name='Ultrasound'
AND Image.Note IS NOT NULL
AND (
  LOCATE('NO',Image.Note) OR
  LOCATE('NA',Image.Note) OR
  LOCATE('NI',Image.Note)
);

UPDATE Image
JOIN Rating ON Image.Id=Rating.ImageId
JOIN Exam ON Exam.Id=Image.ExamId
JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
JOIN Modality ON Modality.Id=ScanType.ModalityId
SET Note = CONCAT_WS(', ',Note, 'ZE')
WHERE Rating.Rating = 1
AND Modality.Name='Ultrasound'
AND (
  IF( LOCATE('ZE',Image.Note),0,
      LOCATE('ME', Image.Note)
      AND (
        LOCATE('AN',Image.Note) OR
        LOCATE('AS',Image.Note) OR
        LOCATE('CS',Image.Note) OR
        LOCATE('PC',Image.Note) OR
        LOCATE('SK',Image.Note) OR
        LOCATE('LO',Image.Note) OR
        LOCATE('SB',Image.Note)
      )
  )
);

UPDATE Image
JOIN Rating ON Image.Id=Rating.ImageId
JOIN Exam ON Exam.Id=Image.ExamId
JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
JOIN Modality ON Modality.Id=ScanType.ModalityId
SET Note = CONCAT_WS(', ',Note, 'ZE')
WHERE Rating.Rating = 1
AND Modality.Name='Ultrasound'
AND (
  IF( LOCATE('ZE',Image.Note),0,
      LOCATE('SB', Image.Note)
      AND (
        LOCATE('AN',Image.Note) OR
        LOCATE('AS',Image.Note) OR
        LOCATE('CS',Image.Note) OR
        LOCATE('PC',Image.Note) OR
        LOCATE('SK',Image.Note) OR
        LOCATE('LO',Image.Note) OR
        LOCATE('ME',Image.Note)
      )
  )
);
