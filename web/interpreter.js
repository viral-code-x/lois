let LOIS_MODULE = null;

window.loisInputResolver = null;

window.loisRequestInput = function (resolve) {
    const inputArea =
        document.getElementById("input-area");

    const input =
        document.getElementById("console-input");

    if (!inputArea || !input) {
        resolve("");
        return;
    }

    inputArea.hidden = false;
    input.value = "";
    input.focus();

    window.loisInputResolver = resolve;
};

function submitInput() {
    const resolver =
        window.loisInputResolver;

    if (!resolver)
        return;

    const input =
        document.getElementById("console-input");

    const inputArea =
        document.getElementById("input-area");

    const value = input.value;

    const output =
        document.getElementById("output");

    output.textContent +=
        "> " + value + "\n";

    inputArea.hidden = true;

    window.loisInputResolver = null;

    resolver(value + "\n");
}

async function initLOIS() {
    try {
        LOIS_MODULE = await LOIS();

        document.getElementById("status").textContent = "Ready";
        document.getElementById("run").disabled = false;
    } catch (error) {
        document.getElementById("status").textContent = "Failed to load LOIS";
        document.getElementById("output").textContent = String(error);
    }
}

async function runLOIS() {
    const code =
        document.getElementById("code").value;

    const output =
        document.getElementById("output");

    if (!LOIS_MODULE) {
        output.textContent =
            "LOIS is still loading...";
        return;
    }

    output.textContent = "";

    try {
        const run =
            LOIS_MODULE.cwrap(
                "lois_run_source",
                "string",
                ["string"],
                { async: true }
            );

        const result =
            await run(code);

        output.textContent =
            result || "";
    } catch (error) {
        output.textContent =
            "Runtime error: " + error;
    }
}

document.addEventListener("DOMContentLoaded", () => {
    document
        .getElementById("run")
        .addEventListener("click", runLOIS);

    const submitButton =
        document.getElementById("submit-input");

    if (submitButton) {
        submitButton.addEventListener(
            "click",
            submitInput
        );
    }

    const consoleInput =
        document.getElementById("console-input");

    if (consoleInput) {
        consoleInput.addEventListener(
            "keydown",
            (event) => {
                if (event.key === "Enter")
                    submitInput();
            }
        );
    }

    initLOIS();
});
