-- Patch to upgrade database to version 1.4

SET AUTOCOMMIT=0;

SOURCE Wave.sql
SOURCE Site.sql
SOURCE Interview.sql
SOURCE ImageNote.sql
SOURCE Image.sql
SOURCE User.sql
SOURCE UserHasModality.sql
SOURCE ScanType.sql
SOURCE ScanTypeHasCodeType.sql
SOURCE Code.sql
SOURCE CodeType.sql
SOURCE Rating.sql
SOURCE Exam.sql

COMMIT;
