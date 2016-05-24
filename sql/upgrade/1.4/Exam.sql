DROP PROCEDURE IF EXISTS patch_Exam;
DELIMITER //
CREATE PROCEDURE patch_Exam()
  BEGIN

    SELECT "Updating Exam table with new unique index" AS "";

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLE_CONSTRAINTS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Exam"
      AND CONSTRAINT_NAME = "uqInterviewIdModalityIdTypeLaterality" );

    IF @test = 1 THEN

      ALTER TABLE Exam
      DROP INDEX uqInterviewIdModalityIdTypeLaterality;

      ALTER TABLE Exam
      ADD UNIQUE INDEX
      uqInterviewIdModalityIdTypeLaterality (InterviewId,ModalityId,Type,Laterality);

    END IF;

  END //
DELIMITER ;

CALL patch_Exam();
DROP PROCEDURE IF EXISTS patch_Exam;
