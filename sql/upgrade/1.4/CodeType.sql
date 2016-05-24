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

    SET @Position_Group1_Id=(SELECT Id FROM CodeGroup WHERE NAME="PositionGroup1");
    SET @Position_Group2_Id=(SELECT Id FROM CodeGroup WHERE NAME="PositionGroup2");
    SET @AngulationId=(SELECT Id FROM CodeGroup WHERE NAME="Angulation");
    SET @Analysis_Group1_Id=(SELECT Id FROM CodeGroup WHERE NAME="AnalysisGroup1");
    SET @Analysis_Group2_Id=(SELECT Id FROM CodeGroup WHERE NAME="AnalysisGroup2");

    INSERT IGNORE INTO CodeType ( Code, Value, CodeGroupId ) VALUES
      ("AI",-1,@Analysis_Group1_Id),
      ("AL",-1,@Analysis_Group1_Id),
      ("AM",-1,@Analysis_Group1_Id),
      ("AM",0,NULL),
      ("AN",-1,NULL),
      ("AR",0,NULL),
      ("ART",-5,NULL),
      ("AS",-1,@Analysis_Group1_Id),
      ("AS",-1,NULL),
      ("CA",-1,@Analysis_Group2_Id),
      ("CB",-1,NULL),
      ("CE",0,NULL),
      ("CS",-1,NULL),
      ("CS",-1,@Analysis_Group2_Id),
      ("FB",-1,@Analysis_Group1_Id),
      ("FC",-1,@Position_Group1_Id),
      ("FI",-1,@Analysis_Group1_Id),
      ("FS",-1,@Position_Group1_Id),
      ("FY",-1,@Analysis_Group1_Id),
      ("Fab",-1,@AngulationId),
      ("Fad",-1,@AngulationId),
      ("GB",0,NULL),
      ("L4",-1,NULL),
      ("LI",-1,@Position_Group1_Id),
      ("LO",-1,NULL),
      ("LS",-1,@Position_Group1_Id),
      ("ME",-4,NULL),
      ("MET",-1,NULL),
      ("MS",-1,NULL),
      ("NA",-5,NULL),
      ("NI",-5,NULL),
      ("NO",-5,NULL),
      ("NU",-5,NULL),
      ("PC",-1,NULL),
      ("PI",0,NULL),
      ("PI",-1,@Position_Group1_Id),
      ("PL",-1,@Position_Group1_Id),
      ("PM",-1,@Position_Group1_Id),
      ("PS",-1,@Position_Group1_Id),
      ("SB",-4,NULL),
      ("SK",-1,NULL),
      ("WC",-1,@Position_Group1_Id),
      ("WFH",-1,@Position_Group1_Id),
      ("WH",-1,@Position_Group2_Id),
      ("WL",-1,@Analysis_Group2_Id),
      ("WN",-1,@Analysis_Group1_Id),
      ("WP",-1,@Analysis_Group2_Id),
      ("WS",-1,@Position_Group1_Id),
      ("WT",-1,@Analysis_Group1_Id),
      ("Wha",-1,@Position_Group2_Id),
      ("Wsh",-1,@Analysis_Group1_Id),
      ("ZE",-5,NULL);

  END //
DELIMITER ;

CALL patch_CodeType();
DROP PROCEDURE IF EXISTS patch_CodeType;
