
const API = "/api";

function getToken() {
    return sessionStorage.getItem("token");
}

function authHeaders() {
    return {
        "Content-Type": "application/json",
        "Authorization": "Bearer " + (getToken() || "")
    };
}

const loginForm = document.getElementById("loginForm");

if (loginForm) {
    loginForm.addEventListener("submit", async event => {
        event.preventDefault();

        const message = document.getElementById("message");

        try {
            const response = await fetch(API + "/login", {
                method: "POST",
                headers: {"Content-Type": "application/json"},
                body: JSON.stringify({
                    email: document.getElementById("email").value,
                    password: document.getElementById("password").value
                })
            });

            const data = await response.json();

            if (!response.ok)
                throw new Error(data.error);

            sessionStorage.setItem("token", data.token);
            sessionStorage.setItem("role", data.role);
            sessionStorage.setItem("name", data.name);

            message.textContent = "Login successful!";

            location.href =
                data.role === "admin"
                ? "dashboard.html"
                : "menu.html";

        } catch (error) {
            message.textContent = error.message;
        }
    });
}

const registerForm = document.getElementById("registerForm");

if (registerForm) {
    registerForm.addEventListener("submit", async event => {
        event.preventDefault();

        const message = document.getElementById("message");

        try {
            const response = await fetch(API + "/register", {
                method: "POST",
                headers: {"Content-Type": "application/json"},
                body: JSON.stringify({
                    name: document.getElementById("name").value,
                    email: document.getElementById("email").value,
                    password: document.getElementById("password").value
                })
            });

            const data = await response.json();

            if (!response.ok)
                throw new Error(data.error);

            message.textContent =
                "Registration successful! Please login.";

            registerForm.reset();

        } catch (error) {
            message.textContent = error.message;
        }
    });
}

function logout() {
    fetch(API + "/logout", {
        method: "POST",
        headers: authHeaders()
    }).finally(() => {
        sessionStorage.clear();
        location.href = "login.html";
    });
}

if (location.pathname.endsWith("dashboard.html") &&
    sessionStorage.getItem("role") !== "admin") {
    location.replace("login.html");
}
