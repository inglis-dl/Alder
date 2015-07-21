SELECT CONCAT('/data/alder/', Interview.Id, '/', Exam.Id, '/', Image.Id) AS path
FROM Image 
JOIN Exam ON Exam.Id=Image.ExamId 
JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
JOIN Interview ON Interview.Id=Exam.InterviewId 
WHERE ScanType.Type = 'Plaque' 
INTO OUTFILE '/tmp/plaque_remove.csv'
LINES TERMINATED BY '\n';

CREATE TEMPORARY TABLE tmp AS
SELECT Image.Id AS Id FROM Image 
JOIN Exam ON Exam.Id=Image.ExamId 
JOIN ScanType ON ScanType.Id=Exam.ScanTypeId  
WHERE ScanType.Type='Plaque';

DELETE Image.* FROM Image
WHERE ParentImageId IN (
  SELECT Id FROM tmp
);

DROP TABLE tmp;

DELETE Image.* FROM Image
JOIN Exam ON Exam.Id=Image.ExamId
JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
WHERE ScanType.Type='Plaque';

DELETE Exam.* FROM Exam
JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
WHERE ScanType.Type='Plaque';

DELETE ScanType.* FROM ScanType
WHERE ScanType.Type='Plaque';
