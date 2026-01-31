<h1>jscc – JavaScript Compiler (Experimental)</h1>

<p>
<strong>jscc</strong> is an experimental JavaScript compiler written in
<strong>C (C11)</strong>. The project focuses on understanding real compiler
internals by implementing each phase manually, without parser generators
or heavyweight frameworks.
</p>

<p>
The compiler currently targets a <strong>Unix-style toolchain</strong> and
generates native executables via the <strong>QBE</strong> backend.
</p>

<hr>

<h2>Project Status</h2>

<ul>
  <li>✔ Lexer</li>
  <li>✔ Recursive-descent parser</li>
  <li>✔ AST construction</li>
  <li>✔ Semantic analysis (scope + basic type checks)</li>
  <li>✔ Intermediate Representation (IR / TAC)</li>
  <li>✔ Control Flow Graph (CFG)</li>
  <li>✔ Constant folding & dead code elimination</li>
  <li>✔ QBE backend (end-to-end working)</li>
  <li>🚧 LLVM backend (planned)</li>
</ul>

<hr>

<h2>Platform Support</h2>

<p>
<strong>Supported:</strong>
</p>
<ul>
  <li>Linux</li>
  <li>WSL (Windows Subsystem for Linux)</li>
</ul>

<p>
<strong>Not supported:</strong>
</p>
<ul>
  <li>Native Windows toolchains (CMD / PowerShell, MinGW, MSVC)</li>
</ul>

<p>
The QBE backend emits Unix-style assembly and expects a POSIX environment.
Windows users should run the compiler inside <strong>WSL</strong>.
</p>

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
|  (scope + type checks) |
+------------------------+
        |
        v
+----------------+
|   IR / TAC     |
+----------------+
        |
        v
+----------------+
|     CFG        |
+----------------+
        |
        v
+------------------------+
|  Optimizations         |
|  (const fold, DCE)     |
+------------------------+
        |
        v
+----------------+
|   QBE IR       |
+----------------+
        |
        v
QBE → Assembly → GCC → Native Executable
</pre>

<hr>

<h2>Directory Structure</h2>

<pre>
(root-directory)
├── include
│   ├── cfg.h
│   ├── codegen.h
│   ├── ir.h
│   ├── lexer.h
│   ├── opt.h
│   ├── parser.h
│   ├── qbe_codegen.h
│   └── semantic.h
├── src
│   ├── cfg
│   │   └── cfg.c
│   ├── codegen
│   │   └── codegen.c
│   ├── ir
│   │   └── ir.c
│   ├── lexer
│   │   └── lexer.c
│   ├── opt
│   │   └── opt.c
│   ├── parser
│   │   └── parser.c
│   ├── qbe
│   │   └── qbe_codegen.c
│   ├── semantic
│   │   └── semantic.c
│   └── main.c
├── tests
│   └── index.js
├── .gitignore
├── Makefile
├── README.md
└── qbe
</pre>

<hr>

<h2>Supported JavaScript Subset</h2>

<ul>
  <li><code>let</code> and <code>const</code> declarations</li>
  <li>Integer literals (decimal, hex, binary)</li>
  <li>Boolean literals</li>
  <li>Binary expressions (<code>+</code>, <code>-</code>, <code>*</code>, <code>/</code>)</li>
  <li>Comparisons (<code>===</code>, <code>&lt;</code>)</li>
  <li><code>if / else</code> statements</li>
  <li><code>for</code> loops (basic form)</li>
  <li>Pre/Post increment (<code>++i</code>, <code>i++</code>)</li>
  <li><code>console.log()</code> for integer expressions</li>
</ul>

<hr>

<h2>Requirements</h2>

<ul>
  <li>Linux or WSL</li>
  <li>GCC (or Clang)</li>
  <li>QBE (<a href="https://c9x.me/compile/">https://c9x.me/compile/</a>)</li>
</ul>

<hr>

<h2>Build</h2>

<p>
The project uses a Makefile to manage the build.
</p>

<pre>
make
</pre>

<p>
This produces the <code>jscc</code> executable in the project root.
</p>

<hr>

<h2>Run</h2>

<pre>
./jscc tests/index.js
</pre>

<hr>

<h2>Running Your Own JavaScript File</h2>

<p>
You can run the compiler on any JavaScript file by passing its path:
</p>

<pre>
./jscc path/to/file.js
</pre>

<p>
Using the Makefile:
</p>

<pre>
make run FILE=path/to/file.js
</pre>

<p>
Examples:
</p>

<pre>
make run FILE=tests/index.js
make run FILE=examples/loops.js
</pre>

<p>
If no file is specified, the Makefile defaults to <code>tests/index.js</code>.
</p>

<p>
By default, the compiler:
</p>
<ul>
  <li>Generates QBE IR (<code>tmp/out.qbe</code>)</li>
  <li>Invokes QBE to produce assembly</li>
  <li>Invokes GCC to produce a native executable</li>
  <li>Runs the executable automatically</li>
</ul>

<hr>

<h2>Command-Line Flags</h2>

<ul>
  <li><code>-d</code> : Enable debug output (AST, IR, CFG)</li>
  <li><code>-q</code> : Stop after emitting QBE IR</li>
</ul>

<hr>

<h2>Limitations</h2>

<ul>
  <li>Integer-only code generation</li>
  <li>Strings parsed but not yet lowered in codegen</li>
  <li>No functions or closures</li>
  <li>No garbage collection</li>
  <li>No native Windows backend</li>
</ul>

<hr>

<h2>Design Principles</h2>

<ul>
  <li>No parser generators</li>
  <li>No external runtime dependencies</li>
  <li>Portable C11 code</li>
  <li>Explicit phase separation</li>
  <li>Educational clarity over performance</li>
</ul>

<hr>

<h2>Planned Improvements</h2>

<ul>
  <li>Full control-flow lowering in QBE</li>
  <li>String literals & data section support</li>
  <li>Improved type tracking in IR</li>
  <li>LLVM backend</li>
  <li>Better CLI and diagnostics</li>
</ul>

<hr>

<p>
<strong>Status:</strong> Active development<br>
<strong>Version:</strong> v0.3
</p>
