USE orderDB;

DROP INDEX customer(gender);
CREATE INDEX customer(gender);

SELECT * FROM customer WHERE name = 'CHAD CABELLO' AND gender = 'F';