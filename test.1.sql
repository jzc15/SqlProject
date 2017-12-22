CREATE DATABASE test_db;
USE test_db;
CREATE TABLE test_table (col1 INT(10), col2 INT(5), col4 VARCHAR(200));
SHOW TABLES;
DESC test_table;

INSERT INTO test_table VALUES (2, 3, '123'), (4, 5, 'abc'), (5, 2, 'qwer'), (5, 2, NULL);
DELETE FROM test_table WHERE col1 = 2;
DELETE FROM test_table WHERE col1 = 2;
DELETE FROM test_table WHERE col1 > col2;
DELETE FROM test_table WHERE col1 < col2;

INSERT INTO test_table VALUES (1, 3, '123'), (1, 5, 'abc'), (2, 2, 'qwer'), (2, 7, NULL);
UPDATE test_table SET col2 = 4 WHERE col1 = 1;
UPDATE test_table SET col4 = '45663' WHERE col4 IS NULL;
SELECT * FROM test_table WHERE col1 = 2;
UPDATE test_table SET col1 = 2 WHERE col1 = 1;
UPDATE test_table SET col4 = NULL WHERE col2 = 7;
SELECT * FROM test_table WHERE col1 = 2;