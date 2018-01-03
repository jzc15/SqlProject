USE orderDB;

-- SELECT * FROM book WHERE id = 200002;

-- SELECT * FROM orders WHERE orders.id = 100032;

-- SELECT * FROM website, customer, orders, book WHERE orders.id = 100032
--     AND website_id = website.id
--     AND customer_id = customer.id
--     AND book_id = book.id;

-- SELECT * FROM website, price, book WHERE book.id = 200002
--     AND website_id = website.id
--     AND book_id = book.id;

-- SELECT price FROM website, price, book WHERE book.id = 200002
--     AND website_id = website.id
--     AND book_id = book.id;

-- SELECT AVG(price) FROM website, price, book WHERE book.id = 200002
--     AND website_id = website.id
--     AND book_id = book.id;

-- SELECT MAX(price) FROM website, price, book WHERE book.id = 200002
--     AND website_id = website.id
--     AND book_id = book.id;

-- SELECT MIN(price) FROM website, price, book WHERE book.id = 200002
--     AND website_id = website.id
--     AND book_id = book.id;

-- SELECT * FROM book WHERE title LIKE '%Without%';

-- SELECT * FROM book WHERE publisher='Monday Morning Books';

-- SELECT title FROM book WHERE authors is null;

-- SELECT book.title,orders.quantity FROM book,orders WHERE book.id=orders.book_id AND orders.quantity>8;

-- SELECT title,authors FROM book WHERE authors like '%David%';

-- SELECT website.name, website.url, price.price FROM website, book, price WHERE website.id=price.website_id AND book.id=price.book_id AND book.title='The New Kid on the Block';

-- SELECT customer.name, website.name, book.title, orders.quantity
--     FROM website, customer, book,orders
--     WHERE website.id=orders.website_id
--         AND customer.id=orders.customer_id
--         AND book.id=orders.book_id
--         and orders.date='2000-02-29';

-- INSERT INTO orders VALUES (127664,1015,315000,200001,'2017-10-9','eight');

-- UPDATE orders SET customer_id = 316001 WHERE id= 100001;

SELECT SUM(quantity) FROM orders;
SELECT AVG(quantity) FROM orders;
SELECT MAX(quantity) FROM orders;
SELECT MIN(quantity) FROM orders;