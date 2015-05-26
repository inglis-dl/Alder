-- Patch to upgrade database to version 1.4

SET AUTOCOMMIT=0;

SOURCE Exam.sql
SOURCE Rating.sql
SOURCE Image.sql

COMMIT;
