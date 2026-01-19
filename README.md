<h1>jscc – JavaScript Compiler (WIP)</h1>

<p>
<strong>jscc</strong> is a work-in-progress JavaScript compiler written in <strong>C (C11)</strong>.
This project focuses on understanding real compiler internals by implementing each phase
manually without external tools or parser generators.
</p>

<hr>

<h2>Project Status</h2>

<ul>
  <li>✔ Lexer</li>
  <li>✔ Parser (recursive descent)</li>
  <li>✔ AST construction</li>
  <li>✔ Symbol table (basic)</li>
  <li>✔ Three-Address Code (TAC)</li>
  <li>🚧 Semantic analysis (in progress)</li>
  <li>🚧 Code generation (planned)</li>
</ul>

<hr>

<h2>Compiler Pipeline</h2>

<pre>
Source Code
    |
    v
+---------+
|  Lexer  |
+---------+
    |
    v
+---------+
| Parser  |
+---------+
    |
    v
+---------+
|  AST    |
+---------+
    |
    v
+------------------+
| Semantic Analysis|
+------------------+
    |
    v
+---------+
|  TAC    |
+---------+
    |
    v
+--------------+
| Codegen (WIP)|
+--------------+
</pre>

<hr>

<h2>Directory Structure</h2>

<pre>
jscc/
├── include/
│   ├── lexer.h
│   └── parser.h
├── src/
│   ├── lexer/
│   │   └── lexer.c
│   ├── parser/
│   │   └── parser.c
│   └── main.c
├── tests/
│   └── index.js
├── Makefile
└── README.html
</pre>

<hr>

<h2>Supported JavaScript Features</h2>

<ul>
  <li><code>let</code> and <code>const</code> declarations</li>
  <li>Assignments and literals (number, string, boolean)</li>
  <li>Binary expressions (<code>+</code>, <code>-</code>, <code>*</code>, <code>/</code>, <code>&lt;</code>, <code>===</code>)</li>
  <li><code>if / else</code> statements</li>
  <li><code>for</code> and <code>while</code> loops</li>
  <li>Pre/Post increment (<code>++i</code>, <code>i++</code>)</li>
  <li><code>console.log()</code> function calls</li>
</ul>

<hr>

<h2>Build</h2>

<pre>
make
</pre>

<hr>

<h2>Run</h2>

<pre>
./jscc tests/index.js
</pre>

<p>The compiler outputs:</p>
<ul>
  <li>Token stream</li>
  <li>AST structure</li>
  <li>Symbol table entries</li>
  <li>Generated Three-Address Code (TAC)</li>
</ul>

<hr>

<h2>Design Principles</h2>

<ul>
  <li>No external dependencies</li>
  <li>No POSIX-specific APIs</li>
  <li>Portable C11 code</li>
  <li>Clear phase separation</li>
  <li>Explicit memory ownership</li>
</ul>

<hr>

<h2>Planned Improvements</h2>

<ul>
  <li>Proper semantic analysis pass</li>
  <li>Scope-aware symbol table</li>
  <li>Expression precedence handling</li>
  <li>IR optimizations</li>
  <li>Target code generation</li>
</ul>

<hr>

<h2>License</h2>

<p>License to be decided.</p>

<hr>

<p>
<strong>Status:</strong> Active development<br>
<strong>Version:</strong> v0.1
</p>