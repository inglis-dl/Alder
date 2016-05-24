DROP PROCEDURE IF EXISTS patch_ScanTypeHasCodeType;
DELIMITER //
CREATE PROCEDURE patch_ScanTypeHasCodeType()
  BEGIN

    SELECT "Creating new ScanTypeHasCodeType table" AS "";

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLES
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "ScanTypeHasCodeType" );

    IF @test = 0 THEN
      CREATE TABLE IF NOT EXISTS ScanTypeHasCodeType (
        ScanTypeId INT UNSIGNED NOT NULL,
        CodeTypeId INT UNSIGNED NOT NULL,
        UpdateTimestamp TIMESTAMP NOT NULL,
        CreateTimestamp TIMESTAMP NOT NULL,
        PRIMARY KEY (ScanTypeId, CodeTypeId),
        INDEX fkCodeTypeId (CodeTypeId ASC),
        INDEX fkScanTypeId (ScanTypeId ASC),
        UNIQUE INDEX uqCodeTypeIdScanTypeId (CodeTypeId ASC, ScanTypeId ASC),
        CONSTRAINT fkScanTypeHasCodeTypeScanTypeId
          FOREIGN KEY (ScanTypeId)
          REFERENCES ScanType (Id)
          ON DELETE NO ACTION
          ON UPDATE NO ACTION,
        CONSTRAINT fkScanTypeHasCodeTypeCodeTypeId
          FOREIGN KEY (CodeTypeId)
          REFERENCES CodeType (Id)
          ON DELETE NO ACTION
          ON UPDATE NO ACTION)
      ENGINE = InnoDB;
    END IF;

    SET @HipTypeId=(SELECT Id FROM ScanType WHERE Type="DualHipBoneDensity");

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @HipTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code IN ("PL","PM","PS","PI","Fad","Fab","AM","AL","AI","CA","MS","MET")
    AND CodeType.Value=-1;

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @HipTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code IN ("ART","NU")
    AND CodeType.Value=-5;

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @HipTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code IN ("AS","CS")
    AND CodeType.Value=-1
    AND CodeType.CodeGroupId IS NOT NULL;

    SET @ForearmTypeId=(SELECT Id FROM ScanType WHERE Type="ForearmBoneDensity");

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @ForearmTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code IN ("FS","FC","CB","FY","FB","FI","MS","MET")
    AND CodeType.Value=-1;

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @ForearmTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code IN ("ART","NU")
    AND CodeType.Value=-5;

    SET @LateralTypeId=(SELECT Id FROM ScanType WHERE Type="LateralBoneDensity");

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @LateralTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code IN ("LI","LS","L4","MS","MET")
    AND CodeType.Value=-1;

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @LateralTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code IN ("ART","NU")
    AND CodeType.Value=-5;

    SET @WholeBodyTypeId=(SELECT Id FROM ScanType WHERE Type="WholeBodyBoneDensity");

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @WholeBodyTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code IN ("WS","WC","Wha","WH","WN","WT","Wsh","WFH","WP","WL","MS","MET")
    AND CodeType.Value=-1;

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @WholeBodyTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code IN ("ART","NU")
    AND CodeType.Value=-5;

    SET @CarotidIntimaTypeId=(SELECT Id FROM ScanType WHERE Type="CarotidIntima");

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @CarotidIntimaTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code IN ("AR","AS","NA","NO","AN","CE","CS","PC","NI","SB","SK","GB","LO","ME","ZE")
    AND CodeType.CodeGroupId IS NULL;

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @CarotidIntimaTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code ="PI" AND CodeType.Value=0;

    INSERT IGNORE INTO ScanTypeHasCodeType (ScanTypeId, CodeTypeId)
    SELECT @CarotidIntimaTypeId AS ScanTypeId, CodeType.Id AS CodeTypeId
    FROM CodeType
    WHERE CodeType.Code ="AM" AND CodeType.Value=0;

  END //
DELIMITER ;

CALL patch_ScanTypeHasCodeType();
DROP PROCEDURE IF EXISTS patch_ScanTypeHasCodeType;
