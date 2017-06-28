-- Patch to upgrade database to version 1.7

SET AUTOCOMMIT=0;

SOURCE Interview.sql
SOURCE Image.sql

COMMIT;
