DROP PROCEDURE IF EXISTS patch_Wave;
DELIMITER //
CREATE PROCEDURE patch_Wave()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLES
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Wave" );

    IF @test = 0 THEN
      SELECT "Creating new Wave table" AS "";

      CREATE TABLE IF NOT EXISTS Wave (
        Id INT UNSIGNED NOT NULL AUTO_INCREMENT,
        UpdateTimestamp TIMESTAMP NOT NULL,
        CreateTimestamp TIMESTAMP NOT NULL,
        Name VARCHAR(45) NOT NULL,
        Rank INT UNSIGNED NOT NULL,
        MetaDataSource VARCHAR(45) NOT NULL,
        ImageDataSource VARCHAR(45) NOT NULL,
        PRIMARY KEY (Id),
        UNIQUE INDEX uqNameRank (Name ASC, Rank ASC))
      ENGINE = InnoDB;

      INSERT INTO Wave ( Name, Rank, MetaDataSource, ImageDataSource )
      VALUES
        ("Baseline",1,"alder","clsa-dcs-images"),
        ("Followup1",2,"alder_f1","clsa-dcs-images-f1");
    END IF;

  END //
DELIMITER ;

CALL patch_Wave();
DROP PROCEDURE IF EXISTS patch_Wave;
