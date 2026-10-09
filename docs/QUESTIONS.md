# LORE Design Questions and Decisions (QUESTIONS.md)

This document tracks all design decisions, grammar clarifications, and questions as required by the LORE specification.

---

### Q1: Display Stage Statements Syntax
- **Context**: The requirements specify:
  `Display stage statements: show display at input., show display at process., show display at output., hide display.`
- **Grammar Status**: The EBNF in `spec/LORE_grammar.md` defines `call_stmt` as:
  `call_stmt = ( "add" , expr , "to" , ident | "subtract" , expr , "from" , ident | "show" , expr | "play" , ident ) , "." ;`
- **Resolution**: We support display statements as statements:
  - `show display at input.`
  - `show display at process.`
  - `show display at output.`
  - `hide display.`
  In addition, top-level or handler-level display statements are permitted.
- **Status**: TODO(question) - Picked simplest option: treat `show display at <stage>.` and `hide display.` as valid statements.

---

### Q2: Package Header Field Order & `lable` Alias
- **Context**: Spec says:
  `package_header = package_field , package_field , package_field ;`
  `package_field = "package:" , ( "name" | "id" | "label" ) , ":" , string , "." ;`
  And notes: `lable` is accepted as an alias of `label`.
- **Resolution**: Any order of the 3 required fields (`name`, `id`, `label` / `lable`) is parsed, each appearing exactly once. Lexer/parser accepts both `label` and `lable`.
- **Status**: Resolved.

---

### Q3: `.end<handle>att.` Token Matching
- **Context**: Each UI element attribute block ends with `.end<handle>att.`. For example, `root is bg.` ... `.endbgatt.`, `put text title inside bg.` ... `.endtitleatt.`.
- **Resolution**: The lexer tokenizes `.end<handle>att.` as an `ATTR_END` token carrying the handle name `<handle>`, and the parser/checker verifies that it matches the element currently being configured. If mismatched, emit a clear error e.g. `Mismatched attribute terminator: expected .endtitleatt. but found .endbgatt.`.
- **Status**: Resolved.

---

### Q4: Dimension Values in Expressions vs Attributes
- **Context**: In attributes, values can be dimensions (`200dp`, `64dp`, `32sp`, `100%`).
- **Resolution**: The lexer tokenizes dimensions as a number with a unit suffix (`dp`, `sp`, `px`, `%`). The parser accepts dimensions in attribute statements.
- **Status**: Resolved.

---

### Q5: ABI Function Prefix
- **Context**: Spec section 13 has `ore_init`, `ore_input`, `ore_render` with note `(Rename to lore_* when you finalize the prefix.)`.
- **Resolution**: We standardize on the `lore_*` prefix:
  - `lore_init(const char* config)`
  - `lore_input(const void* signal)`
  - `lore_stage(int stage)`
  - `lore_render(void* surface)`
- **Status**: Resolved.
