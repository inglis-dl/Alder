UPDATE Rating
LEFT JOIN Image ON Image.Id=Rating.ImageId 
LEFT JOIN Exam ON Exam.Id=Image.ExamId 
LEFT JOIN Interview ON Interview.Id=Exam.InterviewId 
SET Rating.Rating=1
WHERE Rating.Rating IS NULL
AND Image.Note IS NOT NULL
