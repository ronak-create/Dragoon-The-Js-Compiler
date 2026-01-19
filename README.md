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
  <li>✔ Symbol table (scoped, typed)</li>
  <li>✔ Three-Address Code (TAC)</li>
  <li>✔ Semantic analysis (type + scope checks)</li>
  <li>🚧 Native backends (QBE / LLVM) – in progress</li>
</ul>

<hr>

<h2>Compiler Pipeline</h2>

<pre>
JavaScript Source
        |
        v
+----------------+
|     Lexer      |
+----------------+
        |
        v
+----------------+
|     Parser     |
+----------------+
        |
        v
+----------------+
|      AST       |
+----------------+
        |
        v
+------------------------+
|  Semantic Analysis     |
|  (scope + type checks)|
+------------------------+
        |
        v
+----------------+
|   IR / TAC     |
+----------------+
        |
        v
+----------------+
|   QBE / LLVM   |
|   Codegen      |
+----------------+
        |
        v
Native Assembly / ELF
</pre>

<hr>

<h2>Directory Structure</h2>

<pre>
jscc/
├── include/
│   ├── lexer.h
│   ├── parser.h
│   ├── semantic.h
│   ├── ir.h
│   ├── qbe_codegen.h
│   └── llvm_codegen.h
├── src/
│   ├── lexer/
│   │   └── lexer.c
│   ├── parser/
│   │   └── parser.c
│   ├── semantic/
│   │   └── semantic.c
│   ├── ir/
│   │   └── ir.c
│   ├── qbe/
│   │   └── qbe_codegen.c
│   ├── llvm/
│   │   └── llvm_codegen.c
│   └── main.c
├── tests/
│   └── index.js
├── Makefile
└── README.md
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
  <li>Semantic validation</li>
  <li>Three-Address Code (TAC)</li>
  <li>QBE / LLVM IR (experimental)</li>
</ul>

<hr>

<h2>⚠ Development Notes (Important)</h2>

<p>
During QBE backend testing, the generated SSA currently contains incorrect temporary
references. For testing purposes, the following fixes were applied <strong>manually</strong>
to the generated <code>.qbe</code> file:
</p>

<ul>
  <li><code>%t1 =w add 1, %t1</code> → <code>%t1 =w add 1, %t0</code></li>
  <li><code>%t3 =w call $printi(w %t3)</code> → <code>%t3 =w call $printi(w %t2)</code></li>
  <li><code>%t5 =w mul %t5, 3</code> → <code>%t5 =w mul %t4, 3</code></li>
</ul>

<p>
This indicates a bug in <code>qbe_codegen.c</code> where the wrong SSA temporary
is being reused. Fixing correct temporary propagation is a <strong>TODO</strong>.
</p>

<hr>

<h2>Design Principles</h2>

<ul>
  <li>No parser generators</li>
  <li>No external runtime dependencies</li>
  <li>Portable C11</li>
  <li>Clear phase separation</li>
  <li>Explicit memory management</li>
</ul>

<hr>

<h2>Planned Improvements</h2>

<ul>
  <li>Fix SSA temporary reuse bug in QBE backend</li>
  <li>Proper lowering of control flow in QBE</li>
  <li>String literals and data section handling</li>
  <li>Register allocation (QBE-assisted)</li>
  <li>LLVM backend integration</li>
</ul>

<hr>

<p>
<strong>Status:</strong> Active development<br>
<strong>Version:</strong> v0.2
</p>
