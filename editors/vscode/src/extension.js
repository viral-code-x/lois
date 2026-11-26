const vscode = require('vscode');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

const keywords = [
    ['output', 'Display text or an evaluated expression.'],
    ['input', 'Read user input.'],
    ['if', 'Begin a conditional statement.'],
    ['else if', 'Check another condition in an if chain.'],
    ['but if', 'Evaluate an additional conditional branch.'],
    ['else', 'Provide the alternative branch.'],
    ['then', 'Connect a statement to a condition or loop.'],
    ['while', 'Repeat while a condition is true.'],
    ['for', 'Create a counted or conditional loop.'],
    ['repeat', 'Repeat a specified number of times.'],
    ['stop', 'Stop the current loop.'],
    ['skip', 'Skip the current loop iteration.'],
    ['is', 'Assign text or use LOIS is-based syntax.'],
    ['with', 'Supply an optional input prompt.'],
    ['num', 'Numeric variable declaration keyword.'],
    ['text', 'Text variable declaration keyword.'],
    ['True', 'Boolean true value.'],
    ['False', 'Boolean false value.'],
    ['and', 'Logical AND operator.'],
    ['or', 'Logical OR operator.'],
    ['not', 'Logical NOT operator.'],
    ['note:', 'Start a single-line comment.']
];

const builtins = [
    ['abs', 'Absolute value.'],
    ['round', 'Round a number.'],
    ['floor', 'Round down to an integer.'],
    ['ceil', 'Round up to an integer.'],
    ['root', 'Calculate a square root.'],
    ['root3', 'Calculate a cube root.'],
    ['root4', 'Calculate a fourth root.'],
    ['root5', 'Calculate a fifth root.'],
    ['sin', 'Sine function. LOIS uses degrees by default.'],
    ['cos', 'Cosine function. LOIS uses degrees by default.'],
    ['tan', 'Tangent function. LOIS uses degrees by default.'],
    ['log', 'Base-10 logarithm.'],
    ['ln', 'Natural logarithm.'],
    ['rad', 'Convert degrees to radians.'],
    ['pi', 'Mathematical constant pi.']
];

function resolveInterpreter(document) {
    const configured = vscode.workspace
        .getConfiguration('lois')
        .get('executable', '');

    const workspaceFolder =
        vscode.workspace.getWorkspaceFolder(document.uri);

    const root = workspaceFolder
        ? workspaceFolder.uri.fsPath
        : path.dirname(document.uri.fsPath);

    const executableName = process.platform === 'win32'
        ? 'lois.exe'
        : 'lois';

    function isUsable(candidate) {
        if (!fs.existsSync(candidate)) return false;

        if (process.platform === 'win32') return true;

        try {
            fs.accessSync(candidate, fs.constants.X_OK);
            return true;
        } catch {
            return false;
        }
    }

    // An explicitly configured interpreter takes priority.
    if (configured.trim()) {
        const configuredPath = path.isAbsolute(configured)
            ? configured
            : path.resolve(root, configured);

        return isUsable(configuredPath)
            ? configuredPath
            : null;
    }

    // Check workspace builds. This also supports LOIS development.
    const workspaceCandidates = [];
    let current = root;

    while (true) {
        workspaceCandidates.push(
            path.join(current, 'build', executableName),
            path.join(current, 'build', 'Debug', executableName),
            path.join(current, executableName)
        );

        const parent = path.dirname(current);

        if (parent === current) break;
        current = parent;
    }

    for (const candidate of workspaceCandidates) {
        if (isUsable(candidate)) return candidate;
    }

    // Find the bundled interpreter for this operating system and CPU.
    const platformKey = `${process.platform}-${process.arch}`;

    const bundledPath = path.join(
        __dirname,
        '..',
        'runtime',
        platformKey,
        executableName
    );

    if (isUsable(bundledPath)) return bundledPath;

    // Finally, search the system PATH.
    const pathEntries = (process.env.PATH || '')
        .split(path.delimiter)
        .filter(Boolean);

    for (const entry of pathEntries) {
        const candidate = path.join(entry, executableName);

        if (isUsable(candidate)) return candidate;
    }

    return null;
}

async function runFile(output) {
    const editor = vscode.window.activeTextEditor;

    if (!editor || editor.document.languageId !== 'lois') {
        vscode.window.showWarningMessage(
            'Open a LOIS (.is) file before running.'
        );
        return;
    }

    const document = editor.document;

    if (document.isUntitled) {
        vscode.window.showWarningMessage(
            'Save your LOIS file before running it.'
        );
        return;
    }

    if (document.isDirty) {
        const saved = await document.save();

        if (!saved) {
            vscode.window.showWarningMessage(
                'Save the LOIS file before running it.'
            );
            return;
        }
    }

    const file = document.uri.fsPath;

    if (path.extname(file).toLowerCase() !== '.is') {
        vscode.window.showWarningMessage(
            'The active file must have a .is extension.'
        );
        return;
    }

    const executable = resolveInterpreter(document);

    output.clear();
    output.show(true);

    output.appendLine('================================');
    output.appendLine('             LOIS');
    output.appendLine('================================');
    output.appendLine('File: ' + file);
    output.appendLine('');

    if (!executable) {
        output.appendLine(
            'ERROR: LOIS interpreter not found.'
        );
        output.appendLine(
            'Build the LOIS interpreter or configure lois.executable.'
        );

        vscode.window.showErrorMessage(
            'LOIS interpreter not found. Check the LOIS executable path.'
        );
        return;
    }

    const workspaceFolder =
        vscode.workspace.getWorkspaceFolder(document.uri);

    const cwd = workspaceFolder
        ? workspaceFolder.uri.fsPath
        : path.dirname(file);

    output.appendLine('Interpreter: ' + executable);
    output.appendLine('Running...');
    output.appendLine('');

    let processExited = false;

    const child = spawn(executable, [file], {
        cwd,
        windowsHide: true,
        shell: false
    });

    child.stdout.on('data', data => {
        output.append(data.toString());
    });

    child.stderr.on('data', data => {
        output.append(data.toString());
    });

    child.on('error', error => {
        processExited = true;
        output.appendLine('');
        output.appendLine('LOIS launch error: ' + error.message);

        vscode.window.showErrorMessage(
            'Could not launch LOIS: ' + error.message
        );
    });

    child.on('close', (code, signal) => {
        processExited = true;

        output.appendLine('');

        if (code === 0) {
            output.appendLine('LOIS execution completed successfully.');
            vscode.window.setStatusBarMessage(
                'LOIS: Execution successful',
                4000
            );
        } else {
            output.appendLine(
                'LOIS execution failed. Exit code: ' +
                (code === null ? 'unknown' : code)
            );

            if (signal) {
                output.appendLine('Signal: ' + signal);
            }

            vscode.window.showErrorMessage(
                'LOIS execution failed. Check LOIS Output.'
            );
        }
    });

    child.on('exit', () => {
        if (!processExited) {
            // The close event handles final output and exit status.
        }
    });
}

function activate(context) {
    const output = vscode.window.createOutputChannel('LOIS');

    const completionProvider =
        vscode.languages.registerCompletionItemProvider(
            'lois',
            {
                provideCompletionItems() {
                    const items = [];

                    for (const [word, description] of keywords) {
                        const item = new vscode.CompletionItem(
                            word,
                            vscode.CompletionItemKind.Keyword
                        );

                        item.detail = 'LOIS keyword';
                        item.documentation =
                            new vscode.MarkdownString(description);

                        items.push(item);
                    }

                    for (const [name, description] of builtins) {
                        const item = new vscode.CompletionItem(
                            name,
                            vscode.CompletionItemKind.Function
                        );

                        item.detail = 'LOIS built-in';
                        item.documentation =
                            new vscode.MarkdownString(description);

                        if (name !== 'pi') {
                            item.insertText =
                                new vscode.SnippetString(name + '($1)');
                        }

                        items.push(item);
                    }

                    return items;
                }
            }
        );

    const documentation = new Map([
        ...keywords,
        ...builtins
    ]);

    const hoverProvider =
        vscode.languages.registerHoverProvider(
            'lois',
            {
                provideHover(document, position) {
                    const range =
                        document.getWordRangeAtPosition(position);

                    if (!range) return;

                    const word = document.getText(range);
                    const description = documentation.get(word);

                    if (!description) return;

                    return new vscode.Hover(
                        new vscode.MarkdownString(
                            '**' + word + '** — ' + description
                        )
                    );
                }
            }
        );

    const runCommand =
        vscode.commands.registerCommand(
            'lois.runFile',
            () => runFile(output)
        );

    context.subscriptions.push(
        output,
        completionProvider,
        hoverProvider,
        runCommand
    );
}

function deactivate() {}

module.exports = { activate, deactivate };
