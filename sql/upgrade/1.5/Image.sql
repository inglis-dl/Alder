DROP PROCEDURE IF EXISTS patch_Image;
DELIMITER //
CREATE PROCEDURE patch_Image()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Image"
      AND COLUMN_NAME = "Note");

    IF @test THEN

      SET @test_2 = (
        SELECT COUNT(*)
        FROM information_schema.TABLES
        WHERE TABLE_SCHEMA = DATABASE()
        AND TABLE_NAME = "ImageNote");

      IF @test_2 THEN
        SELECT "Removing Note column from Image table" AS "";

        ALTER TABLE Image DROP COLUMN Note;
      END IF;

    END IF;

    SELECT "Modifying Image table referential actions and modifying Dimensionality column" AS "";

    SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
    SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

    ALTER TABLE Image DROP FOREIGN KEY fkImageExamId;

    ALTER TABLE Image ADD CONSTRAINT fkImageExamId
      FOREIGN KEY (ExamId) REFERENCES Exam (Id)
      ON DELETE NO ACTION ON UPDATE CASCADE;

    ALTER TABLE Image DROP FOREIGN KEY fkImageParentImageId;

    ALTER TABLE Image ADD CONSTRAINT fkImageParentImageId
      FOREIGN KEY (ParentImageId) REFERENCES Image (Id)
      ON DELETE NO ACTION ON UPDATE CASCADE;

    SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
    SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

    ALTER TABLE Image MODIFY COLUMN Dimensionality INT NOT NULL DEFAULT 2;
  END //
DELIMITER ;

CALL patch_Image();
DROP PROCEDURE IF EXISTS patch_Image;
