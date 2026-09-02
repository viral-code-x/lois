let LOIS_MODULE = null;

let loisInputResolver = null;

function terminal() {
    return document.getElementById("terminal");
}

function output() {
    return document.getElementById("output");
}

function scrollTerminal() {
    const t = terminal();

    if (t)
        t.scrollTop = t.scrollHeight;
}

/*
 * C calls this whenever LOIS writes output.
 */
window.loisTerminalWrite = function(text) {
    if (!text)
        return;

    const out = output();

    if (!out)
        return;

    out.textContent += text;

    scrollTerminal();
};

/*
 * C calls this when LOIS needs input.
 */
window.loisRequestInput = function(resolve) {
    const area =
        document.getElementById("input-area");

    const input =
        document.getElementById("console-input");

    if (!area || !input) {
        resolve("");
        return;
    }

    loisInputResolver = resolve;

    area.hidden = false;

    input.value = "";
    input.focus();

    scrollTerminal();
};

function submitInput() {
    if (!loisInputResolver)
        return;

    const input =
        document.getElementById("console-input");

    const area =
        document.getElementById("input-area");

    const value = input.value;

    /*
     * Echo input into terminal.
     */
    const out = output();

    if (out)
        out.textContent += "> " + value + "\n";

    area.hidden = true;

    const resolve = loisInputResolver;
    loisInputResolver = null;

    scrollTerminal();

    /*
     * Resume the paused C interpreter.
     */
    resolve(value);
}

async function initLOIS() {
    try {
        LOIS_MODULE = await LOIS();

        const run =
            document.getElementById("run");

        if (run)
            run.disabled = false;

    } catch (error) {
        output().textContent =
            "Failed to load LOIS:\n" +
            String(error);
    }
}

async function runLOIS() {
    const code =
        document.getElementById("code").value;

    const out = output();

    if (!LOIS_MODULE) {
        out.textContent =
            "LOIS is still loading...";

        return;
    }

    /*
     * Fresh terminal.
     */
    out.textContent = "";

    const area =
        document.getElementById("input-area");

    if (area)
        area.hidden = true;

    loisInputResolver = null;

    try {
        const run =
            LOIS_MODULE.cwrap(
                "lois_run_source",
                "string",
                ["string"],
                {
                    async: true
                }
            );

        /*
         * Asyncify pauses here whenever
         * LOIS asks the browser for input.
         */
        await run(code);

        scrollTerminal();

    } catch (error) {
        out.textContent +=
            "\nRuntime error: " +
            String(error);

        scrollTerminal();
    }
}

document.addEventListener(
    "DOMContentLoaded",
    function() {

        const run =
            document.getElementById("run");

        if (run)
            run.addEventListener(
                "click",
                runLOIS
            );

        const input =
            document.getElementById(
                "console-input"
            );

        if (input) {
            input.addEventListener(
                "keydown",
                function(event) {

                    if (event.key === "Enter") {
                        event.preventDefault();
                        submitInput();
                    }

                }
            );
        }

        initLOIS();
    }
);
