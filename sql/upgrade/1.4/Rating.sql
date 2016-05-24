DROP PROCEDURE IF EXISTS patch_Rating;
DELIMITER //
CREATE PROCEDURE patch_Rating()
  BEGIN

    SELECT "Updating Rating table with new DerivedRating column" AS "";

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Rating"
      AND COLUMN_NAME = "DerivedRating" );

    IF @test = 0 THEN

      ALTER TABLE Rating
      ADD COLUMN DerivedRating TINYINT(1) NULL DEFAULT NULL AFTER Rating;

    END IF;

    UPDATE Rating
    LEFT JOIN Image ON Image.Id=Rating.ImageId 
    LEFT JOIN Exam ON Exam.Id=Image.ExamId 
    LEFT JOIN Interview ON Interview.Id=Exam.InterviewId 
    SET Rating.Rating=1
    WHERE Rating.Rating IS NULL
    AND Image.Note IS NOT NULL;

  END //
DELIMITER ;

CALL patch_Rating();
DROP PROCEDURE IF EXISTS patch_Rating;
