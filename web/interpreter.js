let LOIS_MODULE = null;

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

function runLOIS() {
    const code = document.getElementById("code").value;
    const output = document.getElementById("output");

    if (!LOIS_MODULE) {
        output.textContent = "LOIS is still loading...";
        return;
    }

    output.textContent = "";

    try {
        const run = LOIS_MODULE.cwrap(
            "lois_run_source",
            "string",
            ["string"]
        );

        const result = run(code);

        output.textContent = result || "";
    } catch (error) {
        output.textContent = "Runtime error: " + error;
    }
}

document.addEventListener("DOMContentLoaded", () => {
    document.getElementById("run").addEventListener("click", runLOIS);
    initLOIS();
});
