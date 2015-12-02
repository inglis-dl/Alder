DROP PROCEDURE IF EXISTS patch_WaveHasScanType;
DELIMITER //
CREATE PROCEDURE patch_WaveHasScanType()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLES
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "WaveHasScanType" );

    IF @test = 0 THEN
      SELECT "Creating new WaveHasScanType table" AS "";

      CREATE TABLE IF NOT EXISTS WaveHasScanType (
        WaveId INT UNSIGNED NOT NULL,
        ScanTypeId INT UNSIGNED NOT NULL,
        UpdateTimestamp TIMESTAMP NOT NULL,
        CreateTimestamp TIMESTAMP NOT NULL,
        PRIMARY KEY (WaveId, ScanTypeId),
        INDEX fkScanTypeId (ScanTypeId ASC),
        INDEX fkWaveId (WaveId ASC),
        CONSTRAINT fkWaveHasScanTypeWaveId
          FOREIGN KEY (WaveId)
          REFERENCES Wave (Id)
          ON DELETE CASCADE
          ON UPDATE CASCADE,
        CONSTRAINT fkWaveHasScanTypeScanTypeId
          FOREIGN KEY (ScanTypeId)
          REFERENCES ScanType (Id)
          ON DELETE CASCADE
          ON UPDATE CASCADE)
      ENGINE = InnoDB;

      SET @wave_1=(SELECT Id FROM Wave WHERE Rank=1);
      SET @wave_2=(SELECT Id FROM Wave WHERE Rank=2);
      
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_1, Id FROM ScanType WHERE Type="DualHipBoneDensity";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_1, Id FROM ScanType WHERE Type="ForearmBoneDensity";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_1, Id FROM ScanType WHERE Type="LateralBoneDensity";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_1, Id FROM ScanType WHERE Type="WholeBodyBoneDensity";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_1, Id FROM ScanType WHERE Type="RetinalScan";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_1, Id FROM ScanType WHERE Type="CarotidIntima";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_2, Id FROM ScanType WHERE Type="DualHipBoneDensity";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_2, Id FROM ScanType WHERE Type="ForearmBoneDensity";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_2, Id FROM ScanType WHERE Type="LateralBoneDensity";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_2, Id FROM ScanType WHERE Type="WholeBodyBoneDensity";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_2, Id FROM ScanType WHERE Type="SpineBoneDensity";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_2, Id FROM ScanType WHERE Type="RetinalScanRight";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_2, Id FROM ScanType WHERE Type="RetinalScanLeft";
      INSERT INTO WaveHasScanType ( WaveId, ScanTypeId )
      SELECT @wave_2, Id FROM ScanType WHERE Type="CarotidIntima";
    END IF;

  END //
DELIMITER ;

CALL patch_WaveHasScanType();
DROP PROCEDURE IF EXISTS patch_WaveHasScanType;
