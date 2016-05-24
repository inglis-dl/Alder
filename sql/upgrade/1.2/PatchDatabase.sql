-- Patch to upgrade database to version 1.2

SET AUTOCOMMIT=0;

SOURCE Image.sql

COMMIT;
