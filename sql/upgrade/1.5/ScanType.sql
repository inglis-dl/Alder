DROP PROCEDURE IF EXISTS patch_ScanType;
DELIMITER //
CREATE PROCEDURE patch_ScanType()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLE_CONSTRAINTS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "ScanType");

    IF @test THEN
      SELECT "Modifying ScanType table referential actions" AS "";

      SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
      SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

      ALTER TABLE ScanType DROP FOREIGN KEY fkScanTypeModalityId;

      ALTER TABLE ScanType ADD CONSTRAINT fkScanTypeModalityId
        FOREIGN KEY (ModalityId) REFERENCES Modality (Id)
        ON DELETE NO ACTION ON UPDATE CASCADE;

      SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
      SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
    END IF;

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "ScanType"
      AND COLUMN_NAME IN ("SideCount","AcquisitionCount","ChildCount",
      "WaveId","AcquisitionNameFormat","ChildNameFormat","FileSuffix"));

    IF @test=0 THEN
      SELECT "Adding new columns to ScanType table" AS "";

      SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
      SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

      ALTER TABLE ScanType ADD COLUMN SideCount TINYINT NOT NULL DEFAULT 0;
      ALTER TABLE ScanType ADD COLUMN AcquisitionCount TINYINT NOT NULL DEFAULT 1;
      ALTER TABLE ScanType ADD COLUMN ChildCount TINYINT NOT NULL DEFAULT 0;
      ALTER TABLE ScanType ADD COLUMN AcquisitionNameFormat VARCHAR(255) NOT NULL;
      ALTER TABLE ScanType ADD COLUMN ChildNameFormat VARCHAR(255) NULL DEFAULT NULL;
      ALTER TABLE ScanType ADD COLUMN FileSuffix VARCHAR(45) NULL DEFAULT NULL;

      ALTER TABLE ScanType ADD COLUMN WaveId INT UNSIGNED NOT NULL AFTER ModalityId;
      ALTER TABLE ScanType ADD INDEX fkWaveId ( WaveId ASC );
      ALTER TABLE ScanType ADD CONSTRAINT fkScanTypeWaveId
      FOREIGN KEY ( WaveId ) REFERENCES Wave (Id)
      ON DELETE NO ACTION ON UPDATE CASCADE;

      ALTER TABLE ScanType DROP INDEX uqModalityIdType;
      ALTER TABLE ScanType ADD INDEX uqTypeModalityIdWaveId (`Type` ASC, ModalityId ASC, WaveId ASC);

      UPDATE ScanType
      SET WaveId = ( SELECT Id FROM Wave WHERE RANK=1 );

      SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
      SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
    END IF;
  END //
DELIMITER ;

CALL patch_ScanType();
DROP PROCEDURE IF EXISTS patch_ScanType;
