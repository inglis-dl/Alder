ALTER TABLE Exam
DROP INDEX uqInterviewIdModalityIdTypeLaterality;

ALTER TABLE Exam
ADD UNIQUE INDEX
uqInterviewIdModalityIdTypeLaterality (InterviewId,ModalityId,Type,Laterality);
