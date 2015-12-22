DROP PROCEDURE IF EXISTS patch_Code;
DELIMITER //
CREATE PROCEDURE patch_Code()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLE_CONSTRAINTS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Code");

    IF @test THEN
      SELECT "Modifying Code table referential actions" AS "";

      SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
      SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

      ALTER TABLE Code DROP FOREIGN KEY fkCodeImageId;

      ALTER TABLE Code ADD CONSTRAINT fkCodeImageId
        FOREIGN KEY (ImageId) REFERENCES Image (Id)
        ON DELETE NO ACTION ON UPDATE CASCADE;

      ALTER TABLE Code DROP FOREIGN KEY fkCodeUserId;

      ALTER TABLE Code ADD CONSTRAINT fkCodeUserId
        FOREIGN KEY (UserId) REFERENCES User (Id)
        ON DELETE NO ACTION ON UPDATE CASCADE;

      ALTER TABLE Code DROP FOREIGN KEY fkCodeCodeTypeId;

      ALTER TABLE Code ADD CONSTRAINT fkCodeCodeTypeId
        FOREIGN KEY (CodeTypeId) REFERENCES CodeType (Id)
        ON DELETE NO ACTION ON UPDATE CASCADE;

      SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
      SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
    END IF;

  END //
DELIMITER ;

CALL patch_Code();
DROP PROCEDURE IF EXISTS patch_Code;