const form = document.getElementById("review-form");
const reviews = document.getElementById("reviews");

form.addEventListener("submit", function(event) {
    event.preventDefault();

    const text =
        document.getElementById("review").value.trim();

    if (!text)
        return;

    const item = document.createElement("div");

    item.className = "review";

    item.textContent = text;

    reviews.prepend(item);

    form.reset();
});
