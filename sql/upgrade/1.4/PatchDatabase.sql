-- Patch to upgrade database to version 1.4

SET AUTOCOMMIT=0;

SOURCE Exam.sql
SOURCE Rating.sql
SOURCE ScanType.sql
SOURCE Exam2.sql
SOURCE Image.sql
SOURCE CodeGroup.sql
SOURCE CodeType.sql
SOURCE ScanTypeHasCodeType.sql
SOURCE Code.sql
SOURCE RemovePlaque.sql

COMMIT;
