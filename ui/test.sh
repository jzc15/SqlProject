set -ex
rm -rf database
../build/main ../dataset_small/create.sql
../build/main ../dataset_small/book.sql
../build/main ../dataset_small/customer.sql
../build/main ../dataset_small/website.sql
../build/main ../dataset_small/orders.sql
../build/main ../dataset_small/price.sql
