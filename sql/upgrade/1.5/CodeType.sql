DROP PROCEDURE IF EXISTS patch_CodeType;
DELIMITER //
CREATE PROCEDURE patch_CodeType()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLE_CONSTRAINTS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "CodeType");

    IF @test THEN
      SELECT "Modifying CodeType table referential actions" AS "";

      SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
      SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

      ALTER TABLE CodeType DROP FOREIGN KEY fkCodeTypeCodeGroupId;

      ALTER TABLE CodeType ADD CONSTRAINT fkCodeTypeCodeGroupId
        FOREIGN KEY (CodeGroupId) REFERENCES CodeGroup (Id)
        ON DELETE NO ACTION ON UPDATE CASCADE;

      SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
      SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
    END IF;

  END //
DELIMITER ;

CALL patch_CodeType();
DROP PROCEDURE IF EXISTS patch_CodeType;
