USE orderDB;

SELECT * FROM orders WHERE book_id = 203813;

DELETE FROM book WHERE id = 203813;

SELECT * FROM orders WHERE book_id = 203813;