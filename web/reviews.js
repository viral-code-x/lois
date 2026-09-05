const SUPABASE_URL = "https://fktiwkxubfgqsanltmbg.supabase.co";
const SUPABASE_KEY = "sb_publishable_wLDg44I9-rDmCNRY0SgqEQ_3mlkX0Lp";

const form = document.getElementById("review-form");
const reviewInput = document.getElementById("review");
const reviews = document.getElementById("reviews");
const status = document.getElementById("review-status");
const submitButton = document.getElementById("submit-review");

const headers = {
    "apikey": SUPABASE_KEY,
    "Authorization": `Bearer ${SUPABASE_KEY}`,
    "Content-Type": "application/json"
};

function createReviewElement(review) {
    const item = document.createElement("div");
    item.className = "review";

    const text = document.createElement("p");
    text.textContent = review.text;

    const date = document.createElement("small");
    date.textContent = new Date(review.created_at).toLocaleString();

    item.appendChild(text);
    item.appendChild(date);

    return item;
}

async function loadReviews() {
    try {
        const response = await fetch(
            `${SUPABASE_URL}/rest/v1/reviews?select=id,text,created_at&order=created_at.desc`,
            {
                method: "GET",
                headers
            }
        );

        if (!response.ok) {
            throw new Error("Failed to load reviews");
        }

        const data = await response.json();

        reviews.innerHTML = "";

        if (data.length === 0) {
            reviews.innerHTML = "<p>No reviews yet. Be the first!</p>";
            return;
        }

        data.forEach(review => {
            reviews.appendChild(createReviewElement(review));
        });
    } catch (error) {
        console.error(error);
        reviews.innerHTML = "<p>Could not load reviews right now.</p>";
    }
}

form.addEventListener("submit", async function(event) {
    event.preventDefault();

    const text = reviewInput.value.trim();

    if (!text || text.length > 1000) {
        return;
    }

    submitButton.disabled = true;
    status.textContent = "Submitting...";

    try {
        const response = await fetch(
            `${SUPABASE_URL}/rest/v1/reviews`,
            {
                method: "POST",
                headers: {
                    ...headers,
                    "Prefer": "return=minimal"
                },
                body: JSON.stringify({ text })
            }
        );

        if (!response.ok) {
            throw new Error("Failed to submit review");
        }

        form.reset();
        status.textContent = "Review submitted!";
        await loadReviews();
    } catch (error) {
        console.error(error);
        status.textContent = "Could not submit review. Please try again.";
    } finally {
        submitButton.disabled = false;
    }
});

loadReviews();
