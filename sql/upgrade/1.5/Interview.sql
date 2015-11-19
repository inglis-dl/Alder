DROP PROCEDURE IF EXISTS patch_Interview;
DELIMITER //
CREATE PROCEDURE patch_Interview()
  BEGIN

    SELECT "Updating Interview table with new columns" AS "";

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLE_CONSTRAINTS
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Interview"
      AND CONSTRAINT_NAME IN ("fkInterviewWaveId","fkInterviewSiteId"));

    IF @test = 0 THEN

      SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
      SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;

      ALTER TABLE Interview
      ADD COLUMN WaveId INT UNSIGNED NOT NULL AFTER CreateTimestamp;

      UPDATE Interview
      SET WaveId = ( SELECT Id FROM Wave WHERE Name="Baseline" AND RANK=1 );

      ALTER TABLE Interview
      ADD INDEX fkWaveId ( WaveId ASC ),
      ADD CONSTRAINT fkInterviewWaveId
      FOREIGN KEY ( WaveId ) REFERENCES Wave (Id)
      ON DELETE NO ACTION
      ON UPDATE NO ACTION;

      ALTER TABLE Interview
      ADD COLUMN SiteId INT UNSIGNED NOT NULL AFTER WaveId;

      UPDATE Interview
      JOIN Site ON Site.Name=Interview.Site
      SET Interview.SiteId=Site.Id;

      ALTER TABLE Interview DROP COLUMN Site;

      ALTER TABLE Interview
      ADD INDEX fkSiteId ( SiteId ASC ),
      ADD CONSTRAINT fkInterviewSiteId
      FOREIGN KEY ( SiteId ) REFERENCES Site (Id)
      ON DELETE NO ACTION
      ON UPDATE NO ACTION;

      ALTER TABLE Interview
      DROP INDEX uqUIdVisitDate;

      ALTER TABLE Interview
      ADD UNIQUE INDEX
      uqUIdWaveIdVisitDate ( UID ASC, WaveId ASC, VisitDate ASC );

      SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
      SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

    END IF;

  END //
DELIMITER ;

CALL patch_Interview();
DROP PROCEDURE IF EXISTS patch_Interview;
