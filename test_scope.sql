CREATE DATABASE test_scope;
USE test_scope;

CREATE TABLE test (id INT(10), gender VARCHAR(1) NOT NULL, CHECK(gender in ('F', 'M')));

INSERT INTO test VALUES (1, 'F'), (2, 'M');

INSERT INTO test VALUES (3, 'N');