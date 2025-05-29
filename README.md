# Parse Tree Visualizer

This project implements a **Parse Tree Visualizer** that graphically represents the syntactic structure of input code. It leverages a **Recursive Descent Parser** written in C++ to generate the parse tree and uses JavaScript for interactive web-based visualization. A **Symbol Table** is also constructed during the parsing process to manage identifiers and their attributes.

## Table of Contents

- [Features](#features)
- [Technologies Used](#technologies-used)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Building the Parser](#building-the-parser)
  - [Running the Visualizer](#running-the-visualizer)
- [How it Works](#how-it-works)
  - [Recursive Descent Parser (C++)](#recursive-descent-parser-c)
  - [Symbol Table](#symbol-table)
  - [Visualization (JavaScript)](#visualization-javascript)
- [Usage](#usage)
- [Example](#example)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

## Features

- **Recursive Descent Parsing:** Efficiently parses input code based on a defined grammar.
- **Parse Tree Generation:** Creates a detailed hierarchical representation of the code's syntax.
- **Symbol Table Management:** Tracks identifiers, their types, and scopes for semantic analysis.
- **Interactive Visualization:** Renders the parse tree in a web browser using JavaScript, allowing for zooming, panning, and node inspection.
- **Cross-Platform (Parser):** The C++ parser can be compiled and run on various operating systems.
- **Browser-Based Visualization:** The visualization component is accessible through any modern web browser.

## Technologies Used

- **C++:** For the implementation of the Recursive Descent Parser and Symbol Table.
- **JavaScript:** For the interactive web-based parse tree visualization.
- **HTML/CSS:** For structuring and styling the web interface.

## Getting Started

Follow these instructions to get a copy of the project up and running on your local machine.

### Prerequisites

- A C++ compiler (e.g., g++).
- A web browser (e.g., Chrome, Firefox, Edge).
- (Optional) Git for cloning the repository.

### Building the Parser

1.  **Clone the repository (if you haven't already):**

    ```bash
    git clone [https://github.com/Rohittiwari777/PARSE_TREE_VISUALIZER_CD_PBL.git](https://github.com/Rohittiwari777/PARSE_TREE_VISUALIZER_CD_PBL.git)
    cd parse-tree-visualizer
    ```

2.  **Navigate to the parser directory:**

    ```bash
    cd parser_cpp
    ```

    (Note: Adjust `parser_cpp` if your C++ parser code is in a different directory name, e.g., `src` or `compiler`)

3.  **Compile the C++ parser:**
    ```bash
    g++ -o parser main.cpp # Add other source files as needed, e.g., parser.cpp, symbol_table.cpp
    ```
    This will generate an executable named `parser` (or `parser.exe` on Windows).

### Running the Visualizer

1.  **Generate Parse Tree Data:**
    The C++ parser, when executed with input code, should output a data format (e.g., JSON, a custom text format) that the JavaScript visualizer can consume. Ensure your parser outputs this data to a file accessible by the web interface. For example, let's assume it outputs to `parse_tree_data.json` in the `web_visualizer` directory.

    ```bash
    ./parser < input_code.txt > ../web_visualizer/parse_tree_data.json
    ```

    (Adjust paths and filenames as per your project structure and parser's output mechanism.)

2.  **Open the Visualization:**
    Navigate to the `web_visualizer` directory (or wherever your HTML/JavaScript files are located) and open the `index.html` file in your web browser.

    ```bash
    cd ../web_visualizer
    # Then open index.html in your browser.
    ```

    Alternatively, you can use a simple local web server (e.g., Python's `http.server`) to serve the files, which is recommended for more complex JavaScript applications to avoid CORS issues.

    ```bash
    # From the web_visualizer directory
    python -m http.server 8000
    ```

    Then, open your browser and go to `http://localhost:8000`.

## How it Works

### Recursive Descent Parser (C++)

The core of the parsing logic is implemented in C++ using a recursive descent approach. Each non-terminal in the grammar corresponds to a function that attempts to match the corresponding language construct. The parser generates a tree structure (internally in C++) representing the parse tree. This tree is then serialized into a format suitable for the JavaScript visualizer.

### Symbol Table

Alongside parsing, a symbol table is maintained. This data structure stores information about identifiers (variables, functions, etc.) encountered in the source code. For each identifier, it records attributes such as its type, scope, and memory location, which is crucial for subsequent semantic analysis.

### Visualization (JavaScript)

The JavaScript component reads the serialized parse tree data. It then uses a suitable library (e.g., D3.js, although a custom implementation is also feasible) to render the tree graphically. The visualization allows users to explore the tree hierarchy, click on nodes to view details (e.g., from the symbol table), and interact with the tree.

## Usage

1.  **Prepare your input code:** Create a text file containing the code you want to parse and visualize (e.g., `input_code.txt`). The grammar supported by the parser should be documented within the `parser_cpp` directory.
2.  **Run the parser:** Execute the C++ parser with your input file to generate the parse tree data file.
3.  **Open the visualizer:** Load `index.html` in your web browser. The visualizer will automatically load the generated parse tree data (assuming the paths are configured correctly) and display the tree.
4.  **Interact:** Use your mouse to pan and zoom the tree. Click on nodes to see more details about their associated tokens or symbol table entries.

## Example

Suppose your parser supports a simple arithmetic grammar.

**`input_code.txt`:**
x = 10 + y \* 2;

After running the parser, `parse_tree_data.json` might contain something like:

```json
{
  "name": "Program",
  "children": [
    {
      "name": "Assignment",
      "children": [
        {"name": "ID", "value": "x", "symbol_table_entry": "..." },
        {"name": "Operator", "value": "="},
        {
          "name": "Expression",
          "children": [
            {"name": "Number", "value": "10"},
            {"name": "Operator", "value": "+"},
            {
              "name": "Expression",
              "children": [
                {"name": "ID", "value": "y", "symbol_table_entry": "..." },
                {"name": "Operator", "value": "*"},
                {"name": "Number", "value": "2"}
              ]
            }
          ]
        },
        {"name": "Punctuation", "value": ";"}
      ]
    }
  ]
}
The visualizer would then display this hierarchical structure as an interactive tree.


##contributer :
Rohit
Lokesh Singh
Vibha Chandola
Ankit Adhikari



