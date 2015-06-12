DROP PROCEDURE IF EXISTS patch_ScanType;
DELIMITER //
CREATE PROCEDURE patch_ScanType()
  BEGIN

    SELECT "Creating new ScanType table" AS ""; 

    SET @test = ( 
      SELECT COUNT(*)
      FROM information_schema.TABLES
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "ScanType" );

    IF @test = 0 THEN
      CREATE TABLE IF NOT EXISTS ScanType ( 
        Id INT UNSIGNED NOT NULL AUTO_INCREMENT,
        UpdateTimestamp TIMESTAMP NOT NULL,
        CreateTimestamp TIMESTAMP NOT NULL,
        ModalityId INT UNSIGNED NOT NULL,
        Type VARCHAR(255) NOT NULL,
        PRIMARY KEY (Id),
        UNIQUE INDEX uqModalityIdType (ModalityId ASC, Type ASC),
        INDEX fkModalityId (ModalityId ASC),
        CONSTRAINT fkScanTypeModalityId
          FOREIGN KEY (ModalityId)
          REFERENCES Modality (Id)
          ON DELETE NO ACTION
          ON UPDATE NO ACTION)
      ENGINE = InnoDB;

      INSERT INTO ScanType ( ModalityId, Type )
      SELECT DISTINCT ModalityId, Type FROM Exam;
    END IF; 

  END //
DELIMITER ;

CALL patch_ScanType();
DROP PROCEDURE IF EXISTS patch_ScanType;
