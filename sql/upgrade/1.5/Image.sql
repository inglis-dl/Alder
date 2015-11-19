DROP PROCEDURE IF EXISTS patch_Image;
DELIMITER //
CREATE PROCEDURE patch_Image()
  BEGIN

    SELECT "Removing Note column from Image table" AS "";

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Image"
      AND COLUMN_NAME = "Note");

    IF @test = 1 THEN

      SET @test_2 = (
        SELECT COUNT(*)
        FROM information_schema.TABLES
        WHERE TABLE_SCHEMA = DATABASE()
        AND TABLE_NAME = "ImageNote");

      IF @test_2 THEN

        ALTER TABLE Image DROP COLUMN Note;

      END IF;

    END IF;

  END //
DELIMITER ;

CALL patch_Image();
DROP PROCEDURE IF EXISTS patch_Image;
