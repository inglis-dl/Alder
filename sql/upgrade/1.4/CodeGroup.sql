DROP PROCEDURE IF EXISTS patch_CodeGroup;
DELIMITER //
CREATE PROCEDURE patch_CodeGroup()
  BEGIN

    SELECT "Creating new CodeGroup table" AS "";

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLES
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "CodeGroup" );

    IF @test = 0 THEN

      CREATE TABLE IF NOT EXISTS CodeGroup (
        Id INT UNSIGNED NOT NULL AUTO_INCREMENT,
        UpdateTimestamp TIMESTAMP NOT NULL,
        CreateTimestamp TIMESTAMP NOT NULL,
        Value INT NOT NULL DEFAULT 0,
        Name VARCHAR(255) NOT NULL,
        PRIMARY KEY (Id),
        UNIQUE INDEX uqNameValue (Name ASC, Value ASC))
      ENGINE = InnoDB;

    END IF;

    INSERT IGNORE INTO CodeGroup (Value, Name) VALUES
    (-1,"PositionGroup1"),
    (-1,"PositionGroup2"),
    (-1,"Angulation"),
    (-1,"AnalysisGroup1"),
    (-1,"AnalysisGroup2");

  END //
DELIMITER ;

CALL patch_CodeGroup();
DROP PROCEDURE IF EXISTS patch_CodeGroup;
