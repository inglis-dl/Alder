SELECT CONCAT('/data/alder/', Interview.Id, '/', Exam.Id, '/', Image.Id) AS path
FROM Image 
JOIN Exam ON Exam.Id=Image.ExamId 
JOIN Interview ON Interview.Id=Exam.InterviewId 
WHERE Exam.Type = 'Plaque' 
INTO OUTFILE '/tmp/plaque_remove.csv'
LINES TERMINATED BY '\n';

DELETE Image.* FROM Image
JOIN Exam ON Exam.Id=Image.ExamId
WHERE Exam.Type='Plaque';

DELETE Exam.* FROM Exam
WHERE Type='Plaque';
