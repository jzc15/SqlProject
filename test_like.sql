CREATE DATABASE test_like;
USE test_like;

CREATE TABLE test (id INT(10), name VARCHAR(100));

INSERT INTO test VALUES (1, 'Alice'), (2, 'Bob');

SELECT * FROM test WHERE id = 1;
SELECT * FROM test WHERE name LIKE '%A%';
SELECT * FROM test WHERE name LIKE 'A%';
SELECT * FROM test WHERE name LIKE 'B__';