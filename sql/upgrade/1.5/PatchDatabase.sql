-- Patch to upgrade database to version 1.4

SET AUTOCOMMIT=0;

SOURCE Wave.sql
SOURCE Site.sql
SOURCE Interview.sql
SOURCE ImageNote.sql
SOURCE Image.sql

COMMIT;
