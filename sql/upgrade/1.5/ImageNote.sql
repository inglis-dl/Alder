DROP PROCEDURE IF EXISTS patch_ImageNote;
DELIMITER //
CREATE PROCEDURE patch_ImageNote()
  BEGIN

    SET @test = (
      SELECT COUNT(*)
      FROM information_schema.TABLES
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "ImageNote" );

    IF @test = 0 THEN
      SELECT "Creating new ImageNote table" AS "";

      CREATE TABLE IF NOT EXISTS ImageNote (
        Id INT UNSIGNED NOT NULL AUTO_INCREMENT,
        UpdateTimestamp TIMESTAMP NOT NULL,
        CreateTimestamp TIMESTAMP NOT NULL,
        ImageId INT UNSIGNED NOT NULL,
        UserId INT UNSIGNED NOT NULL,
        Note TEXT NOT NULL,
        PRIMARY KEY (Id),
        INDEX fkUserId (UserId ASC),
        INDEX fkImageId (ImageId ASC),
        CONSTRAINT fkNoteUserId
        FOREIGN KEY (UserId)
        REFERENCES User (Id)
        ON DELETE NO ACTION
        ON UPDATE CASCADE,
        CONSTRAINT fkNoteImageId
        FOREIGN KEY (ImageId)
        REFERENCES Image (Id)
        ON DELETE NO ACTION
        ON UPDATE CASCADE)
      ENGINE = InnoDB;

      CREATE TEMPORARY TABLE tmp AS
      SELECT
      u.Id AS UserId,
      i.Id AS ImageId,
      i.Note,
      DATE(r.CreateTimestamp) AS ActionDate
      FROM Image i
      JOIN Rating r ON r.ImageId=i.Id
      JOIN User u ON u.Id=r.UserId
      WHERE i.Note IS NOT NULL
      ORDER BY ImageId, ActionDate;

      CREATE TEMPORARY TABLE tmp2 as
      SELECT DISTINCT ImageId, min(ActionDate) AS MinActionDate
      FROM tmp GROUP BY ImageId;

      CREATE TEMPORARY TABLE tmp3 as
      SELECT DISTINCT tmp.UserId, tmp.ImageId, tmp.Note
      FROM tmp
      INNER JOIN tmp2 ON tmp2.ImageId=tmp.ImageId
      AND tmp2.MinActionDate=tmp.ActionDate;

      INSERT INTO ImageNote
      (UserId, ImageId, Note)
      SELECT
      UserId, ImageId, Note
      FROM tmp3;

      DROP TABLE tmp;

      DROP TABLE tmp2;

      DROP TABLE tmp3;

      CREATE TEMPORARY TABLE tmp AS
      SELECT
      i.Id AS ImageId,
      i.Note,
      DATE(i.CreateTimestamp) AS ImageCreateDate,
      m.Name AS Modality,
      u.Id AS UserId,
      DATE(u.CreateTimestamp) AS UserCreateDate
      FROM Image i
      LEFT JOIN Rating r ON r.ImageId=i.Id
      JOIN Exam e ON e.Id=i.ExamId
      JOIN ScanType s ON s.Id=e.ScanTypeId
      JOIN Modality m ON m.Id=s.ModalityId
      JOIN UserHasModality uhm ON uhm.ModalityId=m.Id
      JOIN User u ON u.Id=uhm.UserId
      WHERE i.Note IS NOT NULL
      AND r.Id IS NULL
      ORDER BY ImageId, ImageCreateDate, UserCreateDate;

      DELETE tmp.*
      FROM tmp
      WHERE Modality = 'Dexa'
      AND UserId!=(SELECT Id FROM User WHERE Name='jackie');

      DELETE tmp.*
      FROM tmp
      WHERE UserCreateDate < ImageCreateDate
      AND Modality='Ultrasound';

      INSERT INTO ImageNote
      (UserId, ImageId, Note)
      SELECT
      UserId, ImageId, Note
      FROM tmp;

      DROP TABLE tmp;

    END IF;

  END //
DELIMITER ;

CALL patch_ImageNote();
DROP PROCEDURE IF EXISTS patch_ImageNote;
