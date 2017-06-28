DROP PROCEDURE IF EXISTS patch_Interview;
DELIMITER //
CREATE PROCEDURE patch_Interview()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.COLUMNS
      WHERE TABLE_NAME = "Interview"
      AND COLUMN_NAME = "Barcode");

    IF @test = 0 THEN
      SELECT "Updating Interview table with new Barcode column" AS "";

      SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
      SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

      ALTER TABLE Interview
      ADD COLUMN Barcode VARCHAR(45) NOT NULL AFTER VisitDate;

      SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
      SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

    END IF;

  END //
DELIMITER ;

CALL patch_Interview();
DROP PROCEDURE IF EXISTS patch_Interview;
