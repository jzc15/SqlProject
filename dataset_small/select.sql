USE orderDB;

SELECT * FROM book WHERE id = 200002;

SELECT * FROM orders WHERE orders.id = 100032;

SELECT * FROM website, customer, orders, book WHERE orders.id = 100032
    AND website_id = website.id
    AND customer_id = customer.id
    AND book_id = book.id;

SELECT * FROM website, price, book WHERE book.id = 200002
    AND website_id = website.id
    AND book_id = book.id;

SELECT price FROM website, price, book WHERE book.id = 200002
    AND website_id = website.id
    AND book_id = book.id;

SELECT AVG(price) FROM website, price, book WHERE book.id = 200002
    AND website_id = website.id
    AND book_id = book.id;

SELECT MAX(price) FROM website, price, book WHERE book.id = 200002
    AND website_id = website.id
    AND book_id = book.id;

SELECT MIN(price) FROM website, price, book WHERE book.id = 200002
    AND website_id = website.id
    AND book_id = book.id;