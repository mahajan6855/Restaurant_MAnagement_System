
CREATE DATABASE restaurant_db;

USE restaurant_db;

-- 1. USERS TABLE
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(150) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    role ENUM('admin', 'customer') DEFAULT 'customer',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 2. FOOD CATEGORIES
CREATE TABLE categories (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL UNIQUE
);

-- 3. FOOD ITEMS
CREATE TABLE food_items (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(150) NOT NULL,
    category_id INT,
    description TEXT,
    price DECIMAL(10,2) NOT NULL,
    image_url VARCHAR(500),
    available BOOLEAN DEFAULT TRUE,
    FOREIGN KEY (category_id)
        REFERENCES categories(id)
        ON DELETE SET NULL
);

-- 4. ORDERS
CREATE TABLE orders (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    total_amount DECIMAL(10,2) NOT NULL,
    status ENUM(
        'Pending',
        'Preparing',
        'Completed',
        'Cancelled'
    ) DEFAULT 'Pending',
    payment_status ENUM(
        'Unpaid',
        'Paid'
    ) DEFAULT 'Unpaid',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id)
        REFERENCES users(id)
);

-- 5. ORDER ITEMS
CREATE TABLE order_items (
    id INT AUTO_INCREMENT PRIMARY KEY,
    order_id INT NOT NULL,
    food_id INT NOT NULL,
    quantity INT NOT NULL,
    price DECIMAL(10,2) NOT NULL,
    subtotal DECIMAL(10,2) NOT NULL,
    FOREIGN KEY (order_id)
        REFERENCES orders(id),
    FOREIGN KEY (food_id)
        REFERENCES food_items(id)
);

-- 6. PAYMENT RECORDS
CREATE TABLE payments (
    id INT AUTO_INCREMENT PRIMARY KEY,
    order_id INT NOT NULL,
    amount DECIMAL(10,2) NOT NULL,
    payment_method VARCHAR(50),
    payment_status ENUM(
        'Pending',
        'Successful',
        'Failed'
    ) DEFAULT 'Pending',
    transaction_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (order_id)
        REFERENCES orders(id)
);

-- SAMPLE CATEGORIES
INSERT INTO categories(name) VALUES
('Pizza'),
('Burger'),
('Indian Food'),
('Chinese Food'),
('Desserts'),
('Drinks');

-- SAMPLE FOOD ITEMS
INSERT INTO food_items
(name, category_id, description, price)
VALUES
('Margherita Pizza', 1, 'Classic cheese pizza', 199),
('Veg Burger', 2, 'Fresh vegetable burger', 99),
('Paneer Biryani', 3, 'Delicious paneer biryani', 179),
('Veg Noodles', 4, 'Chinese style noodles', 149),
('Chocolate Cake', 5, 'Chocolate dessert', 89),
('Cold Coffee', 6, 'Chilled coffee', 79);

-- INDEXES
CREATE INDEX idx_orders_user ON orders(user_id);
CREATE INDEX idx_orders_date ON orders(created_at);
CREATE INDEX idx_food_category ON food_items(category_id);
