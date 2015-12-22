SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='';

DROP SCHEMA IF EXISTS `Alder` ;
CREATE SCHEMA IF NOT EXISTS `Alder` DEFAULT CHARACTER SET latin1 ;
USE `Alder` ;

-- -----------------------------------------------------
-- Table `Alder`.`Wave`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Wave` ;

CREATE TABLE IF NOT EXISTS `Alder`.`Wave` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `Name` VARCHAR(45) NOT NULL,
  `Rank` INT UNSIGNED NOT NULL,
  `MetaDataSource` VARCHAR(45) NOT NULL,
  `ImageDataSource` VARCHAR(45) NOT NULL,
  PRIMARY KEY (`Id`),
  UNIQUE INDEX `uqName` (`Name` ASC),
  UNIQUE INDEX `uqRank` (`Rank` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Site`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Site` ;

CREATE TABLE IF NOT EXISTS `Alder`.`Site` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `Name` VARCHAR(45) NOT NULL,
  `Alias` VARCHAR(45) NULL DEFAULT NULL,
  PRIMARY KEY (`Id`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Interview`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Interview` ;

CREATE TABLE IF NOT EXISTS `Alder`.`Interview` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `WaveId` INT UNSIGNED NOT NULL,
  `SiteId` INT UNSIGNED NOT NULL,
  `UId` VARCHAR(45) NOT NULL,
  `VisitDate` DATE NOT NULL,
  PRIMARY KEY (`Id`),
  UNIQUE INDEX `uqUIdWaveIdVisitDate` (`UId` ASC, `WaveId` ASC, `VisitDate` ASC),
  INDEX `fkWaveId` (`WaveId` ASC),
  INDEX `fkSiteId` (`SiteId` ASC),
  CONSTRAINT `fkInterviewWaveId`
    FOREIGN KEY (`WaveId`)
    REFERENCES `Alder`.`Wave` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT `fkInterviewSiteId`
    FOREIGN KEY (`SiteId`)
    REFERENCES `Alder`.`Site` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Modality`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Modality` ;

CREATE TABLE IF NOT EXISTS `Alder`.`Modality` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `Name` VARCHAR(45) NOT NULL,
  `Help` TEXT NOT NULL,
  PRIMARY KEY (`Id`),
  UNIQUE INDEX `uqName` (`Name` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`ScanType`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`ScanType` ;

CREATE TABLE IF NOT EXISTS `Alder`.`ScanType` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `ModalityId` INT UNSIGNED NOT NULL,
  `Type` VARCHAR(255) NOT NULL,
  `SideCount` TINYINT NOT NULL DEFAULT 0,
  `Parenting` TINYINT NOT NULL DEFAULT 0,
  PRIMARY KEY (`Id`),
  UNIQUE INDEX `uqModalityIdType` (`ModalityId` ASC, `Type` ASC),
  INDEX `fkModalityId` (`ModalityId` ASC),
  CONSTRAINT `fkScanTypeModalityId`
    FOREIGN KEY (`ModalityId`)
    REFERENCES `Alder`.`Modality` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Exam`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Exam` ;

CREATE TABLE IF NOT EXISTS `Alder`.`Exam` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `InterviewId` INT UNSIGNED NOT NULL,
  `ScanTypeId` INT UNSIGNED NOT NULL,
  `Side` ENUM('right','left','none') NOT NULL DEFAULT 'none',
  `Stage` VARCHAR(45) NOT NULL,
  `Interviewer` VARCHAR(45) NOT NULL,
  `DatetimeAcquired` DATETIME NULL,
  `Downloaded` TINYINT(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`Id`),
  INDEX `dkSide` (`Side` ASC),
  INDEX `fkInterviewId` (`InterviewId` ASC),
  UNIQUE INDEX `uqInterviewIdSideScanTypeId` (`InterviewId` ASC, `Side` ASC, `ScanTypeId` ASC),
  INDEX `fkScanTypeId` (`ScanTypeId` ASC),
  CONSTRAINT `fkExamInterviewId`
    FOREIGN KEY (`InterviewId`)
    REFERENCES `Alder`.`Interview` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT `fkExamScanTypeId`
    FOREIGN KEY (`ScanTypeId`)
    REFERENCES `Alder`.`ScanType` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Image`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Image` ;

CREATE TABLE IF NOT EXISTS `Alder`.`Image` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `ExamId` INT UNSIGNED NOT NULL,
  `Acquisition` INT NOT NULL,
  `ParentImageId` INT UNSIGNED NULL DEFAULT NULL,
  `Dimensionality` INT NULL DEFAULT NULL,
  PRIMARY KEY (`Id`),
  INDEX `fkImageExamId` (`ExamId` ASC),
  UNIQUE INDEX `uqExamIdAcquisition` (`ExamId` ASC, `Acquisition` ASC),
  INDEX `fkImageParentImageId` (`ParentImageId` ASC),
  CONSTRAINT `fkImageExamId`
    FOREIGN KEY (`ExamId`)
    REFERENCES `Alder`.`Exam` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT `fkImageParentImageId`
    FOREIGN KEY (`ParentImageId`)
    REFERENCES `Alder`.`Image` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`User`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`User` ;

CREATE TABLE IF NOT EXISTS `Alder`.`User` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `Name` VARCHAR(255) NOT NULL,
  `Password` VARCHAR(255) NOT NULL,
  `Expert` TINYINT(1) NOT NULL DEFAULT 0,
  `InterviewId` INT UNSIGNED NULL DEFAULT NULL,
  `LastLogin` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`Id`),
  UNIQUE INDEX `uqName` (`Name` ASC),
  INDEX `dkLastLogin` (`LastLogin` ASC),
  INDEX `fkInterviewId` (`InterviewId` ASC),
  CONSTRAINT `fkUserInterviewId`
    FOREIGN KEY (`InterviewId`)
    REFERENCES `Alder`.`Interview` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Rating`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Rating` ;

CREATE TABLE IF NOT EXISTS `Alder`.`Rating` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `ImageId` INT UNSIGNED NOT NULL,
  `UserId` INT UNSIGNED NOT NULL,
  `Rating` TINYINT(1) NULL DEFAULT NULL,
  `DerivedRating` TINYINT(1) NULL DEFAULT NULL,
  PRIMARY KEY (`Id`),
  INDEX `fkImageId` (`ImageId` ASC),
  INDEX `fkUserId` (`UserId` ASC),
  INDEX `dkRating` (`Rating` ASC),
  UNIQUE INDEX `uqImageIdUserId` (`ImageId` ASC, `UserId` ASC),
  CONSTRAINT `fkRatingImageId`
    FOREIGN KEY (`ImageId`)
    REFERENCES `Alder`.`Image` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT `fkRatingUserId`
    FOREIGN KEY (`UserId`)
    REFERENCES `Alder`.`User` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`UserHasModality`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`UserHasModality` ;

CREATE TABLE IF NOT EXISTS `Alder`.`UserHasModality` (
  `UserId` INT UNSIGNED NOT NULL,
  `ModalityId` INT UNSIGNED NOT NULL,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  PRIMARY KEY (`UserId`, `ModalityId`),
  INDEX `fkModalityId` (`ModalityId` ASC),
  INDEX `fkUserId` (`UserId` ASC),
  CONSTRAINT `fkUserHasModalityUserId`
    FOREIGN KEY (`UserId`)
    REFERENCES `Alder`.`User` (`Id`)
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fkUserHasModalityModalityId`
    FOREIGN KEY (`ModalityId`)
    REFERENCES `Alder`.`Modality` (`Id`)
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`CodeGroup`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`CodeGroup` ;

CREATE TABLE IF NOT EXISTS `Alder`.`CodeGroup` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `Value` INT NOT NULL DEFAULT 0,
  `Name` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`Id`),
  UNIQUE INDEX `uqNameValue` (`Name` ASC, `Value` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`CodeType`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`CodeType` ;

CREATE TABLE IF NOT EXISTS `Alder`.`CodeType` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `CodeGroupId` INT UNSIGNED NULL DEFAULT NULL,
  `Code` VARCHAR(45) NOT NULL,
  `Value` INT NOT NULL DEFAULT 0,
  `Active` TINYINT(1) NOT NULL DEFAULT 1,
  PRIMARY KEY (`Id`),
  UNIQUE INDEX `uqCodeGroupIdCodeValue` (`CodeGroupId` ASC, `Code` ASC, `Value` ASC),
  INDEX `fkCodeGroupId` (`CodeGroupId` ASC),
  CONSTRAINT `fkCodeTypeCodeGroupId`
    FOREIGN KEY (`CodeGroupId`)
    REFERENCES `Alder`.`CodeGroup` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Code`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Code` ;

CREATE TABLE IF NOT EXISTS `Alder`.`Code` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `ImageId` INT UNSIGNED NOT NULL,
  `UserId` INT UNSIGNED NOT NULL,
  `CodeTypeId` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`Id`),
  INDEX `fkImageId` (`ImageId` ASC),
  INDEX `fkUserId` (`UserId` ASC),
  UNIQUE INDEX `uqCodeTypeIdImageIdUserId` (`CodeTypeId` ASC, `ImageId` ASC, `UserId` ASC),
  INDEX `fkCodeTypeId` (`CodeTypeId` ASC),
  CONSTRAINT `fkCodeImageId`
    FOREIGN KEY (`ImageId`)
    REFERENCES `Alder`.`Image` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT `fkCodeUserId`
    FOREIGN KEY (`UserId`)
    REFERENCES `Alder`.`User` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT `fkCodeCodeTypeId`
    FOREIGN KEY (`CodeTypeId`)
    REFERENCES `Alder`.`CodeType` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`ScanTypeHasCodeType`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`ScanTypeHasCodeType` ;

CREATE TABLE IF NOT EXISTS `Alder`.`ScanTypeHasCodeType` (
  `ScanTypeId` INT UNSIGNED NOT NULL,
  `CodeTypeId` INT UNSIGNED NOT NULL,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  PRIMARY KEY (`ScanTypeId`, `CodeTypeId`),
  INDEX `fkCodeTypeId` (`CodeTypeId` ASC),
  INDEX `fkScanTypeId` (`ScanTypeId` ASC),
  CONSTRAINT `fkScanTypeHasCodeTypeScanTypeId`
    FOREIGN KEY (`ScanTypeId`)
    REFERENCES `Alder`.`ScanType` (`Id`)
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fkScanTypeHasCodeTypeCodeTypeId`
    FOREIGN KEY (`CodeTypeId`)
    REFERENCES `Alder`.`CodeType` (`Id`)
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`ImageNote`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`ImageNote` ;

CREATE TABLE IF NOT EXISTS `Alder`.`ImageNote` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  `ImageId` INT UNSIGNED NOT NULL,
  `UserId` INT UNSIGNED NOT NULL,
  `Note` TEXT NOT NULL,
  PRIMARY KEY (`Id`),
  INDEX `fkUserId` (`UserId` ASC),
  INDEX `fkImageId` (`ImageId` ASC),
  CONSTRAINT `fkNoteUserId`
    FOREIGN KEY (`UserId`)
    REFERENCES `Alder`.`User` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT `fkNoteImageId`
    FOREIGN KEY (`ImageId`)
    REFERENCES `Alder`.`Image` (`Id`)
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`WaveHasScanType`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`WaveHasScanType` ;

CREATE TABLE IF NOT EXISTS `Alder`.`WaveHasScanType` (
  `WaveId` INT UNSIGNED NOT NULL,
  `ScanTypeId` INT UNSIGNED NOT NULL,
  `UpdateTimestamp` TIMESTAMP NOT NULL,
  `CreateTimestamp` TIMESTAMP NOT NULL,
  PRIMARY KEY (`WaveId`, `ScanTypeId`),
  INDEX `fkScanTypeId` (`ScanTypeId` ASC),
  INDEX `fkWaveId` (`WaveId` ASC),
  CONSTRAINT `fkWaveHasScanTypeWaveId`
    FOREIGN KEY (`WaveId`)
    REFERENCES `Alder`.`Wave` (`Id`)
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fkWaveHasScanTypeScanTypeId`
    FOREIGN KEY (`ScanTypeId`)
    REFERENCES `Alder`.`ScanType` (`Id`)
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
