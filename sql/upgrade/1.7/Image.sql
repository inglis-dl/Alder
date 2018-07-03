DROP PROCEDURE IF EXISTS patch_Image;
DELIMITER //
CREATE PROCEDURE patch_Image()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.COLUMNS
      WHERE TABLE_NAME = "Image"
      AND COLUMN_NAME = "Orphan");

    IF @test = 0 THEN
      SELECT "Updating Image table with new Orphan column" AS "";

      SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
      SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

      ALTER TABLE Image
      ADD COLUMN Orphan TINYINT(1) NOT NULL DEFAULT 0 AFTER Dimensionality;

      SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
      SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

      UPDATE Image SET Orphan = 0 WHERE ParentImageId IS NOT NULL;

    END IF;

  END //
DELIMITER ;

CALL patch_Image();
DROP PROCEDURE IF EXISTS patch_Image;