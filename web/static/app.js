const exampleCode = `#include <iostream>

int main() {
    int values[2] = {1, 2};

    for (int i = 0; i <= 2; i++) {
        std::cout << values[i] << "\\n";
    }

    return values[4];
}
`;

const codeInput = document.querySelector("#code");
const editorShell = document.querySelector("#editorShell");
const lineNumbers = document.querySelector("#lineNumbers");
const lineCount = document.querySelector("#lineCount");
const selectedLine = document.querySelector("#selectedLine");
const languageInput = document.querySelector("#language");
const reviewButton = document.querySelector("#reviewButton");
const exampleButton = document.querySelector("#exampleButton");
const clearButton = document.querySelector("#clearButton");
const statusText = document.querySelector("#status");
const summaryText = document.querySelector("#summary");
const totalIssuesText = document.querySelector("#totalIssues");
const findingsList = document.querySelector("#findings");
const filterButtons = Array.from(document.querySelectorAll(".filter-button"));

const severityCounts = {
    CRITICAL: document.querySelector("#criticalCount"),
    HIGH: document.querySelector("#highCount"),
    MEDIUM: document.querySelector("#mediumCount"),
    LOW: document.querySelector("#lowCount")
};

let allFindings = [];
let activeFilter = "ALL";
let highlightedLine = null;

function setStatus(value) {
    statusText.textContent = value;
}

function setButtonState(state) {
    if (state === "analyzing") {
        reviewButton.disabled = true;
        reviewButton.textContent = "Analyzing...";
        return;
    }

    if (state === "complete") {
        reviewButton.disabled = true;
        reviewButton.textContent = "Review Complete";

        window.setTimeout(() => {
            reviewButton.disabled = false;
            reviewButton.textContent = "Review Code";
        }, 900);

        return;
    }

    reviewButton.disabled = false;
    reviewButton.textContent = "Review Code";
}

function codeLines() {
    return codeInput.value.split("\n");
}

function updateLineNumbers() {
    const lines = codeLines();
    const count = Math.max(lines.length, 1);

    lineNumbers.innerHTML = "";

    for (let index = 1; index <= count; index++) {
        const number = document.createElement("div");
        number.className = "line-number";

        if (index === highlightedLine) {
            number.classList.add("highlighted");
        }

        number.textContent = String(index);
        lineNumbers.appendChild(number);
    }

    lineCount.textContent = `${count} ${count === 1 ? "line" : "lines"}`;
    lineNumbers.scrollTop = codeInput.scrollTop;
}

function clearHighlight() {
    highlightedLine = null;
    selectedLine.textContent = "No line selected";
    document.querySelectorAll(".finding.active").forEach((element) => {
        element.classList.remove("active");
    });
    updateLineNumbers();
}

function highlightLine(line, card) {
    const lines = codeLines();
    const targetLine = Math.max(1, Math.min(line, lines.length));
    let start = 0;

    for (let index = 0; index < targetLine - 1; index++) {
        start += lines[index].length + 1;
    }

    const end = start + lines[targetLine - 1].length;
    highlightedLine = targetLine;

    codeInput.focus();
    codeInput.setSelectionRange(start, end);

    const lineHeight = parseFloat(getComputedStyle(codeInput).lineHeight);
    codeInput.scrollTop = Math.max(0, (targetLine - 1) * lineHeight - 80);
    lineNumbers.scrollTop = codeInput.scrollTop;

    selectedLine.textContent = `Line ${targetLine}: ${lines[targetLine - 1].trim() || "(blank)"}`;

    document.querySelectorAll(".finding.active").forEach((element) => {
        element.classList.remove("active");
    });

    if (card) {
        card.classList.add("active");
    }

    updateLineNumbers();
}

function resetSeverityCounts() {
    Object.values(severityCounts).forEach((element) => {
        element.textContent = "0";
    });
}

function renderEmpty(title, message) {
    findingsList.innerHTML = "";
    const empty = document.createElement("div");
    empty.className = "empty-state";
    empty.innerHTML = `<div><strong>${title}</strong><p>${message}</p></div>`;
    findingsList.appendChild(empty);
}

function renderError(title, message) {
    findingsList.innerHTML = "";
    const error = document.createElement("div");
    error.className = "error-state";

    const strong = document.createElement("strong");
    strong.textContent = title;

    const text = document.createElement("p");
    text.textContent = message;

    const wrapper = document.createElement("div");
    wrapper.append(strong, text);
    error.appendChild(wrapper);
    findingsList.appendChild(error);
}

function normalizedSeverity(severity) {
    return severity === "INFO" ? "LOW" : severity;
}

function countSeverities(findings) {
    const counts = {
        CRITICAL: 0,
        HIGH: 0,
        MEDIUM: 0,
        LOW: 0
    };

    findings.forEach((finding) => {
        const severity = normalizedSeverity(finding.severity);

        if (Object.prototype.hasOwnProperty.call(counts, severity)) {
            counts[severity] += 1;
        }
    });

    return counts;
}

function filteredFindings() {
    if (activeFilter === "ALL") {
        return allFindings;
    }

    return allFindings.filter((finding) => {
        return normalizedSeverity(finding.severity) === activeFilter;
    });
}

function createFindingCard(finding) {
    const item = document.createElement("article");
    item.className = "finding";
    item.tabIndex = 0;

    const top = document.createElement("div");
    top.className = "finding-top";

    const severity = document.createElement("span");
    severity.className = `severity-pill ${finding.severity}`;
    severity.textContent = finding.severity;

    const rule = document.createElement("span");
    rule.className = "rule-id";
    rule.textContent = finding.ruleId || finding.rule || "RULE";

    top.append(severity, rule);

    const title = document.createElement("h3");
    title.textContent = finding.title || finding.message || "Review finding";

    const meta = document.createElement("div");
    meta.className = "finding-meta";
    meta.textContent = `Line ${finding.line || 1} | ${finding.category || "GENERAL"}`;

    const description = document.createElement("p");
    description.textContent = finding.description || "No description provided.";

    const suggestion = document.createElement("div");
    suggestion.className = "suggestion";

    const suggestionTitle = document.createElement("strong");
    suggestionTitle.textContent = "Suggested fix";

    const suggestionText = document.createElement("p");
    suggestionText.textContent = finding.suggestion || "Review the highlighted line.";

    suggestion.append(suggestionTitle, suggestionText);
    item.append(top, title, meta, description, suggestion);

    item.addEventListener("click", () => {
        highlightLine(Number(finding.line || 1), item);
    });

    item.addEventListener("keydown", (event) => {
        if (event.key === "Enter" || event.key === " ") {
            event.preventDefault();
            highlightLine(Number(finding.line || 1), item);
        }
    });

    return item;
}

function renderFindings() {
    const findings = filteredFindings();
    findingsList.innerHTML = "";

    if (allFindings.length === 0) {
        renderEmpty(
            "No issues found",
            "Your code passed all available static-analysis rules."
        );
        return;
    }

    if (findings.length === 0) {
        renderEmpty(
            "No matching findings",
            "Choose another severity filter to view more results."
        );
        return;
    }

    findings.forEach((finding) => {
        findingsList.appendChild(createFindingCard(finding));
    });
}

function renderReview(data) {
    allFindings = Array.isArray(data.findings) ? data.findings : [];
    const counts = countSeverities(allFindings);

    summaryText.textContent = data.summary || `${allFindings.length} issues found`;
    totalIssuesText.textContent = String(data.totalIssues ?? allFindings.length);

    Object.entries(counts).forEach(([severity, count]) => {
        severityCounts[severity].textContent = String(count);
    });

    renderFindings();
}

function setActiveFilter(filter) {
    activeFilter = filter;

    filterButtons.forEach((button) => {
        button.classList.toggle("active", button.dataset.filter === filter);
    });

    renderFindings();
}

async function reviewCode() {
    const code = codeInput.value;

    if (!code.trim()) {
        summaryText.textContent = "Review blocked";
        totalIssuesText.textContent = "0";
        resetSeverityCounts();
        renderError("Empty editor", "Enter C++ code before running a review.");
        return;
    }

    clearHighlight();
    setButtonState("analyzing");
    setStatus("Analyzing");
    resetSeverityCounts();

    try {
        const response = await fetch("/api/review", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                code,
                language: languageInput.value
            })
        });

        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.details || data.error || "Review failed.");
        }

        activeFilter = "ALL";
        filterButtons.forEach((button) => {
            button.classList.toggle("active", button.dataset.filter === "ALL");
        });

        renderReview(data);
        setStatus("Complete");
        setButtonState("complete");
    } catch (error) {
        allFindings = [];
        summaryText.textContent = "Review failed";
        totalIssuesText.textContent = "0";
        resetSeverityCounts();
        renderError("Review failed", error.message);
        setStatus("Error");
        setButtonState("normal");
    }
}

function loadExample() {
    codeInput.value = exampleCode;
    clearHighlight();
    updateLineNumbers();
    codeInput.focus();
}

function clearEditor() {
    codeInput.value = "";
    allFindings = [];
    summaryText.textContent = "Ready to review";
    totalIssuesText.textContent = "0";
    resetSeverityCounts();
    clearHighlight();
    updateLineNumbers();
    renderEmpty("Ready to review", "Paste your C++ code and click Review Code to begin.");
    codeInput.focus();
}

codeInput.addEventListener("input", () => {
    clearHighlight();
    updateLineNumbers();
});

codeInput.addEventListener("scroll", () => {
    lineNumbers.scrollTop = codeInput.scrollTop;
});

reviewButton.addEventListener("click", reviewCode);
exampleButton.addEventListener("click", loadExample);
clearButton.addEventListener("click", clearEditor);

filterButtons.forEach((button) => {
    button.addEventListener("click", () => {
        setActiveFilter(button.dataset.filter);
    });
});

codeInput.value = exampleCode;
updateLineNumbers();
resetSeverityCounts();
renderEmpty("Ready to review", "Paste your C++ code and click Review Code to begin.");
