DROP PROCEDURE IF EXISTS patch_UserHasModality;
DELIMITER //
CREATE PROCEDURE patch_UserHasModality()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLE_CONSTRAINTS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "UserHasModality"
      AND CONSTRAINT_NAME = "uqUserIdModalityId");

    IF @test THEN
      SELECT "Removing unique index from UserHasModality table" AS "";

      ALTER TABLE UserHasModality DROP INDEX uqUserIdModalityId;
    END IF;

  END //
DELIMITER ;

CALL patch_UserHasModality();
DROP PROCEDURE IF EXISTS patch_UserHasModality;
