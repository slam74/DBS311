--DBS311 Assignment1 
--by Simon Lam

--1) Display the employee number, full employee name, job title, and hire date of all employees hired in September, 
--excluding employees with Administrative jobs (their title starts with Admin), with the most recently hired employees 
--displayed first, followed by their last name ascending.
SELECT employee_id "Emp Id", last_name ||', '|| first_name "Full Name", job_title "Job", TO_CHAR(hire_date, 'Month ddth "of" YYYY') "Start Date"
FROM employees
WHERE LOWER(job_title) NOT LIKE 'admin%'
AND EXTRACT(MONTH FROM hire_date)=9
ORDER BY 4 DESC, last_name ASC;

--2) The company wants to see the total sale amount per sales person (salesman) for all orders. Assume that some 
--online orders do not have any sales representative. For online orders (orders with no salesman ID), consider the 
--salesman ID as 0. Display the salesman ID and the total sale amount for the employee for each employee. 
--Sort the result according to employee number.
SELECT NVL(o.salesman_id, 0) "Employee Id", TO_CHAR(SUM(i.quantity*i.unit_price),'$999,999,999.99') "Total Sale"
FROM orders o LEFT JOIN order_items i
USING (order_id)
GROUP BY o.salesman_id
ORDER BY 1;

--3) Display customer Id, customer name and total number of orders for customers 
--with their Id less than 200 and with name starting on F or J, but only if their
--total number of orders is less than 3.  Include the customers with no orders in your report as well.
--Sort the result by the value of total orders ascending, followed by name ascending.
SELECT c.customer_id "CustId", c.name "Name", COUNT(o.order_id) "Total Orders"
FROM customers c LEFT JOIN orders o
ON c.customer_id=o.customer_id
WHERE c.customer_id<200
AND SUBSTR(name,1,1) IN('F','J')
GROUP BY c.customer_id, c.name
HAVING COUNT(o.order_id)<3
ORDER BY 3 ASC, 2 ASC;

--4) Display customer Id, customer name, and the order id and the order date of all orders for customer whose ID is 44.
--a.	Show also the total number of items ordered and the total amount of each customer’s order.
--b.	Exclude Orders with the Total Amount exceeding 1 million dollars
--c.	Sort the result from the highest to lowest total order amount.
SELECT c.customer_id "Cust#", c.name "Name", o.order_id "Order Id", TO_CHAR(o.order_date, 'dd-MON-yy') "Order Dat", SUM(i.quantity) "Total Items", TO_CHAR(SUM(i.quantity*i.unit_price),'FM$999,999,999.99') "Total Amount"
FROM customers c JOIN orders o
ON c.customer_id=o.customer_id
JOIN order_items i
ON o.order_id=i.order_id
WHERE c.customer_id=44
GROUP BY c.customer_id, c.name, o.order_id, o.order_date
HAVING SUM(i.quantity*i.unit_price)<=1000000
ORDER BY 6 DESC;

--5) Display customer Id, name, total number of orders, the total number of items ordered, and the total order amount 
--for customers who have more than 30 orders. Sort the result based on the total number of orders.
SELECT c.customer_id "Cust#", c.name "Name", COUNT(o.order_id) "# of Orders", SUM(i.quantity) "Total Items", TO_CHAR(SUM(i.quantity*i.unit_price), '$999,999,999.99') "Total Amount"
FROM customers c JOIN orders o
ON c.customer_id=o.customer_id
JOIN order_items i
ON o.order_id=i.order_id
GROUP BY c.customer_id, c.name
HAVING COUNT(o.order_id)>30
ORDER BY 3;

--6) Display Warehouse Id, warehouse name, product category Id, product category name, and the lowest product standard cost for this combination.
--In your result, include the rows that the lowest standard cost is less then $200.
--Also, include the rows that the lowest cost is more than $500.
--Sort the output according to Warehouse Id, warehouse name and then product category Id, and product category name
SELECT i.warehouse_id "Wrhs#", w.warehouse_name "Warehouse Name", c.category_id "Category ID", c.category_name "Category Name", TO_CHAR(MIN(p.standard_cost),'$999,999,999.99') "Lowest Cost"
FROM warehouses w JOIN inventories i
ON w.warehouse_id=i.warehouse_id
JOIN products p
ON i.product_id=p.product_id
JOIN product_categories c
ON p.category_id=c.category_id
GROUP BY i.warehouse_id, w.warehouse_name, c.category_id, c.category_name
HAVING MIN(p.standard_cost) NOT BETWEEN 200 AND 500
ORDER BY 1, 2, 3, 4;

--7) Display product Id, name, and list Price for products that were purchased in orders 
--handled by salesman Marshall and with list price greater than all average list prices 
--per each category. Sort the output by Id ascending.
SELECT p.product_id "ProdId", p.product_name "Product Name", p.list_price "LPrice"
FROM products p JOIN order_items i
ON p.product_id=i.product_id
JOIN orders o
ON o.order_id=i.order_id
JOIN employees e
ON o.salesman_id=e.employee_id
WHERE o.salesman_id=(SELECT employee_id FROM employees WHERE LOWER(last_name)='marshall')
AND p.list_price>ALL(SELECT a.price FROM (SELECT AVG(list_price) AS price, category_id AS category FROM products GROUP BY category_id) a)
ORDER BY p.product_id ASC;

--8) Display customer Id, name, and total number of orders, for orders handled by salesman Marshall, but only if customer name 
--begins on General or ends on Electric. Exclude customers who placed a single order, but include customers without orders as well. 
--Sort the result based on the total number of orders descending and then by name ascending.
--Do not use LIKE operator and do not join 3 tables
SELECT o.customer_id, c.name, COUNT(o.order_id)
FROM orders o LEFT JOIN customers c
ON o.customer_id=c.customer_id
WHERE (o.salesman_id=(SELECT employee_id FROM employees WHERE UPPER(last_name)='MARSHALL') OR o.salesman_id IS NULL)
AND (UPPER(SUBSTR(c.name,1,7))='GENERAL' OR UPPER(SUBSTR(c.name,-8,8))='ELECTRIC')
GROUP BY o.customer_id, c.name
HAVING COUNT(o.order_id)!=1
UNION
SELECT customer_id, name, NVL(TO_NUMBER(null),0)
FROM customers
WHERE customer_id NOT IN(SELECT customer_id FROM orders)
AND (UPPER(SUBSTR(name,1,7))='GENERAL' OR UPPER(SUBSTR(name,-8,8))='ELECTRIC')
ORDER BY 3 DESC, 2;

--9) Display product Id, name, and list Price for products that their list price is more than any highest product standard cost per warehouse outside Americas regions.
--(You need to find the highest standard cost for each warehouse that is located outside the Americas regions. Then you need to return all products that their list price is higher than any highest standard cost of those warehouses.)
--Sort the result according to list price.
SELECT pr.product_id "Product ID", pr.product_name "Product Name", TO_CHAR(pr.list_price,'$999,999,999.99') "List Price"
FROM products pr
WHERE pr.list_price>ANY(SELECT MAX(p.standard_cost)
FROM products p JOIN inventories i ON p.product_id=i.product_id
JOIN warehouses w ON i.warehouse_id=w.warehouse_id
JOIN locations l ON w.location_id=l.location_id
JOIN countries c ON l.country_id=c.country_id
JOIN regions r ON c.region_id=r.region_id
WHERE LOWER (r.region_name)!='americas'
GROUP BY w.warehouse_id) 
ORDER BY 3 DESC;

--10) Display product Id, name, and list Price for the most expensive product,  then the cheapest product 
--and also for  the product with the price closest to the average product price (rounded to the nearer ten). 
--For the third row exclude products with name that starts on Intel.
SELECT product_id "Product ID", product_name "Product Name", TO_CHAR(list_price,'$999,999,999.99') "Price"
FROM products
WHERE list_price=(SELECT MAX(list_price) FROM products)
UNION 
SELECT product_id "Product ID", product_name "Product Name", TO_CHAR(list_price,'$999,999,999.99') "Price"
FROM products
WHERE list_price=(SELECT MIN(list_price) FROM products)
UNION
(SELECT product_id "Product ID", product_name "Product Name", TO_CHAR(list_price,'$999,999,999.99') "Price"
FROM products
WHERE LOWER(product_name) NOT LIKE 'intel%'
ORDER BY ABS((SELECT ROUND(AVG(list_price),-1) FROM products)-list_price)
FETCH NEXT ROW ONLY);
