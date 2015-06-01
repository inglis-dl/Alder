UPDATE Image
JOIN Exam ON Exam.Id=Image.ExamId
SET Dimensionality=2
WHERE Dimensionality IS NULL
AND ModalityId IN
(
  SELECT Id FROM Modality
  WHERE Name IN ('Retinal','Dexa')
);

UPDATE Image
JOIN Exam ON Exam.Id=Image.ExamId
SET Dimensionality=3
WHERE Dimensionality IS NULL
AND Type IN ('CarotidIntima','Plaque')
AND ParentImageId IS NULL;

UPDATE Image
JOIN Exam ON Exam.Id=Image.ExamId
SET Dimensionality=2
WHERE Dimensionality IS NULL
AND Type='CarotidIntima'
AND ParentImageId IS NOT NULL;

UPDATE Image
JOIN Rating ON Image.Id=Rating.ImageId
JOIN Exam ON Exam.Id=Image.ExamId
JOIN Modality ON Exam.ModalityId=Modality.Id
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
JOIN Modality ON Exam.ModalityId=Modality.Id
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
JOIN Modality ON Exam.ModalityId=Modality.Id
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
