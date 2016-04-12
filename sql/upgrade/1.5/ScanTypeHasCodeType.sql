DROP PROCEDURE IF EXISTS patch_ScanTypeHasCodeType;
DELIMITER //
CREATE PROCEDURE patch_ScanTypeHasCodeType()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLE_CONSTRAINTS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "ScanTypeHasCodeType"
      AND CONSTRAINT_NAME="uqCodeTypeIdScanTypeId");

    IF @test THEN
      SELECT "Removing unique index from ScanTypeHasCodeType table" AS "";

      SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
      SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

      ALTER TABLE ScanTypeHasCodeType DROP INDEX uqCodeTypeIdScanTypeId;

      ALTER TABLE ScanTypeHasCodeType DROP FOREIGN KEY fkScanTypeHasCodeTypeScanTypeId;

      ALTER TABLE ScanTypeHasCodeType ADD CONSTRAINT fkScanTypeHasCodeTypeScanTypeId
        FOREIGN KEY (ScanTypeId) REFERENCES ScanType (Id)
        ON DELETE CASCADE ON UPDATE CASCADE;

      ALTER TABLE ScanTypeHasCodeType DROP FOREIGN KEY fkScanTypeHasCodeTypeCodeTypeId;

      ALTER TABLE ScanTypeHasCodeType ADD CONSTRAINT fkScanTypeHasCodeTypeCodeTypeId
        FOREIGN KEY (CodeTypeId) REFERENCES CodeType (Id)
        ON DELETE CASCADE ON UPDATE CASCADE;

      SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
      SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
    END IF;

  END //
DELIMITER ;

CALL patch_ScanTypeHasCodeType();
DROP PROCEDURE IF EXISTS patch_ScanTypeHasCodeType;
