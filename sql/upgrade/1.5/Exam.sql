DROP PROCEDURE IF EXISTS patch_Exam;
DELIMITER //
CREATE PROCEDURE patch_Exam()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLE_CONSTRAINTS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Exam");

    IF @test THEN
      SELECT "Modifying Exam table referential actions" AS "";

      SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
      SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

      ALTER TABLE Exam DROP FOREIGN KEY fkExamInterviewId;

      ALTER TABLE Exam ADD CONSTRAINT fkExamInterviewId
        FOREIGN KEY (InterviewId) REFERENCES Interview (Id)
        ON DELETE NO ACTION ON UPDATE CASCADE;

      ALTER TABLE Exam DROP FOREIGN KEY fkExamScanTypeId;

      ALTER TABLE Exam ADD CONSTRAINT fkExamScanTypeId
        FOREIGN KEY (ScanTypeId) REFERENCES ScanType (Id)
        ON DELETE NO ACTION ON UPDATE CASCADE;

      SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
      SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

      SELECT "Enforcing singular ForearmBoneDensity exams" AS "";

      DELETE Exam FROM Exam
      JOIN (
        SELECT Exam.Id FROM Exam
        JOIN Interview ON Interview.Id=Exam.InterviewId
        JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
        WHERE Stage="NotApplicable"
        AND Type="ForearmBoneDensity"
        GROUP BY InterviewId
      ) AS x ON x.Id=Exam.Id;

      DELETE Exam FROM Exam
      JOIN (
        SELECT Exam.Id FROM Exam
        JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
        JOIN (
          SELECT Exam.InterviewId FROM Exam
          JOIN Interview ON Interview.Id=Exam.InterviewId
          JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
          AND Type="ForearmBoneDensity"
          GROUP BY InterviewId
          HAVING COUNT(*)>1
        ) AS y ON y.InterviewId=Exam.InterviewId
        WHERE Type="ForearmBoneDensity"
        AND Downloaded=0
      )  AS x ON x.Id=Exam.Id;
    END IF;

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Exam"
      AND COLUMN_NAME = "Laterality");

    IF @test THEN
      SELECT "Renaming Laterality column in Exam table to Side" AS "";

      SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
      SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

      UPDATE Exam SET Laterality='none' WHERE Laterality IS NULL;
      ALTER TABLE Exam DROP INDEX dkLaterality;
      ALTER TABLE Exam DROP INDEX uqInterviewIdLateralityScanTypeId;
      ALTER TABLE Exam CHANGE Laterality Side enum('right','left','none') NOT NULL DEFAULT 'none';
      ALTER TABLE Exam ADD INDEX dkSide (Side ASC);
      ALTER TABLE Exam ADD UNIQUE INDEX uqInterviewIdSideScanTypeId (InterviewId ASC, Side ASC, ScanTypeId ASC);

      SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
      SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
    END IF;

    SET @test = (
      SELECT IF(COUNT(*)=1,0,1)
      FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Exam"
      AND COLUMN_NAME = "ScanIndex");

    IF @test=0 THEN
      SELECT "Adding ScanIndex column to Exam table" AS "";

      ALTER TABLE Exam ADD COLUMN SideIndex TINYINT NULL DEFAULT NULL;
    END IF;  
  END //
DELIMITER ;

CALL patch_Exam();
DROP PROCEDURE IF EXISTS patch_Exam;
