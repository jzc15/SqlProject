CREATE DATABASE test_aggregate;
USE test_aggregate;

CREATE TABLE test (id INT(10), flt FLOAT, age INT(10));

INSERT INTO test VALUES (1, 1.3, 20), (2, 2.3, 25);

SELECT * FROM test WHERE id = 1;
SELECT COUNT(*) FROM test WHERE age < 50;
SELECT MAX(age) FROM test WHERE age < 50;
SELECT AVG(flt) FROM test WHERE age < 50;