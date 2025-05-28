// Get DOM elements
const codeInput = document.getElementById('code-input');
const convertBtn = document.getElementById('convert-btn');
const statusMessage = document.getElementById('status-message');
const visualizationSection = document.getElementById('visualization-section');

// Add event listener to the convert button
convertBtn.addEventListener('click', async () => {
    const code = codeInput.value.trim();
    if (!code) {
        showStatus('Please enter some code first!', 'error');
        return;
    }

    try {
        // Show loading state
        convertBtn.disabled = true;
        convertBtn.textContent = 'Processing...';
        showStatus('Processing your code...', 'info');

        // Save code to input.cpp
        const response = await fetch('/save-code', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ code }),
        });

        if (!response.ok) {
            throw new Error('Failed to save code');
        }

        // Run the parser
        const parseResponse = await fetch('/run-parser', {
            method: 'POST',
        });

        if (!parseResponse.ok) {
            throw new Error('Failed to parse code');
        }

        // Show success message and visualization
        showStatus('Code processed successfully!', 'success');
        visualizationSection.style.display = 'block';

        // Load and render the tree
        const [treeData, traceData] = await Promise.all([
            fetch('tree.json').then(res => res.json()),
            fetch('trace.json').then(res => res.json())
        ]);

        // Clear previous visualization if any
        document.getElementById('tree').innerHTML = '';
        
        // Render the new tree
        renderTree(treeData, traceData);

    } catch (error) {
        showStatus('Error: ' + error.message, 'error');
    } finally {
        // Reset button state
        convertBtn.disabled = false;
        convertBtn.textContent = 'Convert & Visualize';
    }
});

function showStatus(message, type) {
    statusMessage.textContent = message;
    statusMessage.className = type;
}

function getNodeColor(label) {
    if (label.startsWith("Function")) return "url(#func-gradient)";
    if (label.startsWith("If")) return "#ff6f61";
    if (label.startsWith("While")) return "#6a4c93";
    if (label.startsWith("Assignment")) return "#43aa8b";
    if (label.startsWith("Return")) return "#f9c846";
    if (label.startsWith("VarDecl")) return "#577590";
    if (label.startsWith("FunctionCall")) return "#f3722c";
    if (label.startsWith("Parameters")) return "#4d908e";
    if (label.startsWith("Arguments")) return "#90be6d";
    if (label.startsWith("Expr")) return "#277da1";
    if (label.startsWith("Block") || label.startsWith("Body")) return "#b5838d";
    if (label.startsWith("Program")) return "#f9844a";
    return "#adb5bd";
}

function renderTree(treeData, traceData) {
    const margin = { top: 80, right: 120, bottom: 80, left: 120 };
    const width = 1600 - margin.left - margin.right;
    const height = 1000 - margin.top - margin.bottom;

    const svg = d3.select("#tree")
        .append("svg")
        .attr("width", "100%")
        .attr("height", "100%")
        .attr("viewBox", `0 0 ${width + margin.left + margin.right} ${height + margin.top + margin.bottom}`)
        .attr("preserveAspectRatio", "xMidYMid meet");

    svg.append("defs").append("linearGradient")
        .attr("id", "func-gradient")
        .attr("x1", "0%").attr("y1", "0%")
        .attr("x2", "100%").attr("y2", "100%")
        .selectAll("stop")
        .data([
            { offset: "0%", color: "#1971c2" },
            { offset: "100%", color: "#4dabf7" }
        ])
        .enter()
        .append("stop")
        .attr("offset", d => d.offset)
        .attr("stop-color", d => d.color);

    const g = svg.append("g")
        .attr("transform", `translate(${margin.left},${margin.top})`);

    const root = d3.hierarchy(treeData);

    const treeLayout = d3.tree()
        .size([height, width - 200])
        .separation((a, b) => (a.parent === b.parent ? 2.5 : 2));

    treeLayout(root);

    g.selectAll('.link')
        .data(root.links())
        .enter()
        .append('path')
        .attr('class', 'link')
        .attr('fill', 'none')
        .attr('stroke', '#adb5bd')
        .attr('stroke-width', 1.2)
        .attr('stroke-opacity', 0.8)
        .attr('d', d3.linkHorizontal()
            .x(d => d.y)
            .y(d => d.x));

    const node = g.selectAll('.node')
        .data(root.descendants())
        .enter()
        .append('g')
        .attr('class', 'node')
        .attr('transform', d => `translate(${d.y},${d.x})`);

    node.append('circle')
        .attr('r', 14)
        .attr('fill', d => getNodeColor(d.data.name))
        .attr('stroke', '#22223b')
        .attr('stroke-width', 2)
        .style('filter', 'drop-shadow(0 2px 8px rgba(80,80,80,0.15))')
        .on('mouseover', function () {
            d3.select(this)
                .attr('stroke', '#ffbe0b')
                .attr('stroke-width', 3);
        })
        .on('mouseout', function () {
            d3.select(this)
                .attr('stroke', '#22223b')
                .attr('stroke-width', 2);
        });

    node.append('text')
        .attr('dy', 5)
        .attr('x', d => d.children ? -20 : 20)
        .attr('text-anchor', d => d.children ? 'end' : 'start')
        .style('font-size', '13px')
        .style('font-family', '"Segoe UI", Roboto, sans-serif')
        .style('fill', '#343a40')
        .text(d => d.data.name);

    // No animation/highlighting of flow for any statements
}

// Load sample code into textarea
fetch('input.cpp')
    .then(response => response.text())
    .then(code => {
        codeInput.value = code;
    })
    .catch(error => {
        console.error('Error loading sample code:', error);
    });
