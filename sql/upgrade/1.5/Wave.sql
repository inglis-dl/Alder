DROP PROCEDURE IF EXISTS patch_Wave;
DELIMITER //
CREATE PROCEDURE patch_Wave()
  BEGIN

    SELECT "Creating new Wave table" AS "";

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLES
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Wave" );

    IF @test = 0 THEN
      CREATE TABLE IF NOT EXISTS Wave (
        Id INT UNSIGNED NOT NULL AUTO_INCREMENT,
        UpdateTimestamp TIMESTAMP NOT NULL,
        CreateTimestamp TIMESTAMP NOT NULL,
        Name VARCHAR(45) NOT NULL,
        Rank INT UNSIGNED NOT NULL,
        PRIMARY KEY (Id),
        UNIQUE INDEX uqNameRank (Name ASC, Rank ASC),
      ENGINE = InnoDB;

      INSERT INTO Wave ( Name, Rank )
      VALUES ("Baseline",1),("Followup1",2);
    END IF;

  END //
DELIMITER ;

CALL patch_Wave();
DROP PROCEDURE IF EXISTS patch_Wave;
