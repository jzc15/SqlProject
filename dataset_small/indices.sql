USE orderDB;

DROP INDEX price(price);

CREATE INDEX price(price);

SELECT * FROM website, price, book WHERE price = 175.3
    AND website_id = website.id
    AND book_id = book.id;