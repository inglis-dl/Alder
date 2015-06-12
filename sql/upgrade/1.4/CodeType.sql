DROP PROCEDURE IF EXISTS patch_CodeType;
DELIMITER //
CREATE PROCEDURE patch_CodeType()
  BEGIN

    SELECT "Creating new CodeType table" AS "";

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLES
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "CodeType" );

    IF @test = 0 THEN
      CREATE TABLE IF NOT EXISTS CodeType (
        Id INT UNSIGNED NOT NULL AUTO_INCREMENT,
        UpdateTimestamp TIMESTAMP NOT NULL,
        CreateTimestamp TIMESTAMP NOT NULL,
        CodeGroupId INT UNSIGNED NULL DEFAULT NULL,
        Code VARCHAR(45) NOT NULL,
        Value INT NOT NULL DEFAULT 0,
        Active TINYINT(1) NOT NULL DEFAULT 1,
        PRIMARY KEY (Id),
        UNIQUE INDEX uqCodeGroupIdCodeValue (CodeGroupId ASC, Value ASC, Code ASC),
        INDEX fkCodeGroupId (CodeGroupId ASC),
        CONSTRAINT fkCodeTypeCodeGroupId
          FOREIGN KEY (CodeGroupId)
          REFERENCES CodeGroup (Id)
          ON DELETE NO ACTION
          ON UPDATE NO ACTION)
      ENGINE = InnoDB;
    END IF;

    SET @PositionId=(SELECT Id FROM CodeGroup WHERE NAME="Position");
    SET @AngulationId=(SELECT Id FROM CodeGroup WHERE NAME="Angulation");
    SET @AnalysisRegion1Id=(SELECT Id FROM CodeGroup WHERE NAME="AnalysisRegion1");
    SET @AnalysisRegion2Id=(SELECT Id FROM CodeGroup WHERE NAME="AnalysisRegion2");

    INSERT IGNORE INTO CodeType ( Code, Value, CodeGroupId ) VALUES
      ("AI",-1,@AnalysisRegion1Id),
      ("AL",-1,@AnalysisRegion1Id),
      ("AM",-1,@AnalysisRegion1Id),
      ("AM",0,NULL),
      ("AN",-1,NULL),
      ("AR",0,NULL),
      ("AS",-1,@AnalysisRegion1Id),
      ("AS",-1,NULL),
      ("CA",-1,@AnalysisRegion2Id),
      ("CB",-1,NULL),
      ("CE",0,NULL),
      ("CS",-1,NULL),
      ("CS",-1,@AnalysisRegion2Id),
      ("FB",-1,@AnalysisRegion1Id),
      ("FC",-1,@PositionId),
      ("FI",-1,@AnalysisRegion1Id),
      ("FS",-1,@PositionId),
      ("FY",-1,@AnalysisRegion1Id),
      ("Fab",-1,@AngulationId),
      ("Fad",-1,@AngulationId),
      ("GB",0,NULL),
      ("JW",-1,NULL),
      ("L4",-1,NULL),
      ("LI",-1,@AnalysisRegion1Id),
      ("LO",-1,NULL),
      ("LS",-1,@AnalysisRegion1Id),
      ("ME",-4,NULL),
      ("MS",-1,NULL),
      ("NA",-5,NULL),
      ("NI",-5,NULL),
      ("NO",-5,NULL),
      ("PC",-1,NULL),
      ("PI",0,NULL),
      ("PI",-1,@PositionId),
      ("PL",-1,@PositionId),
      ("PM",-1,@PositionId),
      ("PS",-1,@PositionId),
      ("SB",-4,NULL),
      ("SK",-1,NULL),
      ("WC",-1,@PositionId),
      ("WH",-1,@PositionId),
      ("WL",-1,@AnalysisRegion1Id),
      ("WN",-1,@AnalysisRegion1Id),
      ("WP",-1,@AnalysisRegion1Id),
      ("WS",-1,@PositionId),
      ("WT",-1,@AnalysisRegion1Id),
      ("Wha",-1,@PositionId),
      ("Wsh",-1,@AnalysisRegion1Id),
      ("ZE",-5,NULL);

  END //
DELIMITER ;

CALL patch_CodeType();
DROP PROCEDURE IF EXISTS patch_CodeType;
