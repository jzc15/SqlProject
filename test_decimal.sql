CREATE DATABASE test_db;
USE test_db;

CREATE TABLE test (id INT(10), flt FLOAT, deci DECIMAL(2, 2));

INSERT INTO test VALUES (1, 1.3, 2.3), (2, 1.3, 2.03);

SELECT * FROM test WHERE id = 1;
SELECT * FROM test WHERE flt = 1.3;
SELECT * FROM test WHERE deci < 2.3;