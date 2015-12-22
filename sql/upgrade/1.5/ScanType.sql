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
      AND COLUMN_NAME IN ("SideCount","Parenting"));

    IF @test=0 THEN
      SELECT "Adding SideCount and Parenting columns to ScanType table" AS "";

      ALTER TABLE ScanType ADD COLUMN SideCount TINYINT NOT NULL DEFAULT 0;
      ALTER TABLE ScanType ADD COLUMN Parenting TINYINT NOT NULL DEFAULT 0;
    END IF;
  END //
DELIMITER ;

CALL patch_ScanType();
DROP PROCEDURE IF EXISTS patch_ScanType;
