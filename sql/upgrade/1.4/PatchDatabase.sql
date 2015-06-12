-- Patch to upgrade database to version 1.4

SET AUTOCOMMIT=0;

SOURCE Exam.sql
SOURCE Rating.sql
SOURCE Image.sql
SOURCE ScanType.sql
SOURCE Exam2.sql
SOURCE CodeGroup.sql
SOURCE CodeType.sql
SOURCE ScanTypeHasCodeType.sql
SOURCE Code.sql

COMMIT;
