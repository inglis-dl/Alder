DROP PROCEDURE IF EXISTS patch_Code;
DELIMITER //
CREATE PROCEDURE patch_Code()
  BEGIN

    SELECT "Creating new Code table" AS ""; 

    SET @test = ( 
      SELECT COUNT(*)
      FROM information_schema.TABLES
      WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = "Code" );

    IF @test = 0 THEN

      CREATE TABLE IF NOT EXISTS Code ( 
        Id INT UNSIGNED NOT NULL AUTO_INCREMENT,
        UpdateTimestamp TIMESTAMP NOT NULL,
        CreateTimestamp TIMESTAMP NOT NULL,
        ImageId INT UNSIGNED NOT NULL,
        UserId INT UNSIGNED NOT NULL,
        CodeTypeId INT UNSIGNED NOT NULL,
        PRIMARY KEY (Id),
        INDEX fkImageId (ImageId ASC),
        INDEX fkUserId (UserId ASC),
        UNIQUE INDEX uqCodeTypeIdImageIdUserId (CodeTypeId ASC, ImageId ASC, UserId ASC),
        INDEX fkCodeTypeId (CodeTypeId ASC),
        CONSTRAINT fkCodeImageId
          FOREIGN KEY (ImageId)
          REFERENCES Image (Id)
          ON DELETE CASCADE
          ON UPDATE CASCADE,
        CONSTRAINT fkCodeUserId
          FOREIGN KEY (UserId)
          REFERENCES User (Id)
          ON DELETE CASCADE
          ON UPDATE CASCADE,
        CONSTRAINT fkCodeCodeTypeId
          FOREIGN KEY (CodeTypeId)
          REFERENCES CodeType (Id)
          ON DELETE NO ACTION
          ON UPDATE NO ACTION)
      ENGINE = InnoDB;

    END IF;

    CREATE TEMPORARY TABLE tmp AS
    SELECT
    CONCAT(REPLACE(REPLACE(TRIM(TRAILING ',' FROM Note),', ',','),' ',''),',') AS Note,
    Rating,
    5 AS DerivedRating,
    Type,
    UserId,
    ImageId,
    RatingId,
    0 AS ARcode,
    0 AS AScode,
    0 AS NAcode,
    0 AS NOcode,
    0 AS PIcode,
    0 AS ANcode,
    0 AS CEcode,
    0 AS CScode,
    0 AS PCcode,
    0 AS NIcode,
    0 AS SBcode,
    0 AS SKcode,
    0 AS GBcode,
    0 AS LOcode,
    0 AS AMcode,
    0 AS MEcode,
    0 AS ZEcode
    FROM (
      SELECT
      TRIM(REPLACE(REPLACE(Note,'\n',','),',,',',')) AS Note,
      Rating,
      UserId,
      ImageId,
      Rating.Id AS RatingId,
      Type
      FROM Image
      JOIN Rating ON Rating.ImageId=Image.Id
      JOIN Exam ON Exam.Id=Image.ExamId
      JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
      WHERE Note REGEXP '(AR)|(AS)|(NA)|(NO)|(PI)|(AN)|(CE)|(CS)|(PC)|(NI)|(SB)|(SK)|(GB)|(LO)|(AM)|(ME)|(ZE)'
      AND Type='CarotidIntima'
    ) x;

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), AScode=1
    WHERE Note REGEXP 'AS,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-5), NAcode=1
    WHERE Note REGEXP 'NA,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-5), NOcode=1
    WHERE Note REGEXP 'NO,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), ANcode=1
    WHERE Note REGEXP 'AN,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), CScode=1
    WHERE Note REGEXP 'CS,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), PCcode=1
    WHERE Note REGEXP 'PC,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-5), NIcode=1
    WHERE Note REGEXP 'NI,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-4), SBcode=1
    WHERE Note REGEXP 'SB,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), SKcode=1
    WHERE Note REGEXP 'SK,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), LOcode=1
    WHERE Note REGEXP 'LO,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-4), MEcode=1
    WHERE Note REGEXP 'ME,';

    UPDATE tmp
    SET Note = CONCAT_WS(',',Note,'ZE,')
    WHERE DerivedRating<1
    AND Note NOT REGEXP 'ZE,';

    UPDATE tmp
    SET DerivedRating=1
    WHERE DerivedRating<1
    OR Note REGEXP 'ZE,';

    UPDATE tmp
    SET ZEcode=1
    WHERE Note REGEXP 'ZE,';

    UPDATE tmp
    SET ARcode=1
    WHERE Note REGEXP 'AR,';

    UPDATE tmp
    SET PIcode=1
    WHERE Note REGEXP 'PI,';

    UPDATE tmp
    SET CEcode=1
    WHERE Note REGEXP 'CE,';

    UPDATE tmp
    SET GBcode=1
    WHERE Note REGEXP 'GB,';

    UPDATE tmp
    SET AMcode=1
    WHERE Note REGEXP 'AM,';

    UPDATE Rating
    JOIN tmp ON tmp.RatingId=Rating.Id
    SET Rating.DerivedRating=tmp.DerivedRating;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='AR' AND Value=0 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE ARcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='AS' AND Value=-1 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE AScode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='NA' AND Value=-5 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE NAcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='NO' AND Value=-5 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE NOcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='PI' AND Value=0 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE PIcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='AN' AND Value=-1 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE ANcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='CE' AND Value=0 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE CEcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='CS' AND Value=-1 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE CScode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='PC' AND Value=-1 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE PCcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='NI' AND Value=-5 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE NIcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='SB' AND Value=-4 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE SBcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='GB' AND Value=0 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE GBcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='LO' AND Value=-1 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE LOcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='AM' AND Value=0 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE AMcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='ME' AND Value=-4 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE MEcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='ZE' AND Value=-5 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE ZEcode=1;

    DROP TABLE tmp;

    CREATE TEMPORARY TABLE tmp AS
    SELECT
    CONCAT(REPLACE(REPLACE(TRIM(TRAILING ',' FROM Note),', ',','),' ',''),',') AS Note,
    Rating,
    5 AS DerivedRating,
    Type,
    UserId,
    ImageId,
    RatingId,
    0 AS WScode,
    0 AS WCcode,
    0 AS Whacode,
    0 AS WHcode,
    0 AS WNcode,
    0 AS WTcode,
    0 AS Wshcode,
    0 AS WPcode,
    0 AS WLcode,
    0 AS JWcode,
    0 AS MScode
    FROM (
      SELECT
      TRIM(REPLACE(REPLACE(Note,'\n',','),',,',',')) AS Note,
      Rating,
      UserId,
      ImageId,
      Rating.Id AS RatingId,
      Type
      FROM Image
      JOIN Rating ON Rating.ImageId=Image.Id
      JOIN Exam ON Exam.Id=Image.ExamId
      JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
      WHERE Note REGEXP '(WS)|(WC)|(Wha)|(WH)|(WN)|(WT)|(Wsh)|(WP)|(WL)|(JW)|(MS)'
      AND Type='WholeBodyBoneDensity'
    ) x;

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1)
    WHERE Note REGEXP '(WS,)|(WC,)|(Wha,)|(WH,)';

    UPDATE tmp
    SET WScode=1
    WHERE Note REGEXP 'WS,';

    UPDATE tmp
    SET WCcode=1
    WHERE Note REGEXP 'WC,';

    UPDATE tmp
    SET Whacode=1
    WHERE Note REGEXP 'Wha,';

    UPDATE tmp
    SET WHcode=1
    WHERE Note REGEXP 'WH,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1)
    WHERE Note REGEXP '(WN,)|(WT,)|(Wsh,)|(WP,)|(WL,)';

    UPDATE tmp
    SET WNcode=1
    WHERE Note REGEXP 'WN,';

    UPDATE tmp
    SET WTcode=1
    WHERE Note REGEXP 'WT,';

    UPDATE tmp
    SET Wshcode=1
    WHERE Note REGEXP 'Wsh,';

    UPDATE tmp
    SET WPcode=1
    WHERE Note REGEXP 'WP,';

    UPDATE tmp
    SET WLcode=1
    WHERE Note REGEXP 'WL,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), JWcode=1
    WHERE Note REGEXP 'JW,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), MScode=1
    WHERE Note REGEXP 'MS,';

    UPDATE tmp
    SET DerivedRating=1
    WHERE DerivedRating<1;

    UPDATE Rating
    JOIN tmp ON tmp.RatingId=Rating.Id
    SET Rating.DerivedRating=tmp.DerivedRating;

    SET @Position_Group1_Id=(SELECT Id FROM CodeGroup WHERE NAME='PositionGroup1');
    SET @Position_Group2_Id=(SELECT Id FROM CodeGroup WHERE NAME='PositionGroup2');
    SET @AngulationId=(SELECT Id FROM CodeGroup WHERE NAME='Angulation');
    SET @Analysis_Group1_Id=(SELECT Id FROM CodeGroup WHERE NAME='AnalysisGroup1');
    SET @Analysis_Group2_Id=(SELECT Id FROM CodeGroup WHERE NAME='AnalysisGroup2');

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='WS' AND Value=-1 AND CodeGroupId=@Position_Group1_Id)
    FROM tmp
    WHERE WScode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='WC' AND Value=-1 AND CodeGroupId=@Position_Group1_Id)
    FROM tmp
    WHERE WCcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='Wha' AND Value=-1 AND CodeGroupId=@Position_Group2_Id)
    FROM tmp
    WHERE Whacode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='WH' AND Value=-1 AND CodeGroupId=@Position_Group2_Id)
    FROM tmp
    WHERE WHcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='WN' AND Value=-1 AND CodeGroupId=@Analysis_Group1_Id)
    FROM tmp
    WHERE WNcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='WT' AND Value=-1 AND CodeGroupId=@Analysis_Group1_Id)
    FROM tmp
    WHERE WTcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='Wsh' AND Value=-1 AND CodeGroupId=@Analysis_Group1_Id)
    FROM tmp
    WHERE Wshcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='WP' AND Value=-1 AND CodeGroupId=@Analysis_Group2_Id)
    FROM tmp
    WHERE WPcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='WL' AND Value=-1 AND CodeGroupId=@Analysis_Group2_Id)
    FROM tmp
    WHERE WLcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='MET' AND Value=-1 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE JWcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='MS' AND Value=-1 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE MScode=1;

    DROP TABLE tmp;

    CREATE TEMPORARY TABLE tmp AS
    SELECT
    CONCAT(REPLACE(REPLACE(TRIM(TRAILING ',' FROM Note),', ',','),' ',''),',') AS Note,
    Rating,
    5 AS DerivedRating,
    Type,
    UserId,
    ImageId,
    RatingId,
    0 AS FScode,
    0 AS FCcode,
    0 AS CBcode,
    0 AS FYcode,
    0 AS FBcode,
    0 AS FIcode
    FROM (
      SELECT
      TRIM(REPLACE(REPLACE(Note,'\n',','),',,',',')) AS Note,
      Rating,
      UserId,
      ImageId,
      Rating.Id AS RatingId,
      Type
      FROM Image
      JOIN Rating ON Rating.ImageId=Image.Id
      JOIN Exam ON Exam.Id=Image.ExamId
      JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
      WHERE Note REGEXP '(FS)|(FC)|(CB)|(FY)|(FB)|(FI)'
      AND Type='ForearmBoneDensity'
    ) x;

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1)
    WHERE Note REGEXP '(FS,)|(FC,)';

    UPDATE tmp
    SET FScode=1
    WHERE Note REGEXP 'FS,';

    UPDATE tmp
    SET FCcode=1
    WHERE Note REGEXP 'FC,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), CBcode=1
    WHERE Note REGEXP 'CB,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1)
    WHERE Note REGEXP '(FY,)|(FB,)|(FI,)';

    UPDATE tmp
    SET FYcode=1
    WHERE Note REGEXP 'FY,';

    UPDATE tmp
    SET FBcode=1
    WHERE Note REGEXP 'FB,';

    UPDATE tmp
    SET FIcode=1
    WHERE Note REGEXP 'FI,';

    UPDATE tmp
    SET DerivedRating=1
    WHERE DerivedRating < 1;

    UPDATE Rating
    JOIN tmp ON tmp.RatingId=Rating.Id
    SET Rating.DerivedRating=tmp.DerivedRating;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='FS' AND Value=-1 AND CodeGroupId=@Position_Group1_Id)
    FROM tmp
    WHERE FScode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='FC' AND Value=-1 AND CodeGroupId=@Position_Group1_Id)
    FROM tmp
    WHERE FCcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='CB' AND Value=-1 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE CBcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='FB' AND Value=-1 AND CodeGroupId=@Analysis_Group1_Id)
    FROM tmp
    WHERE FBcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='FY' AND Value=-1 AND CodeGroupId=@Analysis_Group1_Id)
    FROM tmp
    WHERE FYcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='FI' AND Value=-1 AND CodeGroupId=@Analysis_Group1_Id)
    FROM tmp
    WHERE FIcode=1;

    DROP TABLE tmp;

    CREATE TEMPORARY TABLE tmp AS
    SELECT
    CONCAT(REPLACE(REPLACE(TRIM(TRAILING ',' FROM Note),', ',','),' ',''),',') AS Note,
    Rating,
    5 AS DerivedRating,
    Type,
    UserId,
    ImageId,
    RatingId,
    0 AS LIcode,
    0 AS LScode,
    0 AS L4code,
    0 AS MScode
    FROM (
      SELECT
      TRIM(REPLACE(REPLACE(Note,'\n',','),',,',',')) AS Note,
      Rating,
      UserId,
      ImageId,
      Rating.Id AS RatingId,
      Type
      FROM Image
      JOIN Rating ON Rating.ImageId=Image.Id
      JOIN Exam ON Exam.Id=Image.ExamId
      JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
      WHERE Note REGEXP '(LI)|(LS)|(L4)|(MS)'
      AND Type='LateralBoneDensity'
    ) x;

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1)
    WHERE Note REGEXP '(LI,)|(LS,)';

    UPDATE tmp
    SET LIcode=1
    WHERE Note REGEXP 'LI,';

    UPDATE tmp
    SET LScode=1
    WHERE Note REGEXP 'LS,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), L4code=1
    WHERE Note REGEXP 'L4,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), MScode=1
    WHERE Note REGEXP 'MS,';

    UPDATE tmp
    SET DerivedRating=1
    WHERE DerivedRating<1;

    UPDATE Rating
    JOIN tmp ON tmp.RatingId=Rating.Id
    SET Rating.DerivedRating=tmp.DerivedRating;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='LI' AND Value=-1 AND CodeGroupId=@Position_Group1_Id)
    FROM tmp
    WHERE LIcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='LS' AND Value=-1 AND CodeGroupId=@Position_Group1_Id)
    FROM tmp
    WHERE LScode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='L4' AND Value=-1 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE L4code=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='MS' AND Value=-1 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE MScode=1;

    DROP TABLE tmp;

    CREATE TEMPORARY TABLE tmp AS
    SELECT
    CONCAT(REPLACE(REPLACE(TRIM(TRAILING ',' FROM Note),', ',','),' ',''),',') AS Note,
    Rating,
    5 AS DerivedRating,
    Type,
    UserId,
    ImageId,
    RatingId,
    0 AS PLcode,
    0 AS PMcode,
    0 AS PScode,
    0 AS PIcode,
    0 AS Fadcode,
    0 AS Fabcode,
    0 AS AMcode,
    0 AS ALcode,
    0 AS AScode,
    0 AS AIcode,
    0 AS CAcode,
    0 AS CScode,
    0 AS MScode
    FROM (
      SELECT
      TRIM(REPLACE(REPLACE(Note,'\n',','),',,',',')) AS Note,
      Rating,
      UserId,
      ImageId,
      Rating.Id AS RatingId,
      Type
      FROM Image
      JOIN Rating ON Rating.ImageId=Image.Id
      JOIN Exam ON Exam.Id=Image.ExamId
      JOIN ScanType ON ScanType.Id=Exam.ScanTypeId
      WHERE Note REGEXP '(PL)|(PM)|(PS)|(PI)|(Fad)|(Fab)|(AM)|(AL)|(AS)|(AI)|(CA)|(CS)|(MS)'
      AND Type='DualHipBoneDensity'
    ) x;

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1)
    WHERE Note REGEXP '(PL,)|(PM,)|(PS,)|(PI,)';

    UPDATE tmp
    SET PLcode=1
    WHERE Note REGEXP 'PL,';

    UPDATE tmp
    SET PMcode=1
    WHERE Note REGEXP 'PM,';

    UPDATE tmp
    SET PScode=1
    WHERE Note REGEXP 'PS,';

    UPDATE tmp
    SET PIcode=1
    WHERE Note REGEXP 'PI,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1)
    WHERE Note REGEXP '(Fab,)|(Fad,)';

    UPDATE tmp
    SET Fabcode=1
    WHERE Note REGEXP 'Fab,';

    UPDATE tmp
    SET Fadcode=1
    WHERE Note REGEXP 'Fad,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1)
    WHERE Note REGEXP '(AM,)|(AL,)|(AS,)|(AI,)';

    UPDATE tmp
    SET AMcode=1
    WHERE Note REGEXP 'AM,';

    UPDATE tmp
    SET ALcode=1
    WHERE Note REGEXP 'AL,';

    UPDATE tmp
    SET AScode=1
    WHERE Note REGEXP 'AS,';

    UPDATE tmp
    SET AIcode=1
    WHERE Note REGEXP 'AI,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1)
    WHERE Note REGEXP '(CA,)|(CS,)';

    UPDATE tmp
    SET CAcode=1
    WHERE Note REGEXP 'CA,';

    UPDATE tmp
    SET CScode=1
    WHERE Note REGEXP 'CS,';

    UPDATE tmp
    SET DerivedRating=(DerivedRating-1), MScode=1
    WHERE Note REGEXP '(MS,)';

    UPDATE tmp
    SET DerivedRating=1
    WHERE DerivedRating<1;

    UPDATE Rating
    JOIN tmp ON tmp.RatingId=Rating.Id
    SET Rating.DerivedRating=tmp.DerivedRating;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='PL' AND Value=-1 AND CodeGroupId=@Position_Group1_Id)
    FROM tmp
    WHERE PLcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='PM' AND Value=-1 AND CodeGroupId=@Position_Group1_Id)
    FROM tmp
    WHERE PMcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='PS' AND Value=-1 AND CodeGroupId=@Position_Group1_Id)
    FROM tmp
    WHERE PScode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='PI' AND Value=-1 AND CodeGroupId=@Position_Group1_Id)
    FROM tmp
    WHERE PIcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='AL' AND Value=-1 AND CodeGroupId=@Analysis_Group1_Id)
    FROM tmp
    WHERE ALcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='AM' AND Value=-1 AND CodeGroupId=@Analysis_Group1_Id)
    FROM tmp
    WHERE AMcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='AS' AND Value=-1 AND CodeGroupId=@Analysis_Group1_Id)
    FROM tmp
    WHERE AScode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='AI' AND Value=-1 AND CodeGroupId=@Analysis_Group1_Id)
    FROM tmp
    WHERE AIcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='Fad' AND Value=-1 AND CodeGroupId=@AngulationId)
    FROM tmp
    WHERE Fadcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='Fab' AND Value=-1 AND CodeGroupId=@AngulationId)
    FROM tmp
    WHERE Fabcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='CA' AND Value=-1 AND CodeGroupId=@Analysis_Group2_Id)
    FROM tmp
    WHERE CAcode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='CS' AND Value=-1 AND CodeGroupId=@Analysis_Group2_Id)
    FROM tmp
    WHERE CScode=1;

    INSERT INTO Code (UserId,ImageId,CodeTypeId)
    SELECT
    UserId,ImageId,(SELECT Id FROM CodeType WHERE Code='MS' AND Value=-1 AND CodeGroupId IS NULL)
    FROM tmp
    WHERE MScode=1;

    DROP TABLE tmp;

  END //
DELIMITER ;

CALL patch_Code();
DROP PROCEDURE IF EXISTS patch_Code;
