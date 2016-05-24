DROP PROCEDURE IF EXISTS patch_Site;
DELIMITER //
CREATE PROCEDURE patch_Site()
  BEGIN

    SELECT "Creating new Site table" AS "";

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLES
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Site" );

    IF @test = 0 THEN
      CREATE TABLE IF NOT EXISTS Site (
        Id INT UNSIGNED NOT NULL AUTO_INCREMENT,
        UpdateTimestamp TIMESTAMP NOT NULL,
        CreateTimestamp TIMESTAMP NOT NULL,
        Name VARCHAR(45) NOT NULL,
        Alias VARCHAR(45) NULL DEFAULT NULL,
        PRIMARY KEY (Id))
      ENGINE = InnoDB;

      INSERT INTO Site ( Name, Alias )
      VALUES
      ("Memorial",NULL),
      ("Victoria",NULL),
      ("Calgary",NULL),
      ("Sherbrooke",NULL),
      ("Hamilton","McMaster"),
      ("Simon Fraser",NULL),
      ("Ottawa",NULL),
      ("Dalhousie",NULL),
      ("Manitoba",NULL),
      ("McGill",NULL),
      ("British Columbia",NULL);

    END IF;

  END //
DELIMITER ;

CALL patch_Site();
DROP PROCEDURE IF EXISTS patch_Site;
