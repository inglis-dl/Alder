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
  END //
DELIMITER ;

CALL patch_Exam();
DROP PROCEDURE IF EXISTS patch_Exam;
