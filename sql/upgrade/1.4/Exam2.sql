DROP PROCEDURE IF EXISTS patch_Exam;
DELIMITER //
CREATE PROCEDURE patch_Exam()
  BEGIN

    SELECT "Updating Exam table with new ScanTypeId column" AS "";

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Exam"
      AND COLUMN_NAME = "ScanTypeId" );

    IF @test = 0 THEN

      SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
      SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

      ALTER TABLE Exam
      ADD COLUMN ScanTypeId INT UNSIGNED NOT NULL AFTER InterviewId;

      UPDATE Exam
      JOIN ScanType ON ScanType.ModalityId=Exam.ModalityId
      AND ScanType.Type=Exam.Type
      SET ScanTypeId=ScanType.Id;

      ALTER TABLE Exam
      ADD INDEX fkScanTypeId ( ScanTypeId ASC ),
      ADD CONSTRAINT fkExamScanTypeId
      FOREIGN KEY ( ScanTypeId ) REFERENCES ScanType (Id)
      ON DELETE NO ACTION
      ON UPDATE NO ACTION;

      ALTER TABLE Exam
      DROP INDEX uqInterviewIdModalityIdTypeLaterality;

      ALTER TABLE Exam
      ADD UNIQUE INDEX uqInterviewIdLateralityScanTypeId
      (InterviewId ASC, Laterality ASC, ScanTypeId ASC);

      ALTER TABLE Exam DROP FOREIGN KEY fkExamModalityId;

      ALTER TABLE Exam DROP KEY dkType;

      ALTER TABLE Exam DROP COLUMN ModalityId;

      ALTER TABLE Exam DROP COLUMN Type;

      SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
      SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

    END IF;

  END //
DELIMITER ;

CALL patch_Exam();
DROP PROCEDURE IF EXISTS patch_Exam;
