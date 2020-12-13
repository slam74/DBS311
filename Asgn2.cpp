/*
DBS311 Assignment 2
*/

#include <iostream>
#include <occi.h>

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;

struct ShoppingCart
{
	int product_id{ 0 };
	double price{ 0 };
	int quantity{ 0 };
};

const int MAX_CART_SIZE = 5;	// the max number of items in one customer order

int mainMenu();
int customerLogin(Connection* conn, int customerId);
int addToCart(Connection* conn, struct ShoppingCart cart[]);
double findProduct(Connection* conn, int product_id);
void displayProducts(struct ShoppingCart cart[], int productCount);
int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount);

int main()
{
	// OCCI Variables
	Environment* env = nullptr;
	Connection* conn = nullptr;
	// Used Variables
	string str;
	string user = "[insert username here]";
	string pass = "[insert password here]";
	string constr = "myoracle12c.senecacollege.ca:1521/oracle12c";
	try
	{
		env = Environment::createEnvironment(Environment::DEFAULT);
		conn = env->createConnection(user, pass, constr);
		cout << "Connection is Successful!" << endl;

		Statement* stmt = conn->createStatement();
		stmt->execute("CREATE OR REPLACE PROCEDURE find_customer (p_customer_id IN NUMBER, found OUT NUMBER) IS"
			" v_custid NUMBER;"
			" BEGIN"
			" found:=1;"
			" SELECT customer_id"
			" INTO v_custid"
			" FROM customers"
			" WHERE customer_id=p_customer_id;"
			" EXCEPTION"
			" WHEN NO_DATA_FOUND THEN"
			" found:=0;"
			" END;"
		);

		stmt->execute("CREATE OR REPLACE PROCEDURE find_product(p_product_id IN NUMBER, price OUT products.list_price%TYPE) IS"
			" BEGIN"
			" SELECT list_price INTO price"
			" FROM products"
			" WHERE product_id=p_product_id;"
			" EXCEPTION"
			" WHEN NO_DATA_FOUND THEN"
			" price:=0;"
			" END;"
		);

		stmt->execute("CREATE OR REPLACE PROCEDURE add_order(p_customer_id IN NUMBER, new_order_id OUT NUMBER) IS"
			" BEGIN"
			" SELECT MAX(order_id) INTO new_order_id"
			" FROM orders;"
			" new_order_id:=new_order_id+1;"
			" INSERT INTO orders"
			" VALUES(new_order_id, p_customer_id, 'Shipped', 56, sysdate);"
			" END;"
		);

		stmt->execute("CREATE OR REPLACE PROCEDURE"
			" add_order_item(orderId IN order_items.order_id % type,"
			" itemId IN order_items.item_id % type,"
			" productId IN order_items.product_id % type,"
			" quantity IN order_items.quantity % type,"
			" price IN order_items.unit_price % type)"
			" IS"
			" BEGIN"
			" INSERT INTO order_items"
			" VALUES(orderId, itemId, productId, quantity, price);"
			" END;"
		);

		ShoppingCart cart[MAX_CART_SIZE];
		int option = 0, id = 0, exists = 0, count = 0;
		do
		{
			// display the main menu options
			option = mainMenu();
			switch (option)
			{
			case 1:
				// prompt the user to enter the customer ID
				cout << "Enter the customer ID: ";
				cin >> id;
				// if the customer ID exists, continue with rest of the program
				exists = customerLogin(conn, id);
				if (exists)
				{
					count = addToCart(conn, cart);
					displayProducts(cart, count);
					checkout(conn, cart, id, count);
				}
				else
				{
					cout << "The customer does not exist." << endl;
				}
				break;
			case 0:
				cout << "Good bye!..." << endl;
				return 0;
			}
		} while (option != 0);	// continue looping until the user enters 0 to exit the program

		env->terminateConnection(conn);
		Environment::terminateEnvironment(env);
	}
	catch (SQLException& sqlExcp)
	{
		cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	}
	return 0;
}

// returns the integer value which is the selected option by the user from the menu
int mainMenu()
{
	cout << "******************** Main Menu ********************" << endl;
	cout << "1) Login" << endl;
	cout << "0) Exit" << endl;
	cout << "Enter an option (0-1): ";

	int choice;
	char newline;
	bool done;
	// check if the user enters a valid option
	// continue prompting the user until a valid option is entered
	do
	{
		cin >> choice;
		newline = cin.get();
		if (cin.fail() || newline != '\n')
		{
			done = false;
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "You entered a wrong value. Enter an option (0-1): ";
		}
		else
		{
			done = choice >= 0 && choice <= 1;
			if (!done)
			{
				cout << "You entered a wrong value. Enter an option (0-1): ";
			}
		}
	} while (!done);
	return choice;
}

// checks if the customer exists in the database
// if customer exists, found is 1, otherwise found is 0
int customerLogin(Connection* conn, int customerId)
{
	int found = 0;
	Statement* stmt = conn->createStatement();
	// call the find_customer procedure
	stmt->setSQL("BEGIN"
		" find_customer(:1, :2);"
		" END;"
	);
	stmt->setNumber(1, customerId);
	stmt->registerOutParam(2, Type::OCCIINT, sizeof(found));
	stmt->executeUpdate();
	// store the out parameter into found
	found = stmt->getInt(2);

	return found;
}

int addToCart(Connection* conn, struct ShoppingCart cart[])
{
	cout << "-------------- Add Products to Cart --------------" << endl;
	for (int i = 0; i < MAX_CART_SIZE; ++i) {
		int productId, qty, choice, numOfProducts = 0;
		ShoppingCart item;

		do {
			// prompt the user for product ID
			cout << "Enter the product ID: ";
			cin >> productId;
			// call findProduct to see if the product ID exists
			if (!findProduct(conn, productId)) {
				cout << "The product does not exist. Try again..." << endl;
			}
			// continue looping the prompt until the product ID entered exists
		} while (!findProduct(conn, productId));

		// displays the product's price
		cout << "Product Price: " << findProduct(conn, productId) << endl;
		// prompt the user to enter the quantity
		cout << "Enter the product Quantity: ";
		cin >> qty;

		item.product_id = productId;
		item.price = findProduct(conn, productId);	// Error handling
		item.quantity = qty;
		cart[i] = item;
		// increment the number of products in the cart
		numOfProducts++;
		// if the cart is full, return the number of products in the cart
		if (numOfProducts == MAX_CART_SIZE)
			return numOfProducts;
		else {
			do {
				cout << "Enter 1 to add more products or 0 to checkout: ";
				cin >> choice;
				// if user enters 0 return the number of products entered
				if (!choice)
					return numOfProducts;
			} while (choice != 0 && choice != 1);
		}
	}
}

// returns the price of the product with ID of product_id
// returns 0 if the product ID is not valid
double findProduct(Connection* conn, int product_id)
{
	Statement* stmt = conn->createStatement();
	// call the find_product stored procedure
	stmt->setSQL("BEGIN"
		" find_product(:1, :2);"
		" END;");
	double price = 0;
	stmt->setNumber(1, product_id);
	stmt->registerOutParam(2, Type::OCCIDOUBLE, sizeof(price));
	stmt->executeUpdate();
	// store the out parameter into price
	price = stmt->getDouble(2);
	conn->terminateStatement(stmt);
	return price;
}

// displays the products added by the user to the shopping cart
void displayProducts(struct ShoppingCart cart[], int productCount)
{
	double total = 0;
	cout << "------- Ordered Products ---------" << endl;
	for (auto i = 0;i < productCount;i++)
	{
		cout << "---Item " << i + 1 << endl;
		cout << "Product ID: " << cart[i].product_id << endl;
		cout << "Price: " << cart[i].price << endl;
		cout << "Quantity: " << cart[i].quantity << endl;
		// add to the total order amount
		total += cart[i].quantity * cart[i].price;
	}
	cout << "----------------------------------" << endl;
	// display the total amount
	cout << "Total: " << total << endl;
}

int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount)
{
	int newOrderId = 0;
	char choice;
	char newline;
	do
	{
		// ask the user if they want to checkout
		cout << "Would you like to checkout? (Y/y or N/n) ";
		cin >> choice;
		// check if the user entered a valid option
		newline = cin.get();
		if (cin.fail() || newline != '\n')
		{
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "Wrong input. Try again..." << endl;
		}
		else
		{
			if (choice != 'Y' && choice != 'y' && choice != 'N' && choice != 'n')
			{
				cout << "Wrong input. Try again..." << endl;
			}
		}
		// keep asking the user until they enter a valid choice
	} while (choice != 'Y' && choice != 'y' && choice != 'N' && choice != 'n');

	if (choice == 'Y' || choice == 'y')
	{
		Statement* stmt = conn->createStatement();
		// call the add_order stored procedure
		stmt->setSQL("BEGIN"
			" add_order(:1, :2);"
			" END;"
		);
		stmt->setNumber(1, customerId);
		stmt->registerOutParam(2, Type::OCCIINT, sizeof(newOrderId));
		stmt->executeUpdate();
		// save the returning value of the out parameter into newOrderId
		newOrderId = stmt->getInt(2);
		conn->terminateStatement(stmt);
	}
	else
	{
		cout << "The order is cancelled." << endl;
		return 0;
	}

	Statement* stmt = conn->createStatement();
	// for each item in the cart, call the stored procedure add_order_item
	for (auto i = 0;i < productCount;i++)
	{
		stmt->setSQL("BEGIN"
			" add_order_item(:1, :2, :3, :4, :5);"
			" END;"
		);
		stmt->setNumber(1, newOrderId);
		stmt->setNumber(2, i + 1);
		stmt->setNumber(3, cart[i].product_id);
		stmt->setNumber(4, cart[i].quantity);
		stmt->setDouble(5, cart[i].price);
	}
	conn->terminateStatement(stmt);

	cout << "The order is successfully completed." << endl;
	return 1;
}
