
let menu = [];
let cart = [];

async function loadMenu() {
    try {
        const response = await fetch("/api/menu");

        if (!response.ok)
            throw new Error("Could not load menu");

        menu = await response.json();

        displayMenu();

    } catch (error) {
        document.getElementById("food-list").textContent =
            error.message;
    }
}

function displayMenu() {
    const container = document.getElementById("food-list");

    container.innerHTML = "";

    menu.forEach(food => {
        const card = document.createElement("div");
        card.className = "food-card";

        const name = document.createElement("h3");
        name.textContent = food.name;

        const category = document.createElement("p");
        category.textContent = food.category;

        const description = document.createElement("p");
        description.textContent = food.description;

        const price = document.createElement("h3");
        price.textContent = "₹" + food.price;

        const button = document.createElement("button");
        button.textContent = "Add to Cart";
        button.addEventListener("click", () => addToCart(food.id));

        card.append(name, category, description, price, button);
        container.appendChild(card);
    });
}

function addToCart(id) {
    const food = menu.find(item => item.id === id);

    if (!food) return;

    const existing = cart.find(item => item.id === id);

    if (existing) {
        existing.quantity++;
    } else {
        cart.push({
            id: food.id,
            name: food.name,
            price: food.price,
            quantity: 1
        });
    }

    displayCart();
}

function displayCart() {
    const container = document.getElementById("cart-list");

    container.innerHTML = "";

    let total = 0;

    cart.forEach((item, index) => {
        const subtotal = item.price * item.quantity;
        total += subtotal;

        const div = document.createElement("div");
        div.className = "cart-item";

        const description = document.createElement("span");
        description.textContent =
            `${item.name} - ₹${item.price} × ${item.quantity}`;

        const remove = document.createElement("button");
        remove.textContent = "Remove";

        remove.addEventListener("click", () => {
            cart.splice(index, 1);
            displayCart();
        });

        div.append(description, remove);
        container.appendChild(div);
    });

    document.getElementById("total").textContent =
        "Total: ₹" + total.toFixed(2);
}

async function placeOrder() {
    const customer =
        document.getElementById("customer").value.trim();

    const message =
        document.getElementById("message");

    if (!customer) {
        message.textContent = "Please enter your name.";
        return;
    }

    if (cart.length === 0) {
        message.textContent = "Your cart is empty.";
        return;
    }

    try {
        const response = await fetch("/api/orders", {
            method: "POST",

            headers: {
                "Content-Type": "application/json"
            },

            body: JSON.stringify({
                customer: customer,
                items: cart.map(item => ({
                    id: item.id,
                    quantity: item.quantity
                }))
            })
        });

        const result = await response.json();

        if (!response.ok)
            throw new Error(result.error || "Order failed");

        message.textContent =
            "Order placed successfully! Order ID: " + result.id;

        cart = [];
        displayCart();

    } catch (error) {
        message.textContent = error.message;
    }
}

async function addFood() {
    const name =
        document.getElementById("food-name").value.trim();

    const category =
        document.getElementById("food-category").value.trim();

    const price =
        Number(document.getElementById("food-price").value);

    const description =
        document.getElementById("food-description").value.trim();

    if (!name || !category ||
        !Number.isFinite(price) || price < 0) {

        alert("Enter valid food details.");
        return;
    }

    try {
        const response = await fetch("/api/menu", {
            method: "POST",

            headers: {
                "Content-Type": "application/json"
            },

            body: JSON.stringify({
                name,
                category,
                price,
                description
            })
        });

        const result = await response.json();

        if (!response.ok)
            throw new Error(result.error || "Unable to add food");

        alert("Food added successfully!");

        document.getElementById("food-name").value = "";
        document.getElementById("food-category").value = "";
        document.getElementById("food-price").value = "";
        document.getElementById("food-description").value = "";

        loadMenu();

    } catch (error) {
        alert(error.message);
    }
}

async function loadOrders() {
    const container =
        document.getElementById("orders-list");

    container.textContent = "Loading orders...";

    try {
        const response = await fetch("/api/orders");

        if (!response.ok)
            throw new Error("Unable to load orders");

        const orders = await response.json();

        container.innerHTML = "";

        if (orders.length === 0) {
            container.textContent = "No orders available.";
            return;
        }

        orders.forEach(order => {
            const div = document.createElement("div");
            div.className = "order-card";

            const heading = document.createElement("h3");
            heading.textContent =
                `Order #${order.id} - ${order.customer}`;

            const details = document.createElement("p");
            details.textContent =
                `Total: ₹${order.total} | Status: ${order.status}`;

            const payment = document.createElement("p");
            payment.textContent =
                `Payment: ${order.payment}`;

            const select = document.createElement("select");

            ["Pending", "Preparing", "Completed", "Cancelled"]
                .forEach(status => {
                    const option = document.createElement("option");
                    option.value = status;
                    option.textContent = status;
                    select.appendChild(option);
                });

            select.value = order.status;

            const button = document.createElement("button");
            button.textContent = "Update Status";

            button.addEventListener("click", async () => {
                try {
                    const result = await fetch(
                        `/api/orders/${order.id}/status`,
                        {
                            method: "PUT",
                            headers: {
                                "Content-Type": "application/json"
                            },
                            body: JSON.stringify({
                                status: select.value
                            })
                        }
                    );

                    if (!result.ok)
                        throw new Error("Update failed");

                    alert("Order status updated!");
                    loadOrders();

                } catch (error) {
                    alert(error.message);
                }
            });

            div.append(
                heading,
                details,
                payment,
                select,
                button
            );

            container.appendChild(div);
        });

    } catch (error) {
        container.textContent = error.message;
    }
}

loadMenu();
