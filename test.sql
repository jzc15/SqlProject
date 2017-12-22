CREATE DATABASE test_db;
USE test_db;
CREATE TABLE students (id INT(10), name VARCHAR(200), age INT(10), PRIMARY KEY(id));
CREATE TABLE books (title VARCHAR(200), weight INT(20), FOREIGN KEY(student_id) REFERENCES students(id));
SHOW TABLES;

INSERT INTO students VALUES (0, 'Alice', 10), (1, 'Bob', 11), (3, 'King', 100), (7, 'King', 100), (2, 'King', 100), (100, 'King', 100);
INSERT INTO students VALUES (110, 'Alice', 10), (111, 'Bob', 11), (113, 'King', 100), (17, 'King', 100), (21, 'King', 100), (10, 'King', 100);
-- INSERT INTO students VALUES (0, 'KKK', 100);

INSERT INTO books VALUES ('Book Name', 20, 0);
INSERT INTO books VALUES ('Book Name', 20, 1);

-- DELETE FROM students WHERE id < 100 AND name = 'Bob';

UPDATE students SET age = 20 WHERE id = 0 AND name = 'Bob';
-- UPDATE students SET age = 'kkk' WHERE id = 0 AND name = 'Bob';

SELECT * FROM students WHERE id = 0;
SELECT * FROM students, books WHERE books.student_id = students.id AND students.name = 'Bob';